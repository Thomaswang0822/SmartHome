#include "device.hpp"

#include "air_fryer.hpp"
#include "demo_device.hpp"
#include "real_ac.hpp"
#include "smart_manager.hpp"
#include "washer_dryer.hpp"

/// per-return-type macro alias at the top of the implementation file
/// They are shorter but less straightforward.
#define VOID_FUNC                                                                                  \
    template <typename Derived, typename DataType>                                                 \
    void Device<Derived, DataType>

#define RET_FUNC(type)                                                                             \
    template <typename Derived, typename DataType>                                                 \
    type Device<Derived, DataType>

// RET_FUNC(std::shared_ptr<DataType>)::convertData(std::shared_ptr<DeviceDataBase> data) const {
//     auto derived_data = std::dynamic_pointer_cast<DataType>(data);
//     Debug::logAssert(
//         derived_data != nullptr,
//         "Should get {}, but got {}",
//         typeid(DataType).name(),
//         data->getDataType()
//     );
//     return derived_data;
// }

template <typename Derived, typename DataType>
void Device<Derived, DataType>::addMalfunctionLog(
    std::string&& log, DeviceDataBase::MfId mf_id
) const {
    auto curr = std::chrono::system_clock::now();
    m_log_system.emplace_back(
        std::make_pair(
            curr,
            std::format("Malfunction Log {} at {}:\n\t", magic_enum::enum_name(mf_id), curr) +
                std::move(log)
        )
    );
}

template <typename Derived, typename DataType>
void Device<Derived, DataType>::addOperationLog(
    std::string&& log, typename DataType::OpId op_id
) const {
    auto curr = std::chrono::system_clock::now();
    m_log_system.emplace_back(
        std::make_pair(
            curr,
            std::format("Operation Log {} at {}:\n\t", magic_enum::enum_name(op_id), curr) +
                std::move(log)
        )
    );
}

template <typename Derived, typename DataType>
void Device<Derived, DataType>::addMiscLog(std::string&& log, std::string identifier) const {
    auto curr = std::chrono::system_clock::now();
    m_log_system.emplace_back(
        std::make_pair(curr, std::format("{} Log at {}:\n\t", identifier, curr) + std::move(log))
    );
}

template <typename Derived, typename DataType>
void Device<Derived, DataType>::run(std::shared_ptr<DeviceDataBase> data) {
    if (data == nullptr) {
        std::cout << "I am a " << this->getName() << " and I do NOTHING!" << std::endl;
        return;
    }

    auto derived_data = convertData(data);
    // alternative to std::get_if or std::holds_alternative
    std::visit(
        [this, derived_data](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, typename DataType::OpId>) {
                // CRTP magic: call derived implementation
                static_cast<Derived*>(this)->implOperate(derived_data);
            } else if constexpr (std::is_same_v<T, DeviceDataBase::MfId>) {
                // CRTP magic: call derived implementation
                static_cast<Derived*>(this)->implMalfunction(derived_data);
            } else {
                // Compile-time error for unexpected types
                static_assert(false, "Unexpected type in variant");
            }
        },
        derived_data->id
    );
}

template <typename Derived, typename DataType>
std::shared_ptr<DataType>
Device<Derived, DataType>::convertData(std::shared_ptr<DeviceDataBase> data) const {
    auto derived_data = std::dynamic_pointer_cast<DataType>(data);
    Debug::logAssert(
        derived_data != nullptr,
        "Should get {}, but got {}",
        typeid(DataType).name(),
        data->getDataType()
    );
    return derived_data;
}

/// Explicit instantiations: without it, we would pass compile but get linking error.
/// Because compiler will NOT compile this cpp file if we don't specify:
/// "I will use Device with the following template args"
template class Device<DemoDevice, DemoDeviceData>;
template class Device<AirFryer, AirFryerData>;
template class Device<RealAC, RealACData>;
template class Device<WasherDryer, WasherDryerData>;