#pragma once

#include "device.hpp"

#include <concepts> // perfect forwarding template type check
#include <unordered_map>
#include <vector>

/// @brief `SmartManager` holds ALL `Device` and `DeviceDataBase` instance (shared_ptr) exclusively,
/// because we restrict users from accessing them directly.
/// In the end, they should be created in main() from config file or Cmdline args, but immediately
/// std::move() to and hold exclusively by `SmartManager`.
class SmartManager final {
public:
    /// @brief Transfer ownership of a `Device` to `SmartManager`
    /// @param device_ptr `Device` instance (will be MOVED FROM and invalidated)
    /// @return success
    bool addDevice(std::shared_ptr<DeviceInterface>&& device_ptr);

    /// @brief Transfer ownership of a single `DeviceDataBase` instance to `SmartManager`
    /// @param device_name `Device` identifier
    /// @param data_ptr `DeviceDataBase` instance (will be MOVED FROM and invalidated)
    /// @return success
    bool addSingleData(std::string device_name, std::shared_ptr<DeviceDataBase>&& data_ptr);

    /// @brief Transfer ownership of a list of `DeviceDataBase` to `SmartManager`
    /// @param device_name `Device` identifier
    /// @param data A vector of `DeviceDataBase` instances (will be MOVED FROM and invalidated)
    /// @return success
    bool addMultipleData(std::string device_name, DataList&& data);

    /// @brief Transfer ownership of a single uint32_t travel time to `SmartManager`
    /// @param device_name `Device` identifier
    /// @param ttime uint32_t travel time instance (will be MOVED FROM and invalidated)
    /// @return success
    bool addTravleTime(std::string device_name, uint32_t&& ttime);

    /// @brief Transfer ownership of a `Room` to `SmartManager`
    /// @param room `Room` instance (will be MOVED FROM and invalidated)
    void connectToRoom(std::shared_ptr<Room>&& room) {
        DeviceInterface::loginRoom(std::move(room));
    }

    void operate();

    size_t getNumDevices() const { return m_device_map.size(); }

private:
    std::vector<std::string> m_device_names;
    /// @brief Name to Device
    DeviceMap m_device_map;
    /// @brief Name to a list of DeviceDataBase
    DataMap m_data_map;
    /// @brief Name to `Device::timeTravel()` input
    std::unordered_map<std::string, uint32_t> m_ttime_map;
};