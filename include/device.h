#pragma once

#include <string>

class Device {
public:
    Device(std::string name) : m_name(name) {};
    std::string GetName() const {
        return m_name;
    }
    virtual void Operate() = 0;

private:
    std::string m_name;
};

// A "better" placeholder class to be removed later
class DemoDevice : Device {
public:
    DemoDevice(std::string name) : Device(name) {};
    void Operate() override;

private:

};
