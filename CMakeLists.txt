# It's important to specify the minimum CMake version upfront required by
# CMakeLists.txt. This is so that a user can clearly understand the reason the 
# build will fail before the build actually occurs, instead of searching for the
# CMake function that was used that is causing the failure.
cmake_minimum_required(VERSION 3.25)

# Set the global package-wide C++ standard. This will be inherited by all
# targets specified in the project. One can also specify the C++ standard in a
# target-specific manner, using:
#   set_target_properties(foo PROPERTIES CXX_STANDARD 11)
# for a target foo
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the project name and version number. This allows for a user of your
# library or tool to specify a particular version when they include it, as in 
#   find_package(SmartHome 1.0 REQUIRED)
project(SmartHome VERSION 1.0)
set(SmartHome_VERSION 1.0)

# enable unit testing via "make test" once the code has been compiled.
include(CTest)

# Function to prepend the subdirectory to source files in subdirectories
function(PREPEND var )
   set(listVar "")
   foreach(f ${${var}})
       list(APPEND listVar "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
   endforeach(f)
   set(${var} "${listVar}" PARENT_SCOPE)
endfunction(PREPEND)

# After a normal build, we can specify the location of various outputs of the
# build. We put executables and static libraries outside the build directory in
# bin/ and lib/, respectively.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")

# Find packages on the system.
find_package(Threads REQUIRED)
find_package(range-v3 REQUIRED)
# find_package(Boost REQUIRED)

# Include source code and headers. This runs the CMakeLists.txt in each
# subdirectory. These can define their own libraries, executables, etc. as targets, 
# but here we define all exportable targets in the root CMakeLists.txt.
add_subdirectory(src)
add_subdirectory(include)

# Add the test directory. It is optional and can be disabled during with
#   cmake -DBUILD_TESTING=OFF ..
# To run unit tests produced here, we only need to run:
#   make test
# or
#   ctest 
# In case your tests are printing to console, you can view their output to
# stdout with:
#   ctest -V

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Add the library SmartHome as a target, with the contents of src/ and include/
# as dependencies.
add_library(SmartHome STATIC ${SmartHome_SRC} ${SmartHome_INC})
# Link deps to the LIBRARY
target_link_libraries(SmartHome PUBLIC 
    Threads::Threads
    range-v3::range-v3
)

# generator expressions are needed for the include directories, since installing 
# headers changes the include path.
# Specify that SmartHome requires the files located in the include/ directory at
# compile time. This would normally look like 
#   target_include_directories(SmartHome PUBLIC include/)
# PUBLIC means that other libraries including SmartHome should also include the
# directory include/.
# However, there is a catch. If we are installing the project in
# CMAKE_INSTALL_PREFIX, we can't specify include/ in the build directory: we have 
# copied the contents of include to CMAKE_INSTALL_PREFIX/include and we would
# like  other projects to include this directory instead of include/. The following
# CMake command handles this. $<BUILD_INTERFACE:...> and
# $<INSTALL_INTERFACE:...> are macros whose values change depending on if we are
# simply building the code or if we are installing it.
# So instead of:

# target_include_directories(SmartHome PUBLIC
#    # headers to include when building from source
#    $<BUILD_INTERFACE:${SmartHome_SOURCE_DIR}/include> 
#    $<BUILD_INTERFACE:${SmartHome_BINARY_DIR}/include> 

#    # headers to include when installing  
#    # (implicitly prefixes with ${CMAKE_INSTALL_PREFIX}).
#    $<INSTALL_INTERFACE:include> 
#    )

add_compile_definitions(MAGIC_ENUM_DEFAULT_ENABLE_ENUM_FORMAT=1)

# Make ./include and ./lib publicly available to anyone using the SmartHome library
target_include_directories(SmartHome PUBLIC 
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)
