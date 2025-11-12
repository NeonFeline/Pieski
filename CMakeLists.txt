cmake_minimum_required(VERSION 3.10)
project(PathCostCalculator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set O3 optimization level
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# For debugging, you might also want to include debug symbols with -g
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

# Alternatively, you can set optimization per target:
# set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-O3")

add_executable(test testing/test.cpp)

# If you want to set optimization specifically for this target:
# target_compile_options(main PRIVATE -O3)
