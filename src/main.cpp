#include "device.hpp"

#include <iostream>
#include <numeric>
#include <range/v3/view/enumerate.hpp>
#include <ranges>
#include <vector>

static constexpr size_t N = 10;

int main() {
    std::vector<uint32_t> vec_ids(N);
    /// C++ equivalent of Python list(range(N))
    std::iota(vec_ids.begin(), vec_ids.end(), 0);

    for (auto id : vec_ids) {
        std::cout << id << "  ";
    }
    std::cout << std::endl;

    std::vector<std::shared_ptr<DemoDevice>> vec_devices;
    /// C++ equivalent of Python [ DemoDevice(i) for i in range(N) ]
    auto range_ids = std::ranges::iota_view{0u, N};
    auto range_devices = std::ranges::transform(
        range_ids,                       // input
        std::back_inserter(vec_devices), // output, intially empty
        [](size_t id) -> std::shared_ptr<DemoDevice> {
            return std::make_shared<DemoDevice>("DemoDevice_" + std::to_string(id));
        } // transform function
    );

#ifdef __cpp_lib_ranges_enumerate
    for (const auto& [index, device] : vec_devices | std::views::enumerate) {
        device->Operate(index % 2);
    }
    std::cout << "std::views::enumerate supported! Value: " << __cpp_lib_ranges_enumerate << "\n";
#else
    std::cout << "std::views::enumerate() NOT supported\n";
    std::cout << "We will use range-v3 instead.\n";

    const auto op_count = static_cast<uint32_t>(DemoDevice::DemoOpId::COUNT);
    for (const auto& [index, device] : vec_devices | ranges::views::enumerate) {
        device->Operate(index % op_count);
    }
#endif

    return 0;
}
