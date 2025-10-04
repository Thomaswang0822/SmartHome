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
    std::chrono::system_clock::time_point GetTime() { return std::chrono::system_clock::now(); }
    void LogTime() { std::cout << std::format("{}", GetTime()) << std::endl; }
    int GetTemp() { return m_temp; }
    void SetTemp(int temp) { m_temp = temp; }
    void LogTemp() {
        std::cout << std::format("Temperature is {} Celsius degree", m_temp) << std::endl;
    }

private:
    int m_temp;
    // std::shared_ptr<SmartManager> m_sm;
};