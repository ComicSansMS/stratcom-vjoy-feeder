
project(stratcom-vjoy)

cmake_minimum_required(VERSION 3.0)

set(LIBSTRATCOM_PREFIX_PATH "" CACHE PATH "Set this to the installation directory of the libstratcom binaries")

if(LIBSTRATCOM_PREFIX_PATH)
    list(APPEND CMAKE_PREFIX_PATH ${LIBSTRATCOM_PREFIX_PATH})
endif()
find_package(libstratcom NO_MODULE REQUIRED)

set(VJOY_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/external/SDK/inc)
set(VJOY_LIBRARY_DIRECTORIES ${PROJECT_SOURCE_DIR}/external/SDK/lib/amd64)
set(VJOY_LIBRARIES ${VJOY_LIBRARY_DIRECTORIES}/vJoyInterface.lib)
set(VJOY_DLLS ${VJOY_LIBRARY_DIRECTORIES}/vJoyInterface.dll)

add_executable(stratcom-vjoy ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_include_directories(stratcom-vjoy PUBLIC ${VJOY_INCLUDE_DIRECTORIES})
target_link_libraries(stratcom-vjoy PUBLIC stratcom ${VJOY_LIBRARIES})
target_compile_options(stratcom-vjoy PUBLIC "/W4")

if(WIN32)
    get_property(dll TARGET stratcom PROPERTY IMPORTED_LOCATION_DEBUG)
    file(COPY ${dll} DESTINATION ${CMAKE_BINARY_DIR})
    get_property(dll TARGET stratcom PROPERTY IMPORTED_LOCATION_RELEASE)
    file(COPY ${dll} DESTINATION ${CMAKE_BINARY_DIR})
    
    file(COPY ${VJOY_DLLS} DESTINATION ${PROJECT_BINARY_DIR})
endif()
