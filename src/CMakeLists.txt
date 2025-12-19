cmake_minimum_required(VERSION 3.13.4)
project(SimpleLoopLICM)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# 這裡改用 LoopLICMPass.cpp
add_library(SimpleLoopLICM MODULE
    LoopLICMPass.cpp
)

set_target_properties(SimpleLoopLICM PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    COMPILE_FLAGS "-fno-rtti"
)

# Important: Do NOT link against LLVM libraries
# The symbols will be resolved at runtime by opt
set_target_properties(SimpleLoopLICM PROPERTIES
    LINK_FLAGS "-Wl,-znodelete"
)

if(APPLE)
    set_target_properties(SimpleLoopLICM PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()