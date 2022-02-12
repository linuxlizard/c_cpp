# hint: command line invocation
# cmake -DCMAKE_BUILD_TYPE=Debug ..
# cmake -DCMAKE_BUILD_TYPE=Release ..
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# hint 20210716
# cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=~/src/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake_minimum_required(VERSION 3.7)
project(wanstat)

enable_testing()

# http://fmtlib.net/latest/usage.html
#add_subdirectory(fmt EXCLUDE_FROM_ALL)
#add_library(fmt CONFIG REQUIRED)

# debian (work in progress):
# apt install build-essential libcpprest-dev libboost-program-options-dev libboost-filesystem-dev libboost-test-dev libjsoncpp-dev libfmt3-dev
#
# debian's cpprest doesn't install the .cmake
# https://github.com/Microsoft/cpprestsdk/issues/686
set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)

# Fedora
# dnf install make gcc gcc-c++ kernel-devel git libmicrohttpd-devel pkg-config zlib-devel libnl3-devel flex bison openssl-devel
# dnf install cmake cpprest-devel boost-devel jsoncpp-devel # TODO keep up to date

find_package(cpprestsdk REQUIRED)
#find_package(Boost REQUIRED COMPONENTS system program_options regex filesystem )
find_package(Boost REQUIRED COMPONENTS system program_options regex filesystem unit_test_framework)

message(STATUS "Boost_FOUND=${BOOST_FOUND}")
message(STATUS "Boost_LIBRARIES=${Boost_LIBRARIES}")
message(STATUS "Boost_INCLUDE_DIRS=${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_VERSION_STRING=${Boost_VERSION_STRING}")

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
target_include_directories(netrc PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(test_netrc PUBLIC ${Boost_INCLUDE_DIRS})
if (${CMAKE_COMPILER_IS_GNUCXX})
	set(FSLIB "stdc++fs")
else()
	set(FSLIB "c++fs")
endif()

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
target_include_directories(wanstat PUBLIC ${Boost_INCLUDE_DIRS})
#target_link_libraries(wanstat PRIVATE fmt::fmt)

add_executable(wlansurvey $<TARGET_OBJECTS:netrc> wlansurvey.cc)
target_compile_features(wlansurvey PUBLIC cxx_std_17)
target_link_libraries(wlansurvey PRIVATE cpprestsdk::cpprest)
target_link_libraries(wlansurvey PRIVATE ${Boost_LIBRARIES} ${FSLIB})
#https://cmake.org/cmake/help/v3.13/command/target_compile_options.html#command:target_compile_options
#https://stackoverflow.com/questions/23995019/what-is-the-modern-method-for-setting-general-compile-flags-in-cmake/23995391#23995391
target_compile_options(wlansurvey PUBLIC $<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>)
target_include_directories(wlansurvey PUBLIC ${Boost_INCLUDE_DIRS})
#target_link_libraries(wlansurvey PRIVATE fmt::fmt)

add_library(uri OBJECT uri.cc)
target_compile_features(uri PUBLIC cxx_std_17)
add_executable(test_uri test_uri.cc)
target_compile_features(test_uri PUBLIC cxx_std_17)
target_link_libraries(test_uri PRIVATE uri ${Boost_LIBRARIES})
target_include_directories(test_uri PUBLIC ${Boost_INCLUDE_DIRS})

add_library(opts OBJECT opts.cc)
target_compile_features(opts PUBLIC cxx_std_17)
target_include_directories(opts PUBLIC ${Boost_INCLUDE_DIRS})

add_executable(test_opts $<TARGET_OBJECTS:opts> test_opts.cc)
target_compile_options(test_opts PUBLIC $<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>)
target_compile_features(test_opts PUBLIC cxx_std_17)
target_include_directories(test_opts PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(test_opts PRIVATE ${Boost_LIBRARIES})
add_test(
	NAME test_opts
	COMMAND $<TARGET_FILE:test_opts>
	)

# libfmt 
# http://fmtlib.net/latest/index.html

add_executable(format format.cc)
target_compile_features(format PUBLIC cxx_std_17)
target_link_libraries(format PRIVATE ${Boost_LIBRARIES})
#target_link_libraries(format PRIVATE fmt::fmt)

add_executable(sort sort.cc)
target_compile_features(sort PUBLIC cxx_std_17)
target_compile_options(sort PUBLIC $<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>)
target_link_libraries(sort PRIVATE ${Boost_LIBRARIES})
#target_link_libraries(sort PRIVATE fmt::fmt)

# jsoncpp jsoncpp-devel
# https://github.com/open-source-parsers/jsoncpp
#add_executable(json json.cc)
#target_compile_features(json PUBLIC cxx_std_17)
#target_link_libraries(json PRIVATE jsoncpp)

# stuff for http_client (boost::beast)
# hint:  
# 	pushd ~/src && git clone https://github.com/ReneNyffenegger/cpp-base64.git && popd
# 	CMAKE_PREFIX_PATH=~/src/ cmake ..
find_path(CPP_BASE64_INCLUDE_DIRS "cpp-base64/base64.cpp")
message(STATUS "CPP_BASE64_INCLUDE_DIRS=${CPP_BASE64_INCLUDE_DIRS}")
#find_package(nlohmann_json CONFIG REQUIRED)

# davep 20210730 ; convert wanstat to boost::beast
add_executable(http_client_sync $<TARGET_OBJECTS:netrc> http_client_sync.cc ${CPP_BASE64_INCLUDE_DIRS}/cpp-base64/base64.cpp)
target_include_directories(http_client_sync PRIVATE ${CPP_BASE64_INCLUDE_DIRS})
target_include_directories(http_client_sync PUBLIC ${Boost_INCLUDE_DIRS})
target_compile_features(http_client_sync PUBLIC cxx_std_17)
target_link_libraries(http_client_sync PRIVATE ${Boost_LIBRARIES})
#target_link_libraries(http_client_sync PRIVATE fmt::fmt)
target_link_libraries(http_client_sync PRIVATE Boost::boost)
#target_link_libraries(http_client_sync PRIVATE nlohmann_json nlohmann_json::nlohmann_json)
target_link_libraries(http_client_sync PRIVATE pthread)

# davep 20220211 ; move cfg80211 c++ from duwt to this project
add_library(duwt STATIC cfg80211/oui.cc cfg80211/mimetypes.cc cfg80211/survey.cc)
target_include_directories(duwt PUBLIC ${libnl_INCLUDE_DIRS} ${jsoncpp_INCLUDE_DIRS})
target_include_directories(duwt PUBLIC ${libnl_INCLUDE_DIRS} ${jsoncpp_INCLUDE_DIRS})
target_include_directories(duwt PUBLIC libiw)
target_compile_options(duwt PUBLIC $<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>)

add_executable(oui cfg80211/run_oui.cc cfg80211/oui.cc)

foreach(test IN ITEMS test_oui test_survey)
	message("test=${test}")
	# strip the "test_" from the test name to get the src we're trying to test
	string(REPLACE "test_" "" test_target ${test})
	message("test_target=${test_target}")

	add_executable(${test} cfg80211/${test}.cc cfg80211/${test_target}.cc cfg80211/${test}_main.cc)

	target_include_directories(${test} PUBLIC include)
	target_link_libraries(${test} duwt jansson pthread)
	target_include_directories(${test} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libiw)
	target_link_libraries(${test} iw ${libnl_LDFLAGS} ${libnl_genl_LDFLAGS} )

	add_test(NAME ${test} COMMAND ${test} )
endforeach()

# TODO move test_mimetypes to catch.hpp
add_executable(test_mimetypes cfg80211/test_mimetypes.cc)
target_link_libraries(test_mimetypes duwt)
target_link_libraries(test_mimetypes stdc++fs)
add_test(NAME test_mimetypes COMMAND test_mimetypes)
