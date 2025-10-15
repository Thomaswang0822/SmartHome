# Make an explicit list of all source files in `SmartHome_INC`. This is important
# because CMake is not a build system: it is a build system generator. Suppose
# you add a file foo.cpp to src/ after running "cmake ..". If you set
# `SmartHome_INC` with `file(GLOB ... )`, this change is not passed to the makefile;
# the makefile doesn't know that foo.cpp exists and will not re-run cmake. Your
# collaborator's builds will fail and it will be unclear why. Whether you use
# file(GLOB ...) or not, you will need to re-run cmake, but with an explicit
# file list, you know beforehand why your code isn't compiling. 
set(SmartHome_INC
    utils.hpp
    device_data.hpp
    device.hpp
    air_fryer.hpp
    washer_dryer.hpp
    room.hpp
    real_ac.hpp
    smart_manager.hpp
)

# Form the full path to the source files...
PREPEND(SmartHome_INC)
# ... and pass the variable to the parent scope.
set(SmartHome_INC ${SmartHome_INC}  PARENT_SCOPE)