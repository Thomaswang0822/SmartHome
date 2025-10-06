#include "air_fryer.hpp"
#include "device.hpp"
#include "washer_dryer.hpp"

#include <iostream>
#include <numeric>
#include <range/v3/view/enumerate.hpp>
#include <ranges>
#include <vector>

static constexpr size_t N = 10;
typedef std::vector<std::vector<std::shared_ptr<DeviceData>>> NestedDeviceData;

/// @brief TEMP: hard-code device creation. Will be replaced by ConfigFile/Cmdline parsing.
/// @param vec Empty vector of Device shared_ptr to be populated.
static void PopulateDevices(std::vector<std::shared_ptr<Device>>& vec) {
    if (!vec.empty())
        vec.clear();

    // base class Device is not pure virtual
    vec.push_back(std::make_shared<Device>("Device"));

    vec.push_back(std::make_shared<DemoDevice>("DemoBot"));

    vec.push_back(std::make_shared<AirFryer>(3.0f /* volume in liter */));

    vec.push_back(std::make_shared<WasherDryer>(10.f /* volume in liter */));
}

/// @brief TEMP: hard-code creation of device operation data.
/// In reality, each data should be created on-the-fly by `SmartManager`.
/// @param vec Empty vector of DeviceData shared_ptr to be populated.
static void PopulateData(NestedDeviceData& vec) {
    if (!vec.empty())
        vec.clear();

    // For device
    {
        std::vector<std::shared_ptr<DeviceData>> vdata;
        auto data0a = std::make_shared<DeviceData>();
        auto data0b = nullptr;
        vdata.push_back(data0a);
        vdata.push_back(data0b);

        vec.push_back(vdata);
    }

    // For DemoDevice
    {
        std::vector<std::shared_ptr<DeviceData>> vdata;
        {
            auto data = std::make_shared<DeviceData>();
            data->op_id = DeviceOpId::eHello;
            data->mf_id = DeviceMfId::eBroken;
            vdata.push_back(data);
        }
        {
            auto data = std::make_shared<DeviceData>();
            data->op_id = DeviceOpId::eSing;
            data->mf_id = DeviceMfId::eNormal;
            vdata.push_back(data);
        }
        vec.push_back(vdata);
    }

    // For AirFryer, int and float are time and volume of food to cook
    {
        std::vector<std::shared_ptr<DeviceData>> vdata;
        {
            auto data = std::make_shared<DeviceData>();
            data->op_id = DeviceOpId::eAirFryerCook;
            data->mf_id = DeviceMfId::eNormal;
            data->dint = 5;
            data->dfloat = 2.0f;
            vdata.push_back(data);
        }
        {
            auto data = std::make_shared<DeviceData>();
            data->op_id = DeviceOpId::eAirFryerClean;
            data->mf_id = DeviceMfId::eLowBattery;
            vdata.push_back(data);
        }
        vec.push_back(vdata);
    }

    // For WahserDryer, int and float are time and volume of cloth
    {
        std::vector<std::shared_ptr<DeviceData>> vdata;
        {
            auto data = std::make_shared<DeviceData>();
            data->op_id = DeviceOpId::eWashDryerDryOnly;
            data->mf_id = DeviceMfId::eNormal;
            data->dint = 5;
            data->dfloat = 8.0f;
            vdata.push_back(data);
        }
        {
            auto data = std::make_shared<DeviceData>();
            data->op_id = DeviceOpId::eWashDryerWashOnly;
            data->mf_id = DeviceMfId::eNormal;
            data->dint = 4;
            data->dfloat = 9.0f;
            vdata.push_back(data);
        }
        {
            auto data = std::make_shared<DeviceData>();
            data->op_id = DeviceOpId::eWashDryerCombo;
            data->mf_id = DeviceMfId::eHacked;
            data->dint = 3;
            data->dfloat = 10.0f;
            vdata.push_back(data);
        }
        vec.push_back(vdata);
    }
}

int main() {

#if defined(__cplusplus)
    std::cout << "C++ version: " << __cplusplus << std::endl;
#else
    std::cout << 'C++ version could not be determined.' << std::endl;
#endif

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

    /// Above is more for demo of tranditional for-loop alternative.
    /// When device type grows, this quickly becomes difficult to use.
    vec_devices.clear();
    PopulateDevices(vec_devices);

#ifdef __cpp_lib_ranges_enumerate
    namespace enum_view = std::views;
    std::cout << "std::views::enumerate() supported! Value: " << __cpp_lib_ranges_enumerate << "\n";
#else
    namespace enum_view = ranges::views;
    std::cout << "std::views::enumerate() NOT supported. We will use range-v3 instead.\n";
#endif

    constexpr auto op_count = static_cast<uint32_t>(DeviceOpId::COUNT);
    constexpr auto mf_count = static_cast<uint32_t>(DeviceMfId::COUNT);

    auto data = std::make_shared<DeviceData>();
    for (const auto& [index, device] : vec_devices | enum_view::enumerate) {
        data->op_id = static_cast<DeviceOpId>(index % op_count);
        data->mf_id = static_cast<DeviceMfId>(index % mf_count);
        data->dint = static_cast<int>(index); // seconds
        data->dfloat = static_cast<float>(index) + 0.1f;

        // device->Operate(data);
        // device->Malfunction(data);
        // std::cout << std::string(20, '=') << std::endl;
    }

#ifdef __cpp_lib_ranges_zip
    namespace zip_view = std::views;
    std::cout << "std::views::zip() supported! Value: " << __cpp_lib_ranges_zip << "\n";
#else
    namespace zip_view = ranges::views;
    std::cout << "std::views::zip() NOT supported. We will use range-v3 instead.\n";
#endif

    NestedDeviceData all_data;
    PopulateData(all_data);
    for (const auto [device, vdata] : zip_view::zip(vec_devices, all_data)) {
        std::cout << std::string(20, '=') << device->GetName() << std::string(20, '=') << std::endl;
        // Operate
        for (const auto d : vdata) {
            device->Operate(d);
            device->Malfunction(d);
        }
        if (auto wd = std::dynamic_pointer_cast<WasherDryer>(device)) {
            wd->FinishAll();
        }

        // Then Log
        for (const auto d : vdata) {
            device->LogOperation(d);
        }
    }

    return 0;
}
