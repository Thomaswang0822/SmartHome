#include "smart_manager.hpp"

bool SmartManager::addDevice(std::shared_ptr<Device>&& device_ptr) {
    auto device_name = device_ptr->getName();
    if (m_device_map.contains(device_name)) {
        std::cerr << std::format("{} already exist in SmartManager device list.\n", device_name);
        return false;
    } else {
        m_device_names.push_back(device_name);
        m_device_map.insert({device_name, std::move(device_ptr)});
        return true;
    }
}

bool SmartManager::addSingleData(std::string device_name, std::shared_ptr<DeviceData>&& data_ptr) {
    if (!m_device_map.contains(device_name)) {
        std::cerr << std::format(
            "{} doesn't exist in SmartManager device list. Use addDevice() first.\n", device_name
        );
        return false;
    }

    /// [] operator gives default value (the empty vector in our case) when the key doesn't exist.
    m_data_map[device_name].push_back(std::move(data_ptr));
    return true;
}

bool SmartManager::addMultipleData(std::string device_name, DataList&& data) {
    // Always move elements (regardless of original value category)
    std::move(data.begin(), data.end(), std::back_inserter(m_data_map[device_name]));
    // Explicitly clear to emphasize invalidation (optional but clear)
    data.clear();
    return true;
}

bool SmartManager::addTravleTime(std::string device_name, uint32_t&& ttime) {
    if (!m_device_map.contains(device_name)) {
        std::cerr << std::format(
            "{} doesn't exist in SmartManager device list. Use addDevice() first.\n", device_name
        );
        return false;
    }

    m_ttime_map[device_name] = std::move(ttime);
    return true;
}

void SmartManager::operate() {
    if (m_device_map.empty()) {
        std::cout << "No device registered, thus nothing happened.\n";
        return;
    }

    for (auto& device_name : m_device_names) {
        auto& device = m_device_map[device_name];
        std::cout << std::string(20, '=')
                  << std::format("{} at {}", device->getName(), device->getCurrentTime())
                  << std::string(20, '=') << std::endl;

        if (!m_data_map.contains(device_name)) {
            // no operation for this device, see Device::logOperation()
            m_data_map[device_name].push_back(nullptr);
            continue;
        }

        for (auto& data : m_data_map[device_name]) {
            device->operate(data);
            device->malfunction(data);
        }

        device->timeTravel(m_ttime_map[device_name]);

        for (const auto& data : m_data_map[device_name]) {
            device->logOperation(data);
        }
    }

    return;
}
