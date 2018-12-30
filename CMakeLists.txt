cmake_minimum_required (VERSION 3.10)
project (TrafficLights)
add_definitions(-std=c++1z)
#Can manually add the sources using the set command as follows:
#set(SOURCES src/mainapp.cpp src/Student.cpp)
#However, the file(GLOB...) allows for wildcard additions:
#file(GLOB SOURCES "src/*.cpp")
set(SOURCES ${SOURCES} src/TrafficLights.cpp)
add_executable(TrafficLights ${SOURCES} src/main.cpp)
add_compile_options(
  "-Wall" "-Wpedantic" "-Wextra" "-fexceptions"
  "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
)
target_link_libraries(TrafficLights glut GL GLU gtest pthread boost_system)
include_directories(include)

set(SOURCES2 ${SOURCES})
#set(SOURCES2 ${SOURCES2} src/XXXX.cpp)
add_executable(tests ${SOURCES2})
target_link_libraries(tests gtest_main gtest pthread boost_system)
