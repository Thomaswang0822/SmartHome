#include "device.h"

#include <iostream>
#include <numeric>
#include <ranges>
#include <vector>

static constexpr size_t N = 10;

int main() {
    std::vector<uint32_t> vec_ids(N);
    /// C++ equivalent of Python list(range(N))
    std::iota(vec_ids.begin(), vec_ids.end(), 0);

    for (auto id : vec_ids) {
        std::cout << id << std::endl;
    }

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

    for (auto& dev : vec_devices) {
        dev->Operate();
    }

    return 0;
}
