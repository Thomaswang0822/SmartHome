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

    std::vector<std::shared_ptr<Device>> vec_devices;
    /// C++ equivalent of Python [ DemoDevice(i) for i in range(N) ]
    auto range_ids = std::ranges::iota_view{0u, N};
    auto range_devices = std::ranges::transform(
        range_ids,                       // input
        std::back_inserter(vec_devices), // output, intially empty
        [](size_t id) -> std::shared_ptr<Device> {
            return std::make_shared<DemoDevice>("DemoDevice_" + std::to_string(id));
        } // transform function
    );

#ifdef __cpp_lib_ranges_enumerate
    namespace enum_view = std::views;
    std::cout << "std::views::enumerate supported! Value: " << __cpp_lib_ranges_enumerate << "\n";
#else
    namespace enum_view = ranges::views;
    std::cout << "std::views::enumerate() NOT supported\n";
    std::cout << "We will use range-v3 instead.\n";
#endif

    const auto op_count = static_cast<uint32_t>(DemoDevice::DemoOpId::COUNT);
    const auto mf_count = static_cast<uint32_t>(DemoDevice::MalfuncId::COUNT);
    for (const auto& [index, device] : vec_devices | enum_view::enumerate) {
        device->Operate(index % op_count);
        device->Malfunction(index % mf_count);
    }

// This macro is defined by the compiler to indicate the C++ version
#if defined(__cplusplus)
    // Output the version of C++
    std::cout << "C++ version: " << __cplusplus << std::endl;
#else
    // Notify that the version could not be detected
    std::cout << 'C++ version could not be determined.' << std::endl;
#endif

    return 0;
}
