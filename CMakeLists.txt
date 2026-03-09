cmake_minimum_required(VERSION 3.20)
project(Aura-U VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(ADW REQUIRED libadwaita-1)
pkg_check_modules(GLIB REQUIRED gio-2.0)
pkg_check_modules(UDEV REQUIRED libudev)

# Daemon Executable
set(DAEMON_SOURCES
    src/daemon/main.cpp
    src/daemon/AuraCtrl.cpp
    src/daemon/UsbCore.cpp
    src/daemon/InputListener.cpp
    src/daemon/DBusServer.cpp
)
add_executable(daura-u ${DAEMON_SOURCES})

target_include_directories(daura-u PRIVATE
    include
    ${LIBEVDEV_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${UDEV_INCLUDE_DIRS}
)
target_link_libraries(daura-u PRIVATE
    ${LIBEVDEV_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${UDEV_LIBRARIES}
)
target_compile_options(daura-u PRIVATE
    ${GLIB_CFLAGS_OTHER}
    ${UDEV_CFLAGS_OTHER}
)

# App Executable
set(APP_SOURCES
    src/app/main.cpp
    src/app/DBusClient.cpp
    src/app/MainWindow.cpp
)
add_executable(aura-u ${APP_SOURCES})
target_compile_definitions(aura-u PRIVATE
    INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
)
target_include_directories(aura-u PRIVATE
    include
    ${GTK4_INCLUDE_DIRS}
    ${ADW_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
)
target_link_libraries(aura-u PRIVATE
    ${GTK4_LIBRARIES}
    ${ADW_LIBRARIES}
    ${GLIB_LIBRARIES}
)
target_compile_options(aura-u PRIVATE
    ${GTK4_CFLAGS_OTHER}
    ${ADW_CFLAGS_OTHER}
    ${GLIB_CFLAGS_OTHER}
)

# Generate systemd service file
configure_file(
    ${CMAKE_SOURCE_DIR}/systemd/daura-u.service.in
    ${CMAKE_BINARY_DIR}/daura-u.service
)

# Installation
install(TARGETS daura-u DESTINATION bin)
install(TARGETS aura-u DESTINATION bin)
install(FILES dbus/aura-u.xml DESTINATION share/dbus-1/interfaces/)
install(FILES dbus/io.AuraU.conf DESTINATION /etc/dbus-1/system.d/)
install(DIRECTORY DESTINATION /var/lib/aura-u)
install(FILES assets/rog_logo.png DESTINATION share/icons/hicolor/256x256/apps/)
install(FILES aura-u.desktop DESTINATION share/applications/)
install(FILES ${CMAKE_BINARY_DIR}/daura-u.service DESTINATION /etc/systemd/system/)
install(CODE "execute_process(COMMAND gtk-update-icon-cache ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/)")
install(CODE "execute_process(COMMAND systemctl daemon-reload)")
install(CODE "execute_process(COMMAND systemctl enable daura-u)")
install(CODE "execute_process(COMMAND systemctl start daura-u)")