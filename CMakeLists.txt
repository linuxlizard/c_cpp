cmake_minimum_required(VERSION 3.7)
project(wanstat)

find_package(cpprestsdk REQUIRED)
find_package(Boost REQUIRED COMPONENTS system program_options regex)

#https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
#-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic
set(MY_DEBUG_OPTIONS -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic)

add_executable(wanstat wanstat.cc)
target_compile_features(wanstat PUBLIC cxx_std_17)
target_link_libraries(wanstat PRIVATE cpprestsdk::cpprest)
target_link_libraries(wanstat PRIVATE ${Boost_LIBRARIES})

add_executable(wlansurvey wlansurvey.cc)
target_compile_features(wlansurvey PUBLIC cxx_std_17)
target_link_libraries(wlansurvey PRIVATE cpprestsdk::cpprest)
target_link_libraries(wlansurvey PRIVATE ${Boost_LIBRARIES})
#https://cmake.org/cmake/help/v3.13/command/target_compile_options.html#command:target_compile_options
#https://stackoverflow.com/questions/23995019/what-is-the-modern-method-for-setting-general-compile-flags-in-cmake/23995391#23995391
target_compile_options(wlansurvey PUBLIC $<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>)

add_executable(uri uri.cc)
target_compile_features(uri PUBLIC cxx_std_17)
target_link_libraries(uri PRIVATE ${Boost_LIBRARIES})

add_executable(opts opts.cc)
target_compile_features(opts PUBLIC cxx_std_17)
target_link_libraries(opts PRIVATE ${Boost_LIBRARIES})

add_executable(format format.cc)
target_compile_features(format PUBLIC cxx_std_17)
target_link_libraries(format PRIVATE ${Boost_LIBRARIES})

# command line invocation:
# cmake -DCMAKE_BUILD_TYPE=Debug ..
# cmake -DCMAKE_BUILD_TYPE=Release ..
