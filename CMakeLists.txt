
project(stratcom-vjoy)

cmake_minimum_required(VERSION 3.0)

set(LIBSTRATCOM_PREFIX_PATH "" CACHE PATH "Set this to the installation directory of the libstratcom binaries")

if(LIBSTRATCOM_PREFIX_PATH)
    list(APPEND CMAKE_PREFIX_PATH ${LIBSTRATCOM_PREFIX_PATH})
endif()
find_package(libstratcom NO_MODULE REQUIRED)

set(VJOY_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/external/vjoy/SDK/inc)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(VJOY_LIBRARY_DIRECTORIES ${PROJECT_SOURCE_DIR}/external/vjoy/SDK/lib/amd64)
else()
    set(VJOY_LIBRARY_DIRECTORIES ${PROJECT_SOURCE_DIR}/external/vjoy/SDK/lib/)
endif()
set(VJOY_LIBRARIES ${VJOY_LIBRARY_DIRECTORIES}/vJoyInterface.lib)
set(VJOY_DLLS ${VJOY_LIBRARY_DIRECTORIES}/vJoyInterface.dll)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
if(WIN32)
    find_package(Qt5BaseDir)
    list(APPEND CMAKE_PREFIX_PATH ${QT5_BASE_DIR} ${WINSDK_LIB_DIR})
endif()
find_package(Qt5Widgets)

set(STRATCOM_VJOY_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/Log.cpp
)

set(STRATCOM_VJOY_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/src/Log.hpp
)

set(STRATCOM_VJOY_QT_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/EventProcessor.cpp
    ${PROJECT_SOURCE_DIR}/src/TrayIcon.cpp
)

set(STRATCOM_VJOY_QT_HEADER_FILES
)

set(STRATCOM_VJOY_QT_MOC_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/src/EventProcessor.hpp
    ${PROJECT_SOURCE_DIR}/src/TrayIcon.hpp
)
qt5_wrap_cpp(STRATCOM_VJOY_QT_MOC_SOURCE_FILES ${STRATCOM_VJOY_QT_MOC_HEADER_FILES})

source_group(includes FILES
    ${STRATCOM_VJOY_HEADER_FILES}
    ${STRATCOM_VJOY_QT_HEADER_FILES}
    ${STRATCOM_VJOY_QT_MOC_HEADER_FILES}
)
source_group(moc FILES
    ${STRATCOM_VJOY_QT_MOC_SOURCE_FILES}
)


add_executable(stratcom-vjoy WIN32
    ${STRATCOM_VJOY_SOURCE_FILES}
    ${STRATCOM_VJOY_HEADER_FILES}
    ${STRATCOM_VJOY_QT_SOURCE_FILES}
    ${STRATCOM_VJOY_QT_HEADER_FILES}
    ${STRATCOM_VJOY_QT_MOC_HEADER_FILES}
    ${STRATCOM_VJOY_QT_MOC_SOURCE_FILES}
)
target_include_directories(stratcom-vjoy PUBLIC ${VJOY_INCLUDE_DIRECTORIES})
target_link_libraries(stratcom-vjoy PUBLIC stratcom ${VJOY_LIBRARIES} Qt5::Widgets)
target_compile_options(stratcom-vjoy PUBLIC "/W4")

if(WIN32)
    get_property(dll TARGET stratcom PROPERTY IMPORTED_LOCATION_DEBUG)
    file(COPY ${dll} DESTINATION ${CMAKE_BINARY_DIR})
    get_property(dll TARGET stratcom PROPERTY IMPORTED_LOCATION_RELEASE)
    file(COPY ${dll} DESTINATION ${CMAKE_BINARY_DIR})
    
    file(COPY ${VJOY_DLLS} DESTINATION ${PROJECT_BINARY_DIR})
    
    getQt5Dlls(Qt5::Widgets qt_DLLS)
    file(COPY ${qt_DLLS} ${QT5_ADDITIONAL_DLLS} DESTINATION ${PROJECT_BINARY_DIR})
endif()
