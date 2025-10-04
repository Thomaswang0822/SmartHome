#include "air_fryer.hpp"
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
        [](uint32_t id) -> std::shared_ptr<Device> {
            if (id < static_cast<uint32_t>(DeviceOpId::COUNT))
                return std::make_shared<AirFryer>();
            else
                return std::make_shared<DemoDevice>("DemoDevice_" + std::to_string(id));
        } // transform function
    );

#ifdef __cpp_lib_ranges_enumerate
    namespace enum_view = std::views;
    std::cout << "std::views::enumerate supported! Value: " << __cpp_lib_ranges_enumerate << "\n";
#else
    namespace enum_view = ranges::views;
    std::cout << "std::views::enumerate() NOT supported.\t";
    std::cout << "We will use range-v3 instead.\n";
#endif

    constexpr auto op_count = static_cast<uint32_t>(DeviceOpId::COUNT);
    constexpr auto mf_count = static_cast<uint32_t>(DeviceMfId::COUNT);
    auto data = std::make_shared<DeviceData>();
    for (const auto& [index, device] : vec_devices | enum_view::enumerate) {
        data->op_id = static_cast<DeviceOpId>(index % op_count);
        data->mf_id = static_cast<DeviceMfId>(index % mf_count);
        data->dint = static_cast<int>(index) * 1000;
        data->dfloat = static_cast<float>(index) + 0.1f;

        device->Operate(data);
        device->Malfunction(data);

        std::cout << std::string(20, '=') << std::endl;
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
