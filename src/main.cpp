#include "air_fryer.hpp"
#include "demo_device.hpp"
#include "device.hpp"
#include "smart_manager.hpp"
#include "washer_dryer.hpp"

#include <iostream>
#include <numeric>
#include <range/v3/view/enumerate.hpp>
#include <ranges>
#include <real_ac.hpp>
#include <vector>

static constexpr bool SHOULD_DEMO = false;
static constexpr size_t N = 10;
static constexpr float ROOM_TEMP = 25.f;
typedef std::vector<std::vector<std::shared_ptr<DeviceDataBase>>> NestedDeviceData;

/// @brief TEMP: hard-code device creation. Will be replaced by ConfigFile/Cmdline parsing.
/// @param vec Empty vector of Device shared_ptr to be populated.
static void populateDevices(std::vector<std::shared_ptr<DeviceInterface>>& vec) {
    if (!vec.empty())
        vec.clear();

    vec.push_back(std::make_shared<DemoDevice>("DemoBot"));

    vec.push_back(std::make_shared<AirFryer>(3.0f /* volume in liter */));

    vec.push_back(std::make_shared<WasherDryer>(10.f /* volume in liter */));

    vec.push_back(std::make_shared<RealAC>(2000 /* power in watt */));
}

static void populateTravelTimes(std::vector<uint32_t>& vec) {
    // no override for DemoDevice or AirFryer, so 0 is no_op.
    vec.push_back(0);
    vec.push_back(0);

    vec.push_back(10);

    // For RealAC, 0 is sim to end.
    vec.push_back(0);
}

/// @brief TEMP: hard-code creation of device operation data.
/// In reality, each data should be created on-the-fly by `SmartManager`.
/// @param vec Empty vector of DeviceDataBase shared_ptr to be populated.
static void populateData(NestedDeviceData& vec) {
    if (!vec.empty())
        vec.clear();

    // For DemoDevice
    {
        std::vector<std::shared_ptr<DeviceDataBase>> vdata;
        {
            auto data = std::make_shared<DeviceDataBase>();
            data->op_id = OpId::eHello;
            data->mf_id = DeviceDataBase::MfId::eBroken;
            vdata.push_back(data);
        }
        {
            auto data = std::make_shared<DeviceDataBase>();
            data->op_id = OpId::eSing;
            data->mf_id = DeviceDataBase::MfId::eNormal;
            vdata.push_back(data);
        }
        vec.push_back(vdata);
    }

    // For AirFryer, int and float are time and volume of food to cook
    {
        std::vector<std::shared_ptr<DeviceDataBase>> vdata;
        {
            auto data = std::make_shared<AirFryerData>();
            data->op_id = OpId::eAirFryerCook;
            data->mf_id = DeviceDataBase::MfId::eNormal;
            data->time_sec = 5;
            data->food_volume = 2.0f;
            vdata.push_back(data);
        }
        {
            auto data = std::make_shared<AirFryerData>();
            data->op_id = OpId::eAirFryerClean;
            data->mf_id = DeviceDataBase::MfId::eLowBattery;
            vdata.push_back(data);
        }
        vec.push_back(vdata);
    }

    // For WahserDryer, int and float are time and volume of cloth
    {
        std::vector<std::shared_ptr<DeviceDataBase>> vdata;
        {
            auto data = std::make_shared<WasherDryerData>();
            data->op_id = OpId::eWashDryerDryOnly;
            data->mf_id = DeviceDataBase::MfId::eNormal;
            data->dry_sec = 3;
            data->cloth_volume = 8.0f;
            vdata.push_back(data);
        }
        {
            auto data = std::make_shared<WasherDryerData>();
            data->op_id = OpId::eWashDryerWashOnly;
            data->mf_id = DeviceDataBase::MfId::eNormal;
            data->wash_sec = 5;
            data->cloth_volume = 9.0f;
            vdata.push_back(data);
        }
        {
            auto data = std::make_shared<WasherDryerData>();
            data->op_id = OpId::eWashDryerCombo;
            data->mf_id = DeviceDataBase::MfId::eHacked;
            data->wash_sec = 7;
            data->dry_sec = 11;
            data->cloth_volume = 10.0f;
            vdata.push_back(data);
        }
        vec.push_back(vdata);
    }

    // For RealAC, float, int, bool, string are target temperature, duration (mins but actually
    // executed in secs), heat or not, mode.
    {
        std::vector<std::shared_ptr<DeviceDataBase>> vdata;
        {
            // 2000w in low is 500w, +3 degree needs 10 mins
            auto data = std::make_shared<RealACData>();
            data->op_id = OpId::eRealAcOpenTillDeg;
            data->mf_id = DeviceDataBase::MfId::eNormal;
            // data->duration_sec = 10;
            data->target_temp = ROOM_TEMP + 3.0f;
            data->is_heat = true;
            data->mode = RealACData::Mode::eLow;
            vdata.push_back(data);
        }
        {
            // 2000w in mid is 1000w, for 5 mins, expect -= 3 degree
            auto data = std::make_shared<RealACData>();
            data->op_id = OpId::eRealAcOpenForMins;
            data->mf_id = DeviceDataBase::MfId::eNormal;
            data->duration_sec = 5;
            // data->target_temp = ROOM_TEMP + 3.0f;
            data->is_heat = false;
            data->mode = RealACData::Mode::eMid;
            vdata.push_back(data);
        }
        vec.push_back(vdata);
    }
}

static void demo() {
    // Get C++ standard
#if defined(__cplusplus)
    std::cout << "C++ version: " << __cplusplus << std::endl;
#else
    std::cout << 'C++ version could not be determined.' << std::endl;
#endif

    // std::iota: C++ equivalent of Python list(range(N))
    std::vector<uint32_t> vec_ids(N);
    std::iota(vec_ids.begin(), vec_ids.end(), 0);
    for (auto id : vec_ids) {
        std::cout << id << "  ";
    }
    std::cout << std::endl;

    // create our room first: creation in main until we implement SmartManager
    std::shared_ptr<Room> sp_room = std::make_shared<Room>(ROOM_TEMP);
    DeviceInterface::loginRoom(sp_room);

    // std::ranges::transform: C++ equivalent of Python [ DemoDevice(i) for i in range(N) ]
    std::vector<std::shared_ptr<DeviceInterface>> vec_devices;
    auto range_ids = std::ranges::iota_view{0u, N};
    std::ranges::transform(
        range_ids,                       // input
        std::back_inserter(vec_devices), // output, intially empty
        [](uint32_t id) -> std::shared_ptr<DeviceInterface> {
            if (id < static_cast<uint32_t>(OpId::COUNT))
                return std::make_shared<AirFryer>();
            else
                return std::make_shared<DemoDevice>("DemoDevice_" + std::to_string(id));
        } // transform lambda function
    );

    // std::views::enumerate(): C++ equivalent of Python for i, element in enumerate(listA)
#ifdef __cpp_lib_ranges_enumerate
    namespace enum_view = std::views;
    std::cout << "std::views::enumerate() supported! Value: " << __cpp_lib_ranges_enumerate << "\n";
#else
    namespace enum_view = ranges::views;
    std::cout << "std::views::enumerate() NOT supported. We will use range-v3 instead.\n";
#endif

    constexpr auto OP_COUNT = static_cast<uint32_t>(OpId::COUNT);
    constexpr auto MF_COUNT = static_cast<uint32_t>(DeviceDataBase::MfId::COUNT);
    auto data = std::make_shared<DeviceDataBase>();
    auto af_data = std::make_shared<AirFryerData>();
    for (const auto& [index, device] : vec_devices | enum_view::enumerate) {
        std::cout << std::string(20, '=')
                  << std::format("{} at {}", device->getName(), device->getCurrentTime())
                  << std::string(20, '=') << std::endl;

        if (auto af = std::dynamic_pointer_cast<AirFryer>(device)) {
            af_data->op_id = static_cast<OpId>(index % OP_COUNT);
            af_data->mf_id = static_cast<DeviceDataBase::MfId>(index % MF_COUNT);
            af_data->time_sec = static_cast<int>(index); // seconds
            af_data->food_volume = static_cast<float>(index) + 0.1f;

            device->operate(af_data);
            device->malfunction(af_data);
        } else {
            data->op_id = static_cast<OpId>(index % OP_COUNT);
            data->mf_id = static_cast<DeviceDataBase::MfId>(index % MF_COUNT);

            device->operate(data);
            device->malfunction(data);
        }
        std::cout << std::string(20, '=') << std::endl;
    }

    /// std::views::zip(): C++ equivalent of Python for k1, k2 in zip(listA, listB)
#ifdef __cpp_lib_ranges_zip
    namespace zip_view = std::views;
    std::cout << "std::views::zip() supported! Value: " << __cpp_lib_ranges_zip << "\n";
#else
    namespace zip_view = ranges::views;
    std::cout << "std::views::zip() NOT supported. We will use range-v3 instead.\n";
#endif

    NestedDeviceData all_data;
    populateData(all_data);
    std::vector<uint32_t> travel_times;
    populateTravelTimes(travel_times);
    for (const auto& [device, vdata, ttime] : zip_view::zip(vec_devices, all_data, travel_times)) {
        std::cout << std::string(20, '=')
                  << std::format("{} at {}", device->getName(), device->getCurrentTime())
                  << std::string(20, '=') << std::endl;
        // Operate
        for (const auto d : vdata) {
            device->operate(d);
            device->malfunction(d);
        }
        device->timeTravel(ttime);
        /// Here we only demo how pointer cast works. The best practice is NOT to:
        /// define any public function particular for a derived class
        /// then call function by pointer_cast to derived class.
        if (auto wd = std::dynamic_pointer_cast<WasherDryer>(device)) {
            wd->getCurrentTime();
        }

        // Then Log
        for (const auto d : vdata) {
            device->logOperation(d);
        }
    }
}

int main() {
    if (SHOULD_DEMO)
        demo();

    // Create SmartManager and connect Room to it
    std::shared_ptr<Room> sp_room = std::make_shared<Room>(ROOM_TEMP);
    std::shared_ptr<SmartManager> sp_manager = std::make_shared<SmartManager>();
    sp_manager->connectToRoom(std::move(sp_room));

    // prepare data
    std::vector<std::shared_ptr<DeviceInterface>> vec_devices;
    populateDevices(vec_devices);
    NestedDeviceData all_data;
    populateData(all_data);
    std::vector<uint32_t> travel_times;
    populateTravelTimes(travel_times);

    /// std::views::zip(): C++ equivalent of Python for k1, k2 in zip(listA, listB)
#ifdef __cpp_lib_ranges_zip
    namespace zip_view = std::views;
    std::cout << "std::views::zip() supported! Value: " << __cpp_lib_ranges_zip << "\n";
#else
    namespace zip_view = ranges::views;
    std::cout << "std::views::zip() NOT supported. We will use range-v3 instead.\n";
#endif

    for (const auto& [device, vdata, ttime] : zip_view::zip(vec_devices, all_data, travel_times)) {
        auto name = device->getName();

        sp_manager->addDevice(std::move(device));
        sp_manager->addMultipleData(name, std::move(vdata));
        sp_manager->addTravleTime(name, std::move(ttime));

        Debug::logAssert(device == nullptr, "device == nullptr");
        Debug::logAssert(vdata.size() == 0, "vdata.size() == 0");
    }

    sp_manager->operate();

    return 0;
}
