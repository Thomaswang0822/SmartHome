#pragma once

#include <chrono>
#include <format>
#include <iostream>
#include <memory>
#include <vector>

class Room final {
public:
    Room() = default;
    Room(float temp) : m_temp(temp) {};

    // Getter and setter: time should be retrieved on-the-fly and not be stored.
    std::chrono::system_clock::time_point getTime() const {
        return std::chrono::system_clock::now();
    }
    void logTime() const { std::cout << std::format("{}", getTime()) << std::endl; }
    float getTemp() const { return m_temp; }
    void setTemp(float temp) { m_temp = temp; }
    void logTemp() const {
        std::cout << std::format("Temperature is {} Celsius degree", m_temp) << std::endl;
    }

private:
    float m_temp;
    // std::shared_ptr<SmartManager> m_sm;
};