#include "UsbCore.h"
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <libudev.h>
#include <cstring>

UsbCore::UsbCore() = default;

UsbCore::~UsbCore() {
    disconnect();
}

std::string UsbCore::findHidraw() {
    udev* ud = udev_new();
    if (!ud) return {};

    udev_enumerate* en = udev_enumerate_new(ud);
    udev_enumerate_add_match_subsystem(en, "hidraw");
    udev_enumerate_scan_devices(en);

    udev_list_entry* devices = udev_enumerate_get_list_entry(en);
    udev_list_entry* entry;
    std::string result;

    udev_list_entry_foreach(entry, devices) {
        const char* path = udev_list_entry_get_name(entry);
        udev_device* dev = udev_device_new_from_syspath(ud, path);

        // Walk up to USB parent to get vendor/product
        udev_device* usbDev = udev_device_get_parent_with_subsystem_devtype(
            dev, "usb", "usb_device");

        if (usbDev) {
            const char* vid = udev_device_get_sysattr_value(usbDev, "idVendor");
            const char* pid = udev_device_get_sysattr_value(usbDev, "idProduct");

            if (vid && pid &&
                strtol(vid, nullptr, 16) == ASUS_VENDOR_ID &&
                strtol(pid, nullptr, 16) == ASUS_PRODUCT_ID) {
                const char* node = udev_device_get_devnode(dev);
                if (node) result = node;
                udev_device_unref(dev);
                break;
            }
        }
        udev_device_unref(dev);
    }

    udev_enumerate_unref(en);
    udev_unref(ud);
    return result;
}

bool UsbCore::connect() {
    std::lock_guard lock(m_mutex);

    std::string node = findHidraw();
    if (node.empty()) {
        m_lastError = "No ROG keyboard hidraw device found";
        return false;
    }
    fprintf(stdout, "Using hidraw device: %s\n", node.c_str());

    m_fd = open(node.c_str(), O_RDWR | O_NONBLOCK);
    if (m_fd < 0) {
        m_lastError = std::string("Cannot open ") + node + ": " + strerror(errno);
        return false;
    }

    m_connected = true;
    return true;
}

void UsbCore::disconnect() {
    std::lock_guard lock(m_mutex);
    if (m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }
    m_connected = false;
}

bool UsbCore::isConnected() const {
    return m_connected;
}

bool UsbCore::controlTransfer(const Packet& packet) {
    // hidraw write requires a leading report ID byte (0x00 for report ID 0)
    uint8_t buf[MSG_LEN + 1] = {};
    buf[0] = 0x00;  // report ID
    memcpy(buf + 1, packet.data(), MSG_LEN);

    ssize_t r = write(m_fd, buf, sizeof(buf));
    fprintf(stdout, "hidraw write: packet[0]=%02x result=%zd\n", packet[0], r);
    if (r < 0) {
        m_lastError = std::string("hidraw write failed: ") + strerror(errno);
        return false;
    }
    return true;
}

bool UsbCore::sendPacket(const Packet& packet, bool finalize) {
    std::lock_guard lock(m_mutex);
    if (!m_connected) return false;
    if (!controlTransfer(packet)) return false;
    if (finalize) {
        controlTransfer(AuraCtrl::setPacket());
        controlTransfer(AuraCtrl::applyPacket());
    }
    return true;
}

bool UsbCore::sendPackets(const std::vector<Packet>& packets, bool finalize) {
    std::lock_guard lock(m_mutex);
    if (!m_connected) return false;
    for (const auto& p : packets) {
        if (!controlTransfer(p)) return false;
    }
    if (finalize) {
        controlTransfer(AuraCtrl::setPacket());
        controlTransfer(AuraCtrl::applyPacket());
    }
    return true;
}