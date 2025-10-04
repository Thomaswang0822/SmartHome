// Placeholder content

#include "device.hpp"

#include <chrono>
#include <format>
#include <iostream>

void DemoDevice::Operate(uint32_t op_id) {
    switch (static_cast<DemoOpId>(op_id)) {
    case DemoOpId::eHello:
        std::cout << this->Hello() << std::endl;
        break;
    case DemoOpId::eSing:
        this->Sing();
        break;
    default:
        // DemoOpId::eDefault
        std::cout << "I am a " << this->GetName() << " and I do NOTHING!" << std::endl;
        break;
    }
    return;
}

void DemoDevice::Malfunction(uint32_t mf_id) {
    switch (static_cast<MalfuncId>(mf_id)) {
    case MalfuncId::eLowBattery:
        m_on = false;
        break;
    case MalfuncId::eHacked:
        // replace "Demo" with "Evil"
        HackName("Evil");
        break;
    case MalfuncId::eBroken:
        std::cout << GetName() << " is broken!" << std::endl;
        break;
    default:
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

void DemoDevice::HackName(std::string newName) {
    // Hack the name from the beginning
    m_name.replace(0, newName.length(), newName);
    std::cout << "I got hacked and become " << GetName() << std::endl;
}
