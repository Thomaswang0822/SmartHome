#pragma once

#include <chrono>
#include <format>
#include <iostream>
#include <memory>
#include <vector>

class World final {
public:
    World() = default;
    World(int temp) : m_temp(temp) {};

    // Getter and setter: time should be retrieved on-the-fly and not be stored.
    std::chrono::system_clock::time_point getTime() { return std::chrono::system_clock::now(); }
    void logTime() { std::cout << std::format("{}", getTime()) << std::endl; }
    int getTemp() { return m_temp; }
    void setTemp(int temp) { m_temp = temp; }
    void logTemp() {
        std::cout << std::format("Temperature is {} Celsius degree", m_temp) << std::endl;
    }

private:
    int m_temp;
    // std::shared_ptr<SmartManager> m_sm;
};