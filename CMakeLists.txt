cmake_minimum_required (VERSION 3.10)
project (Pacer)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Packages
# find_package (Threads REQUIRED)

# Common dependency target
add_library (pacer_core INTERFACE)

# Attach all the includes/libraries to the core
target_include_directories (pacer_core INTERFACE 
    include 
    include/hazard
    lib
)

# target_link_libraries (pacer_core INTERFACE 
#     Threads::Threads
# )

# Executables
add_executable (sender src/sender.cpp)
target_link_libraries (sender PRIVATE pacer_core)

add_executable (receiver src/receiver.cpp)
target_link_libraries (receiver PRIVATE pacer_core)

add_executable (emulator src/emulator.cpp)
target_link_libraries (emulator PRIVATE pacer_core)