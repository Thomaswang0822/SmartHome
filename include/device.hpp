#pragma once

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
    virtual void Operate(uint32_t op_id = 0) = 0;

    /// @brief Simulate how the device behave when function incorrectly
    /// @param mf_id Identify which operations to be performed, because there can be many.
    virtual void Malfunction(uint32_t mf_id) = 0;

    virtual ~Device() = default;

protected:
    std::string m_name;
};

/// @brief A "better" placeholder class to demo
/// how an easiest-possible device works.
class DemoDevice : public Device {
public:
    DemoDevice(std::string name) : Device(name), m_on(true) {};
    /// @brief Operate() overridden by DemoDevice
    /// @param op_id Identify which operations to be performed, because there can be many.
    void Operate(uint32_t op_id = 0) override;

    void Malfunction(uint32_t mf_id) override;

    /// @brief Different normal operations of DemoDevice
    enum class DemoOpId : uint32_t {
        eDefault = 0,
        eHello = 1,
        eSing = 2,

        COUNT,
    };

    enum class MalfuncId : uint32_t {
        eLowBattery = 0,
        eHacked = 1,
        eBroken = 2,

        COUNT,
    };

private:
    // All normal operations
    std::string Hello() const;
    void Sing() const;

    // All malfunctions
    void HackName(std::string newName);

    // Data
    bool m_on = false;
};
