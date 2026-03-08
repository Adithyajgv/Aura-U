cmake_minimum_required(VERSION 3.20)
project(Aura-U VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB   REQUIRED libusb-1.0)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)
pkg_check_modules(GTK4     REQUIRED gtk4)
pkg_check_modules(ADW      REQUIRED libadwaita-1)

set(SOURCES
    src/main.cpp
    src/AuraCtrl.cpp
    src/UsbCore.cpp
    src/AuraController.cpp
    src/InputListener.cpp
    src/MainWindow.cpp
)

add_executable(Aura-U ${SOURCES})

target_include_directories(Aura-U PRIVATE
    include
    ${LIBUSB_INCLUDE_DIRS}
    ${LIBEVDEV_INCLUDE_DIRS}
    ${GTK4_INCLUDE_DIRS}
    ${ADW_INCLUDE_DIRS}
)

target_link_libraries(Aura-U PRIVATE
    ${LIBUSB_LIBRARIES}
    ${LIBEVDEV_LIBRARIES}
    ${GTK4_LIBRARIES}
    ${ADW_LIBRARIES}
)

target_compile_options(Aura-U PRIVATE
    ${LIBUSB_CFLAGS_OTHER}
    ${GTK4_CFLAGS_OTHER}
    ${ADW_CFLAGS_OTHER}
)

install(TARGETS Aura-U DESTINATION bin)