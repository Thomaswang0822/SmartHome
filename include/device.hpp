#pragma once

#include "device_data.hpp"

#include <format>
#include <iostream>
#include <memory>
#include <string>

/// @brief Device base class. All devices in this project should extend from it.
class Device {
public:
    /// @brief Constructor
    /// @param name device name, should be unique.
    Device(std::string name) : m_name(name) {};

    /// @brief
    /// @return device name
    std::string GetName() const { return m_name; }

    // BEGIN virtual functions

    /// @brief Simulate how the device behave when function properly
    /// @param op_id Identify which operations to be performed, because there can be many.
    virtual void Operate(std::shared_ptr<DeviceData> data = nullptr) {
        if (data == nullptr || data->op_id == DeviceOpId::eDefault) {
            std::cout << "I am a " << this->GetName() << " and I do NOTHING!" << std::endl;
        }
    }

    /// @brief Simulate how the device behave when function incorrectly
    /// @param mf_id Identify which operations to be performed, because there can be many.
    virtual void Malfunction(std::shared_ptr<DeviceData> data = nullptr) {
        if (data == nullptr || data->mf_id == DeviceMfId::eNormal) {
            std::cout << std::format("Philosophical question from {}: ", GetName())
                      << "If I run normally while malfunction, do I run correctly or incorrectly?"
                      << std::endl;
        }
    }

    virtual ~Device() = default;

protected:
    std::string m_name;

    void HackName(std::string newName);
};

/// @brief A "better" placeholder class to demo
/// how an easiest-possible device works.
class DemoDevice : public Device {
public:
    DemoDevice(std::string name) : Device(name), m_on(true) {};
    /// @brief Operate() overridden by DemoDevice
    /// @param op_id Identify which operations to be performed, because there can be many.
    void Operate(std::shared_ptr<DeviceData> data) override;

    void Malfunction(std::shared_ptr<DeviceData> data) override;

private:
    // All normal operations
    std::string Hello() const;
    void Sing() const;

    // All malfunctions

    // Data
    bool m_on = false;
};
