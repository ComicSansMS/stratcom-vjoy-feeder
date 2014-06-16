
project(stratcom-vjoy)

cmake_minimum_required(VERSION 3.0)

set(LIBSTRATCOM_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/external/libstratcom/include)
set(LIBSTRATCOM_LIBRARY_DIRECTORIES ${PROJECT_SOURCE_DIR}/external/libstratcom/lib)
set(LIBSTRATCOM_BINARY_DIRECTORIES ${PROJECT_SOURCE_DIR}/external/libstratcom/bin)
set(LIBSTRATCOM_LIBRARIES ${LIBSTRATCOM_LIBRARY_DIRECTORIES}/stratcom.lib)
set(LIBSTRATCOM_DLLS ${LIBSTRATCOM_BINARY_DIRECTORIES}/stratcom.dll)

set(VJOY_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/external/SDK/inc)
set(VJOY_LIBRARY_DIRECTORIES ${PROJECT_SOURCE_DIR}/external/SDK/lib/amd64)
set(VJOY_LIBRARIES ${VJOY_LIBRARY_DIRECTORIES}/vJoyInterface.lib)
set(VJOY_DLLS ${VJOY_LIBRARY_DIRECTORIES}/vJoyInterface.dll)


add_executable(stratcom-vjoy ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_include_directories(stratcom-vjoy PUBLIC ${LIBSTRATCOM_INCLUDE_DIRECTORIES} ${VJOY_INCLUDE_DIRECTORIES})
target_link_libraries(stratcom-vjoy PUBLIC ${LIBSTRATCOM_LIBRARIES} ${VJOY_LIBRARIES})
target_compile_options(stratcom-vjoy PUBLIC /W4)

file(COPY ${LIBSTRATCOM_DLLS} ${VJOY_DLLS} DESTINATION ${PROJECT_BINARY_DIR})
