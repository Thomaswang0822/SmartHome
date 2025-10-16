#pragma once

#include "device_data.hpp"
#include "room.hpp"
#include "utils.hpp"

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

    virtual void operate(std::shared_ptr<DeviceDataBase> data) = 0;
    virtual void malfunction(std::shared_ptr<DeviceDataBase> data) = 0;
    virtual uint32_t timeTravel(uint32_t duration_sec = 0) = 0;
    virtual void logOperation(const std::shared_ptr<DeviceDataBase> data = nullptr) const = 0;
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
public:
    /// @brief Constructor
    /// @param name device name, should be unique.
    Device(std::string name) : m_name(name + "_" + std::to_string(s_global_id)), m_on(true) {
        s_total_count++;
        s_global_id++;
    }

    /// @brief
    /// @return device name
    std::string getName() const override { return m_name; }

    std::string getCurrentTime() const override {
        return std::format("{:%T}", std::chrono::system_clock::now());
    }

    /// @brief Log what has been done in an `operate()` which is stored in `data->log_str`.
    /// @param data
    void logOperation(const std::shared_ptr<DeviceDataBase> data) const override {
        if (data == nullptr) {
            std::cout << std::format("Empty log: I have done nothing!\n");
        } else {
            std::cout << std::format(
                "{} log: {}\n", magic_enum::enum_name(data->op_id), data->log_str
            );
        }
    }

    /// @brief Should better be called before creating any Device instance.
    static void loginRoom(std::shared_ptr<Room> room) { s_room = room; }

    // BEGIN virtual functions

    /// @brief Simulate how the device behave when function properly
    /// @param op_id Identify which operations to be performed, because there can be many.
    void operate(std::shared_ptr<DeviceDataBase> data = nullptr) override {
        if (data == nullptr || data->op_id == OpId::eDefault) {
            std::cout << "I am a " << this->getName() << " and I do NOTHING!" << std::endl;
        } else {
            // CRTP magic: call derived implementation
            auto derived_data = convertData(data); // nullptr if inconsistent cast
            Debug::logAssert(
                derived_data != nullptr,
                "Should get {}, but got {}",
                typeid(DataType).name(),
                data->getDataType()
            );
            static_cast<Derived*>(this)->implOperate(derived_data);
        }
    }

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

    /// @brief Simulate how the device behave when function incorrectly
    /// @param mf_id Identify which operations to be performed, because there can be many.
    void malfunction(std::shared_ptr<DeviceDataBase> data = nullptr) override {
        if (data == nullptr || data->mf_id == DeviceDataBase::MfId::eNormal) {
            std::cerr << std::format("Philosophical question from {}: ", getName())
                      << "If I run normally while malfunction, do I run correctly or incorrectly?"
                      << std::endl;
        } else {
            // CRTP magic: call derived implementation
            auto derived_data = convertData(data); // nullptr if inconsistent cast
            Debug::logAssert(
                derived_data != nullptr,
                "Should get {}, but got {}",
                typeid(DataType).name(),
                data->getDataType()
            );
            static_cast<Derived*>(this)->implMalfunction(derived_data);
        }
    }

    virtual ~Device() { s_total_count--; }

protected:
    std::string m_name = "NULL";
    bool m_on = false;
    // increment only
    inline static uint32_t s_global_id = 0;
    // increment & decrement
    inline static uint32_t s_total_count = 0;

    /// @brief A universal malfunction corresponding to MfId::eHacked,
    /// replace the first `len` char of `m_name` with `newName`.
    /// `E.g. "DemoDevice_1".replace(0 /* from beginning */, 4, "Bad") = "BadDevice_1";`
    /// @param newName
    /// @param len
    void hackName(std::string newName, size_t len) {
        // Hack the name from the beginning
        m_name.replace(0, len, newName);
        std::cerr << "I got hacked and become " << getName() << std::endl;
    }

    /// @brief Type-safe data conversion
    std::shared_ptr<DataType> convertData(std::shared_ptr<DeviceDataBase> data) {
        return std::dynamic_pointer_cast<DataType>(data);
    }
};

typedef std::vector<std::shared_ptr<DeviceDataBase>> DataList;
typedef std::unordered_map<std::string, std::shared_ptr<DeviceInterface>> DeviceMap;
typedef std::unordered_map<std::string, DataList> DataMap;
