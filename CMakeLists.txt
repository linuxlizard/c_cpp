cmake_minimum_required(VERSION 3.7)
project(wanstat)

find_package(cpprestsdk REQUIRED)
find_package(Boost REQUIRED COMPONENTS system program_options regex)

add_executable(wanstat wanstat.cc)
target_compile_features(wanstat PUBLIC cxx_std_17)
target_link_libraries(wanstat PRIVATE cpprestsdk::cpprest)
target_link_libraries(wanstat PRIVATE ${Boost_LIBRARIES})