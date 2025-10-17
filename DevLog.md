# SmartHome Project Development Log

This development log can serve as an extended version of git commit messages.

## Step 1: Start a Skeleton CMake Project from a Template

Commit: **c085da4**

The CMake skeleton was adapted from [a very helpful and educative blog](https://mjmorse.com/blog/cmake-template/).

For now there isn't any non-trivial adaptation: we simply replace their project name "CMakeDemo" with our "SmartHome" and remove those cmake lines about INSTALL.

## Step 2: CMake Revision and Modern (C++ 20) Features

Commit: **03c5639**

The original template was designed for a library-like project: files are compiled, "installed", and ready to be used in another project. What we have is a traditional single-executable project with no shipping need for now.

Learned more about orchestration done by the root CMakelists.txt from AI. Especially, I learned how it communicates with CMakelists.txt in subdir. This is very important when dealing with complex real-world CMake projects: those with CMakeLists at root and nested subdir, generated to a VS solution containing multiple projects, multiple executables.

Also, I learned and played with `std::iota()` and `std::ranges::transform()`. Hoestly, the code is no shorter or easier to write than a traditional for loop, but it's worth trying out new features covered in our lectures.

## Step 3: Device class Design & `enumerate()`

Commit: **0a7934f**

Turn the `Operate()` function to multi-purpose:

```cpp
    /// @brief Simulate how the device behave when function properly
    /// @param op_id Identify which operations to be performed, because there can be many.
    virtual void Operate(uint32_t op_id = 0) = 0;
```

And the actual device class can define its own `enum` to specify a list of operations:

```cpp
class DemoDevice : Device {
public:
    DemoDevice(std::string name) : Device(name) {};
    /// @brief Operate() overridden by DemoDevice
    /// @param op_id Identify which operations to be performed, because there can be many.
    void Operate(uint32_t op_id = 0) override;

    /// @brief Different normal operations of DemoDevice
    enum class DemoOpId : uint32_t {
        eDefault = 0,
        eHello = 1, // call Hello()
        eSing = 2, // call Sing()

        COUNT,
    };

private:
    std::string Hello() const;
    void Sing() const;
};
```

Also, try to achieve Python-style `for i, item in enumerate(listA):`
First I tried `std::views::enumerate` but it's not supported even after C++23 is turned on. This is confirmed by `#ifdef __cpp_lib_ranges_enumerate`.

Then I achieve the goal with `ranges::views::enumerate` from lib **range-v3**, which works the same way.

## Step 4: Real AirFryer Class & Unified `DeviceData`

Commit: **811a5f1**

`DeviceData` is a unified data stroage class for ALL device types, used as `std::shared_ptr` for both input and output.
It has general-purpose data of each common types: `int`, `float`, `string`, `bool`. For example, the `int dint` can represent cook time in second for `AirFryer::Cook()` or target temperature for `AirConditioner::setTemp()`. Other data have specific purpose, like operation success flag.

Also, we group all Operate enum and Malfunction enum into 2 unified enum class `DeviceOpId` and `DeviceMfId`.

Other small additions

- add static 3rd-party lib **magic_enum** to print enum class
- define a custom `Debug::Assert` that enables formated message.

## Step 5: Real WasherDryer Class & Async Enabled by Timer

Commit: **28d6c27**

```cpp
/// @brief A FIFO async Washer-Dryer twin.
/// Unlike AirFryer, WashDryer should act atomically:
/// User should NOT be able to add or take out cloth in the middle.
///
/// "Async" in the sense that `Operate()` only submits the wash/dry job and returns immediately.
/// `FinishAll()` should be called to really execute all waiting cloth in the bin when properly.
class WasherDryer : public Device {
    // Implementation
};
```

The atomic asynchronicity is achieved with the helper struct `Timer`, a reusable time check that does NOT simulate time elapsing.
i.e. it computes non-negative remaining time `total_time - (now - start_time)`. When submitting a wash/dry job, unless it's the first submission, a previously submitted job would take up the washer/dryer. We simulate the remaining time from `Timer` and mark it as completed, before submitting the current job to bin, a `std::deque` of `DeviceData` shared pointer.

We used multi-slot `std::deque` instead of single-slot `DeviceData` because later we want to enable multi-threading. i.e. the `SmartManager` may submit several jobs simultaneously.

Also, I gave a try to **range-v3** `ranges::views::zip()`, which is an alternaive of `std::views::zip()`.

## Step 6: RealAC & Room class, Simulation API, and Log with Time Point

Commit: **11c48e6**

`Room` is a simple class that currently only keeps track of room temperature. We store a `static shared_ptr<Room>` inside `Device` to make sure there is only a single `Room` instance, while all devices can potentially modify this instance.

`RealAC` is an air conditionor that also works async, but with simpler logic than `WasherDryer`. Users can either "open for N minutes" or "open till T degree". But both are async "submission" which does not guarantee full power-on for specified minutes.

We also added a new virtual function, `uint32_t timeTravel(const uint32_t duration_sec)`, that simulates elapsed time.
It will update `Device` accordingly, which includes "partical update" of gradually changed variables like temperature.

In the end, we added a `getCurrentTime()` helper to format `now()`. We use it to pin time point in the device operation log.

## Step 7: Exclusive Device Control by `SmartManager`

Commit: **19613d4**

The `SmartManager` itself isn't special. The special part is how we ensure exclusive control of all `Device` instances, such that users cannot directly access them where they are created (likely in `main()`).
We use rvalue in the function arg list and `std::move` to invalidate the original instance.

Another small fix is to move those feature demo code, like for `std::iota`, to a helper function and run it conditionally (on a global bool flag). This makes the log cleaner.

## (VERSION 2.0) Step 8: CRTP Polymorphism of Device

Commit: **aeaa2e9**

Previously, we use the traditional runtime polymorphism for our devices and define virtual functions like `operate()`, `malfunction()`, etc. In version 2.0, we swtiched to CRTP (Curiously Recurring Template Pattern) compile-time polymorphism.

First we define a pure virtual `DeviceInterface` and put ALL public virtual functions there. Then we have a `template <typename Derived, typename DataType> class Device : public DeviceInterface` CRTP device base. Finally, an actual derived device class looks like `class AirFryer : public Device<AirFryer, AirFryerData>`.

Then we can use `static_cast` to achieve polymorphism in a very different way.

```cpp
// In Device:
    uint32_t timeTravel(const uint32_t duration_sec) override {
        // CRTP magic: call derived implementation
        return static_cast<Derived*>(this)->implTimeTravel(duration_sec);
    }

// In WasherDryer, for example:
    void implOperate(std::shared_ptr<WasherDryerData> data);
    void implMalfunction(std::shared_ptr<WasherDryerData> data);
    uint32_t implTimeTravel(const uint32_t duration_sec);
```

Note that using CRTP doesn't necessarily runs faster than traditional virtual function override. We use it mainly for self-study purpose.

## Step 9: More Robust Execution Data and Formatted Execution Log

Commit: **TODO**

First, we redesign the operation identifiers. Previously, they are 2 universal `enum class OpId` and `enum class MfId`.
The problem is that though all devices share the same set of malfunction ids, they have different operation ids.

```cpp
enum class OpId : uint32_t {
    eDefault = 0,
    // for DemoDevice
    eHello = 1,
    eSing = 2,
    // for AirFryer
    eAirFryerCook = 3,
    eAirFryerClean = 4,
    // for WashDryer
    eWashDryerCombo = 5,
    eWashDryerWashOnly = 6,
    eWashDryerDryOnly = 7,
    // for RealAC
    eRealAcOpenTillDeg = 8,
    eRealAcOpenForMins = 9,

    COUNT,
};
```

In the old universal operation id, not only is it verbose (we prefixed device name to differentiate them), but also type-unsafe.
i.e. It's perfectly managable to pass `eWashDryerCombo` to a `RealAC`, and of course this is not what we want. Thus, we changed to device-local enum that contains its own ids.

Then we unified the device execution data `DeviceDataBase`. Now it only has a unique id of type `std::variant` of universal malfunction id and device-local operation id. Following that, we combine the 2 API's `operate()` and `malfunction()` into a `run()`,
which will unpack the variant id and call respective implementation function.
Now, we can freely trigger operations and malfunctions by setting the unique id only, instead of restricted by the "for each device in the device list" loop.

Finally we collect all the `cout` and `cerr` into a log system and give them a universal format. Now our log looks like:

```txt
Operation Log eRealAcOpenForMins at 2025-10-17 07:16:12.318554:
        openForMins() starts from 25 deg at 07:16:12.318528, set to cool on eMid, will run for 5 sec and reach 22 deg, but may be stopped early.
Malfunction Log eHacked at 2025-10-17 07:16:12.318597:
        RealAC_0 gets hacked! Burning everyone to death!
```
