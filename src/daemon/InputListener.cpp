#include "InputListener.h"
#include <libevdev/libevdev.h>
#include <linux/input-event-codes.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <cstdio>

InputListener::InputListener() = default;

InputListener::~InputListener() {
    stop();
}

std::string InputListener::findKeyboardDevice() {
    DIR* dir = opendir("/dev/input");
    if (!dir) return {};

    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (strncmp(ent->d_name, "event", 5) != 0) continue;

        std::string path = std::string("/dev/input/") + ent->d_name;
        int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;

        libevdev* dev = nullptr;
        if (libevdev_new_from_fd(fd, &dev) < 0) {
            close(fd);
            continue;
        }

        bool isAsusKeyboard =
            libevdev_get_id_vendor(dev) == 0x0b05 &&
            libevdev_get_id_product(dev) == 0x19b6 &&
            libevdev_has_event_code(dev, EV_KEY, KEY_PROG3);

        libevdev_free(dev);
        close(fd);

        if (isAsusKeyboard) {
            closedir(dir);
            return path;
        }
    }
    closedir(dir);
    return {};
}

bool InputListener::start() {
    std::string path = findKeyboardDevice();
    if (path.empty()) {
        fprintf(stderr, "InputListener: could not find Asus keyboard\n");
        return false;
    }
    fprintf(stdout, "InputListener: using %s\n", path.c_str());
    m_running = true;
    m_thread = std::thread(&InputListener::run, this, path);
    return true;
}

void InputListener::stop() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
}

void InputListener::run(const std::string& devicePath) {
    int fd = open(devicePath.c_str(), O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "InputListener: cannot open %s\n", devicePath.c_str());
        return;
    }

    libevdev* dev = nullptr;
    if (libevdev_new_from_fd(fd, &dev) < 0) {
        fprintf(stderr, "InputListener: libevdev_new_from_fd failed\n");
        close(fd);
        return;
    }

    while (m_running) {
        input_event ev{};
        int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_BLOCKING, &ev);

        if (rc == LIBEVDEV_READ_STATUS_SYNC) {
            while (libevdev_next_event(dev, LIBEVDEV_READ_FLAG_SYNC, &ev) == LIBEVDEV_READ_STATUS_SYNC) {}
            continue;
        }
        if (rc < 0) break;
        if (ev.type != EV_KEY) continue;
        if (ev.value != 1) continue;  // key down only

        if      (ev.code == KEY_KBDILLUMDOWN && onBrightnessDown) onBrightnessDown();
        else if (ev.code == KEY_KBDILLUMUP   && onBrightnessUp)   onBrightnessUp();
        else if (ev.code == KEY_PROG3        && onAuraButton)      onAuraButton();
    }

    libevdev_free(dev);
    close(fd);
}