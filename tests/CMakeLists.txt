cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Explicitly list the test source code and headers. The Catch header-only unit
# test framework is stored in with the test source.
set(SmartHome_TEST_SRC
    test_smart_home.cpp
)
set(SmartHome_TEST_HEADER
    catch.hpp
)

PREPEND(SmartHome_TEST_SRC)

# Make an executable target that depends on the test source code we specified
# above.
add_executable(TestSmartHome ${SmartHome_TEST_SRC} ${SmartHome_TEST_HEADER})

# Enable testing via CTest
enable_testing()
# Add our test as runnable via CTest
add_test(NAME TestSmartHome  COMMAND TestSmartHome)

# Link our unit tests against the library we compiled
target_link_libraries(TestSmartHome SmartHome)