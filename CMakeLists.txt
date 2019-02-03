# hint: command line invocation
# cmake -DCMAKE_BUILD_TYPE=Debug ..
# cmake -DCMAKE_BUILD_TYPE=Release ..

cmake_minimum_required(VERSION 3.7)
project(wanstat)

enable_testing()

# debian (work in progress):
# apt install build-essential libcpprest-dev libboost-program-options-dev libboost-filesystem-dev
#
# debian's cpprest doesn't install the .cmake
# https://github.com/Microsoft/cpprestsdk/issues/686
set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)

find_package(cpprestsdk REQUIRED)
find_package(Boost REQUIRED COMPONENTS system program_options regex filesystem unit_test_framework)

#https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
#-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic
set(MY_DEBUG_OPTIONS -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic)

add_executable(my_gps my_gps.cc)
target_compile_features(my_gps PUBLIC cxx_std_17)
target_link_libraries(my_gps PRIVATE cpprestsdk::cpprest)
target_link_libraries(my_gps PRIVATE ${Boost_LIBRARIES})
target_compile_options(my_gps PUBLIC $<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>)

add_library(netrc OBJECT netrc.cc)
target_compile_features(netrc PUBLIC cxx_std_17)
add_executable(test_netrc $<TARGET_OBJECTS:netrc> test_netrc.cc)
target_compile_features(test_netrc PUBLIC cxx_std_17)
if (${CMAKE_COMPILER_IS_GNUCXX})
	set(FSLIB "stdc++fs")
else()
	set(FSLIB "c++fs")
endif()
message(STATUS "Boost_Libraries=${Boost_LIBRARIES}")
target_link_libraries(test_netrc PRIVATE ${Boost_LIBRARIES} ${FSLIB})
target_compile_options(test_netrc PUBLIC $<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>)
# netrc is using filesystem library. An opportunity to learn more CMake!
# https://en.cppreference.com/w/cpp/filesystem
# "Using this library may require additional compiler/linker options. GNU
# implementation requires linking with -lstdc++fs and LLVM implementation
# requires linking with -lc++fs"
add_test(
	NAME test_netrc
	COMMAND $<TARGET_FILE:test_netrc>
	)

add_executable(wanstat $<TARGET_OBJECTS:netrc> $<TARGET_OBJECTS:opts> wanstat.cc)
target_compile_features(wanstat PUBLIC cxx_std_17)
target_link_libraries(wanstat PRIVATE cpprestsdk::cpprest)
target_link_libraries(wanstat PRIVATE ${Boost_LIBRARIES} ${FSLIB})
target_compile_options(wanstat PUBLIC $<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>)

add_executable(wlansurvey $<TARGET_OBJECTS:netrc> wlansurvey.cc)
target_compile_features(wlansurvey PUBLIC cxx_std_17)
target_link_libraries(wlansurvey PRIVATE cpprestsdk::cpprest)
target_link_libraries(wlansurvey PRIVATE ${Boost_LIBRARIES} ${FSLIB})
#https://cmake.org/cmake/help/v3.13/command/target_compile_options.html#command:target_compile_options
#https://stackoverflow.com/questions/23995019/what-is-the-modern-method-for-setting-general-compile-flags-in-cmake/23995391#23995391
target_compile_options(wlansurvey PUBLIC $<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>)


add_executable(uri uri.cc)
target_compile_features(uri PUBLIC cxx_std_17)
target_link_libraries(uri PRIVATE ${Boost_LIBRARIES})

add_library(opts OBJECT opts.cc)
target_compile_features(opts PUBLIC cxx_std_17)
add_executable(test_opts $<TARGET_OBJECTS:opts> test_opts.cc)
target_compile_features(test_opts PUBLIC cxx_std_17)
target_link_libraries(test_opts PRIVATE ${Boost_LIBRARIES})
target_compile_options(test_opts PUBLIC $<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>)
add_test(
	NAME test_opts
	COMMAND $<TARGET_FILE:test_opts>
	)

add_executable(format format.cc)
target_compile_features(format PUBLIC cxx_std_17)
target_link_libraries(format PRIVATE ${Boost_LIBRARIES})

add_executable(sort sort.cc)
target_compile_features(sort PUBLIC cxx_std_17)
target_link_libraries(sort PRIVATE ${Boost_LIBRARIES})

