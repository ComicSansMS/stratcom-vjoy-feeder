
project(stratcom-vjoy)

cmake_minimum_required(VERSION 3.0)

find_package(Boost REQUIRED)

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
find_package(Qt5WinExtras)

set(STRATCOM_VJOY_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/Config.cpp
    ${PROJECT_SOURCE_DIR}/src/Install.cpp
    ${PROJECT_SOURCE_DIR}/src/Log.cpp
)

set(STRATCOM_VJOY_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/src/Barrier.hpp
    ${PROJECT_SOURCE_DIR}/src/Config.hpp
    ${PROJECT_SOURCE_DIR}/src/HidRepDesc.hpp
    ${PROJECT_SOURCE_DIR}/src/Install.hpp
    ${PROJECT_SOURCE_DIR}/src/Log.hpp
)

set(STRATCOM_VJOY_QT_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/EventProcessor.cpp
    ${PROJECT_SOURCE_DIR}/src/IconProvider.cpp
    ${PROJECT_SOURCE_DIR}/src/TrayIcon.cpp
)

set(STRATCOM_VJOY_QT_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/src/IconProvider.hpp
)

set(STRATCOM_VJOY_QT_MOC_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/src/EventProcessor.hpp
    ${PROJECT_SOURCE_DIR}/src/TrayIcon.hpp
)
qt5_wrap_cpp(STRATCOM_VJOY_QT_MOC_SOURCE_FILES ${STRATCOM_VJOY_QT_MOC_HEADER_FILES})

set(STRATCOM_VJOY_RESOURCE_DIRECTORY ${PROJECT_SOURCE_DIR}/gfx)
set(STRATCOM_VJOY_RESOURCE_FILES
    ${STRATCOM_VJOY_RESOURCE_DIRECTORY}/Resource.rc
)
set(STRATCOM_VJOY_RESOURCE_HEADER_FILES
    ${STRATCOM_VJOY_RESOURCE_DIRECTORY}/resource.h
)

source_group(includes FILES
    ${STRATCOM_VJOY_HEADER_FILES}
    ${STRATCOM_VJOY_QT_HEADER_FILES}
    ${STRATCOM_VJOY_QT_MOC_HEADER_FILES}
)
source_group(moc FILES
    ${STRATCOM_VJOY_QT_MOC_SOURCE_FILES}
)

source_group(resource FILES
    ${STRATCOM_VJOY_RESOURCE_FILES}
    ${STRATCOM_VJOY_RESOURCE_HEADER_FILES}
)

add_executable(stratcom-vjoy WIN32
    ${STRATCOM_VJOY_SOURCE_FILES}
    ${STRATCOM_VJOY_HEADER_FILES}
    ${STRATCOM_VJOY_QT_SOURCE_FILES}
    ${STRATCOM_VJOY_QT_HEADER_FILES}
    ${STRATCOM_VJOY_QT_MOC_HEADER_FILES}
    ${STRATCOM_VJOY_QT_MOC_SOURCE_FILES}
    ${STRATCOM_VJOY_RESOURCE_FILES}
    ${STRATCOM_VJOY_RESOURCE_HEADER_FILES}
)
target_include_directories(stratcom-vjoy PUBLIC ${Boost_INCLUDE_DIRS} ${VJOY_INCLUDE_DIRECTORIES} ${STRATCOM_VJOY_RESOURCE_DIRECTORY})
target_link_libraries(stratcom-vjoy PUBLIC stratcom ${VJOY_LIBRARIES} Qt5::Widgets Qt5::WinExtras)
target_compile_options(stratcom-vjoy PUBLIC "/W4")

if(WIN32)
    get_property(dll TARGET stratcom PROPERTY IMPORTED_LOCATION_DEBUG)
    file(COPY ${dll} DESTINATION ${CMAKE_BINARY_DIR})
    get_property(dll TARGET stratcom PROPERTY IMPORTED_LOCATION_RELEASE)
    file(COPY ${dll} DESTINATION ${CMAKE_BINARY_DIR})

    file(COPY ${VJOY_DLLS} DESTINATION ${PROJECT_BINARY_DIR})

    getQt5Dlls(Qt5::Widgets qt_DLLS)
    getQt5Dlls(Qt5::WinExtras qt_DLLS)
    file(COPY ${qt_DLLS} ${QT5_ADDITIONAL_DLLS} DESTINATION ${PROJECT_BINARY_DIR})
endif()

install(TARGETS stratcom-vjoy DESTINATION .)
install(FILES $<TARGET_PDB_FILE:stratcom-vjoy> DESTINATION . OPTIONAL)
if(WIN32)
    install(FILES ${dll} ${VJOY_DLLS} DESTINATION .)
    find_program(QT5_WINDEPLOYQT windeployqt HINTS ${QT5_BASE_DIR} PATH_SUFFIXES bin)
    mark_as_advanced(QT5_WINDEPLOYQT)
    install(SCRIPT CODE "execute_process(COMMAND ${QT5_WINDEPLOYQT} ${CMAKE_INSTALL_PREFIX})")
endif()
