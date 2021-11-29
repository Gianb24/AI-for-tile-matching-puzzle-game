cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
set(QT_MIN_VERSION "5.0.0")

set(RELEASE_SERVICE_VERSION_MAJOR "0")
set(RELEASE_SERVICE_VERSION_MINOR "9")
set(RELEASE_SERVICE_VERSION_MICRO "5")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(bejeweled VERSION ${RELEASE_SERVICE_VERSION})

file(GLOB my_sources *.h *.cpp *.qrc *.ui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(Bejeweled ${my_sources})

find_package(Qt5Widgets ${QT_MIN_VERSION} CONFIG REQUIRED)
find_package(Qt5Multimedia REQUIRED)

target_link_libraries(Bejeweled Qt5::Widgets Qt5::Multimedia)

if (WIN32)
set_property(TARGET Bejeweled PROPERTY WIN32_EXECUTABLE true)
endif (WIN32)

# CPACK
install (TARGETS Bejeweled)

set (CPACK PACKAGE VERSION MAJOR 0 )
set (CPACK PACKAGE VERSION MINOR 9 )
set (CPACK PACKAGE VERSION PATCH 5 )

set(CPACK_PACKAGE_NAME "Bejeweled")
set(CPACK_PACKAGE_ICON  "${CMAKE_CURRENT_SOURCE_DIR}/res/images\\\\unicas1.bmp")
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/res/images/icon.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/res/images/icon.ico")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/res/license.txt")
include (CPack)