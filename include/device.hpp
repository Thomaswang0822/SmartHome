#pragma once

#include "device_data.hpp"
#include "room.hpp"
#include "utils.hpp"

#include <algorithm>
#include <chrono>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

// Common interface for all devices, pure virtual
class DeviceInterface {
public:
    virtual ~DeviceInterface() = default;

    virtual void run(std::shared_ptr<DeviceDataBase> data) = 0;
    virtual uint32_t timeTravel(uint32_t duration_sec = 0) = 0;

    virtual void printLog() const = 0;
    static void loginRoom(std::shared_ptr<Room> room) { s_room = room; }

    virtual std::string getName() const = 0;
    virtual std::string getCurrentTime() const = 0;

protected:
    /// @brief static because the room should be unique, while it's shared
    /// across all devices.
    inline static std::shared_ptr<Room> s_room = nullptr;
};

/// @brief Device base class with CRTP. All devices in this project should extend from it.
/// It makes more sense for devices to affect `Room` directly, instead of the eaiser design:
/// `SmartManager` collects all updates and modifies the world.
///
/// With CRTP (Curiously Recurring Template Pattern), we can replace traditionaly virtual function
/// override with static_cast for `Derived` and dynamic_pointer_cast for `DataType`
/// @tparam Derived e.g. RealAC
/// @tparam DataType e.g. RealACData
template <typename Derived, typename DataType>
class Device : public DeviceInterface {
    typedef std::variant<DeviceDataBase::MfId, typename DataType::OpId> VariantType;
    typedef std::pair<std::chrono::system_clock::time_point, std::string> LogEntry;

public:
    /// @brief Constructor
    /// @param name device name, should be unique.
    Device(std::string name) : m_name(name + "_" + std::to_string(s_global_id)), m_on(true) {
        s_total_count++;
        s_global_id++;
    }

#pragma region LogisticFunc
    /// @brief
    /// @return device name
    std::string getName() const override { return m_name; }

    inline std::string getCurrentTime() const override {
        return std::format("{:%T}", std::chrono::system_clock::now());
    }

    /// @brief DataType::OpId is a ​dependent name​ (on DataType). We need to use the typename
    /// keyword to tell the compiler that DataType::OpId refers to a type.
    inline typename DataType::OpId getOpId(const std::shared_ptr<DeviceDataBase> data) const {
        // std::get will throw error for us
        return std::get<typename DataType::OpId>(convertData(data)->id);
    }

    inline DeviceDataBase::MfId getMfId(const std::shared_ptr<DeviceDataBase> data) const {
        // std::get will throw error for us
        return std::get<DeviceDataBase::MfId>(convertData(data)->id);
    }

    void addMalfunctionLog(std::string&& log, DeviceDataBase::MfId mf_id) const;
    void addOperationLog(std::string&& log, typename DataType::OpId op_id) const;
    void addMiscLog(std::string&& log, std::string identifier) const;
    /// @brief Sort vector of LogEntry in chronological order.
    /// Will it ever be used? (Will we somehow create unchronological log?)
    void sortLog() { std::sort(m_log_system.begin(), m_log_system.end(), std::less<>{}); }
    /// @brief Log what has been done up to now.
    void printLog() const override {
        // header
        std::cout << std::string(20, '=') << std::format("{} at {}", getName(), getCurrentTime())
                  << std::string(20, '=') << std::endl;
        // content
        for (auto& [time, log_str] : m_log_system) {
            std::cout << log_str << std::endl;
        }
    }

    /// @brief Should better be called before creating any Device instance.
    static void loginRoom(std::shared_ptr<Room> room) { s_room = room; }
#pragma endregion

#pragma region VirtualFunc
    void run(std::shared_ptr<DeviceDataBase> data = nullptr) override;

    /// @brief Simulate time elapsing and update Device accordingly.
    /// It supports partial update for Device with relevant data.
    /// @example For `RealAC` that will open for 10s and go 25->20 degree, simulte for 6s will
    /// result in temperature updated to 22 degree.
    /// @example For `WasherDryer` that doesn't have "gradual" data like temperature, update is
    /// all or nothing except for the `Timer`.
    /// @param duration_sec If set to 0, the device should simulate till the finish of current
    /// opeation. Otherwise it simulate for exactly `duration_sec` seconds.
    /// @return How long we have simulated, equal to `duration_sec` if it != 0.
    uint32_t timeTravel(const uint32_t duration_sec) override {
        // CRTP magic: call derived implementation
        return static_cast<Derived*>(this)->implTimeTravel(duration_sec);
    }

    virtual ~Device() { s_total_count--; }
#pragma endregion

protected:
    std::string m_name = "NULL";
    bool m_on = false;
    // const function can modify log
    mutable std::vector<LogEntry> m_log_system;
    // increment only
    inline static uint32_t s_global_id = 0;
    // increment & decrement
    inline static uint32_t s_total_count = 0;

    inline void questionNotNormal() const {
        addMalfunctionLog(
            "Philosophical question: If I run normally while malfunction, do I run correctly?",
            DeviceDataBase::MfId::eNormal
        );
    }

    inline void closeLowBattery() {
        addMalfunctionLog("Low battery, closed!", DeviceDataBase::MfId::eLowBattery);
        m_on = false;
    }

    inline bool processCommonMf(DeviceDataBase::MfId mf_id) {
        if (mf_id == DeviceDataBase::MfId::eNormal) {
            questionNotNormal();
            return true;
        } else if (mf_id == DeviceDataBase::MfId::eLowBattery) {
            closeLowBattery();
            return true;
        }
        return false;
    }

    /// @brief A universal malfunction corresponding to MfId::eHacked,
    /// replace the first `len` char of `m_name` with `newName`.
    /// `E.g. "DemoDevice_1".replace(0 /* from beginning */, 4, "Bad") = "BadDevice_1";`
    /// @param newName
    /// @param len
    void hackName(std::string newName, size_t len) {
        // Hack the name from the beginning
        m_name.replace(0, len, newName);
        addMalfunctionLog(
            std::format("I got hacked and become {}.", getName()), DeviceDataBase::MfId::eHacked
        );
    }

    /// @brief Type-safe data conversion
    std::shared_ptr<DataType> convertData(std::shared_ptr<DeviceDataBase> data) const;
};

typedef std::vector<std::shared_ptr<DeviceDataBase>> DataList;
typedef std::unordered_map<std::string, std::shared_ptr<DeviceInterface>> DeviceMap;
typedef std::unordered_map<std::string, DataList> DataMap;
