# Step 1: Set minimum CMake version and project name
cmake_minimum_required(VERSION 3.13.4)
project(SimpleLoopLICM)

# Step 2: Locate the required LLVM package
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Step 3: Configure include paths and preprocessor definitions
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Step 4: Define the target library as a loadable MODULE
# This creates SimpleLoopLICM.so from LoopLICMPass.cpp
add_library(SimpleLoopLICM MODULE
    LoopLICMPass.cpp
)

# Step 5: Configure C++ standards and compiler flags
# -fno-rtti is mandatory for LLVM pass compatibility
set_target_properties(SimpleLoopLICM PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    COMPILE_FLAGS "-fno-rtti"
)

# Step 6: Configure linker flags
# We do NOT link against LLVM libraries directly; opt resolves symbols at runtime
set_target_properties(SimpleLoopLICM PROPERTIES
    LINK_FLAGS "-Wl,-znodelete"
)

# Step 7: Handle macOS specific linker requirements for dynamic lookup
if(APPLE)
    set_target_properties(SimpleLoopLICM PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()