# Make an explicit list of all source files in `SmartHome_SRC`. This is important
# because CMake is not a build system: it is a build system generator. Suppose
# you add a file foo.cpp to src/ after running "cmake ..". If you set
# `SmartHome_SRC` with `file(GLOB ... )`, this change is not passed to the makefile;
# the makefile doesn't know that foo.cpp exists and will not re-run cmake. Your
# collaborator's builds will fail and it will be unclear why. Whether you use
# file(GLOB ...) or not, you will need to re-run cmake, but with an explicit
# file list, you know beforehand why your code isn't compiling. 
set(SmartHome_SRC
    device.cpp
    air_fryer.cpp
    washer_dryer.cpp
    real_ac.cpp
    smart_manager.cpp
)

# Form the full path to the source files...
PREPEND(SmartHome_SRC)
# ... and pass the variable to the parent scope.
set(SmartHome_SRC ${SmartHome_SRC}  PARENT_SCOPE)

# List source files for the EXECUTABLE (just the entry point)
set(APP_SRCS
    main.cpp
)
PREPEND(APP_SRCS)
# Create the executable target.
add_executable(${APP_TARGET_NAME} ${APP_SRCS})

# To avoid being asked for TCC permissions each build
if(APPLE)
    find_program(CODESIGN_EXECUTABLE codesign)
    # TODO: change this per machine
    set(CERT_NAME "MacCodeSigning")
    
    if(CODESIGN_EXECUTABLE)
        add_custom_command(TARGET ${APP_TARGET_NAME} POST_BUILD
            COMMAND ${CODESIGN_EXECUTABLE} --force --deep -s ${CERT_NAME} "$<TARGET_FILE:${APP_TARGET_NAME}>"
            COMMENT "Signing ${APP_TARGET_NAME} with certificate \"${CERT_NAME}\" for macOS TCC permissions"
            VERBATIM
        )
    else()
        message(WARNING "codesign not found - macOS app won't be signed")
    endif()
endif()

# Link the executable to our internal library and its dependencies.
target_link_libraries(SmartHomeApp PRIVATE SmartHome) # Communication point 2
