// Placeholder content

#include "device.hpp"

#include <chrono>
#include <format>
#include <iostream>

void Device::HackName(std::string newName) {
    // Hack the name from the beginning
    m_name.replace(0, newName.length(), newName);
    std::cout << "I got hacked and become " << GetName() << std::endl;
}

void DemoDevice::Operate(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->op_id) {
    case DeviceOpId::eHello:
        std::cout << this->Hello() << std::endl;
        break;
    case DeviceOpId::eSing:
        this->Sing();
        break;
    default:
        // DeviceOpId::eDefault
        Device::Operate();
        break;
    }
    return;
}

void DemoDevice::Malfunction(std::shared_ptr<DeviceData> data) {
    if (data == nullptr || !m_on)
        return;

    switch (data->mf_id) {
    case DeviceMfId::eLowBattery:
        m_on = false;
        break;
    case DeviceMfId::eHacked:
        // replace "Demo" with "Evil"
        HackName("Evil");
        break;
    case DeviceMfId::eBroken:
        std::cout << GetName() << " is broken!" << std::endl;
        break;
    default:
        // eNormal
        Device::Malfunction();
        break;
    }
}

std::string DemoDevice::Hello() const {
    // std::format doesn't accept high_resolution_clock as it's not intended for display
    const std::string message = std::format(
        "Hello World! This is device {}, greeting at {}!",
        GetName(),
        std::chrono::system_clock::now()
    );
    return message;
}

void DemoDevice::Sing() const {
    std::cout << GetName() << ": 哈吉米, 哈吉米, 哈吉米, 哈吉米南北绿豆！" << std::endl;
}
