#include "UsbCore.h"

UsbCore::UsbCore() {
    libusb_init(&m_ctx);
}

UsbCore::~UsbCore() {
    disconnect();
    if (m_ctx) libusb_exit(m_ctx);
}

bool UsbCore::connect() {
    std::lock_guard lock(m_mutex);

    libusb_device** list = nullptr;
    int n = libusb_get_device_list(m_ctx, &list);
    if (n < 0) {
        m_lastError = "Failed to get USB device list";
        return false;
    }

    libusb_device* found = nullptr;
    for (int i = 0; i < n; ++i) {
        libusb_device_descriptor desc{};
        libusb_get_device_descriptor(list[i], &desc);
        if (desc.idVendor != ASUS_VENDOR_ID) continue;
        for (auto pid : ASUS_PRODUCT_IDS) {
            if (desc.idProduct == pid) {
                found = list[i];
                break;
            }
        }
        if (found) break;
    }

    if (!found) {
        libusb_free_device_list(list, 1);
        m_lastError = "No ROG keyboard found";
        return false;
    }

    int r = libusb_open(found, &m_handle);
    libusb_free_device_list(list, 1);
    if (r < 0) {
        m_lastError = std::string("Cannot open device: ") + libusb_error_name(r);
        return false;
    }

    libusb_set_auto_detach_kernel_driver(m_handle, 1);

    libusb_config_descriptor* cfg = nullptr;
    libusb_get_active_config_descriptor(found, &cfg);
    if (!cfg || cfg->bNumInterfaces == 0) {
        m_lastError = "No interfaces on device";
        libusb_close(m_handle);
        m_handle = nullptr;
        return false;
    }
    m_interfaceNum = cfg->interface[0].altsetting[0].bInterfaceNumber;
    libusb_free_config_descriptor(cfg);

    r = libusb_claim_interface(m_handle, m_interfaceNum);
    if (r < 0) {
        m_lastError = std::string("Cannot claim interface: ") + libusb_error_name(r);
        libusb_close(m_handle);
        m_handle = nullptr;
        return false;
    }

    m_connected = true;
    sendPacket(AuraCtrl::initKeyboard(), false);
    return true;
}

void UsbCore::disconnect() {
    std::lock_guard lock(m_mutex);
    if (m_handle) {
        libusb_release_interface(m_handle, m_interfaceNum);
        libusb_close(m_handle);
        m_handle = nullptr;
    }
    m_connected = false;
}

bool UsbCore::isConnected() const {
    return m_connected;
}

bool UsbCore::controlTransfer(const Packet& packet) {
    int r = libusb_control_transfer(
        m_handle,
        0x21,    // host→device, class, interface
        9,       // HID SET_REPORT
        0x035d,  // report type + report ID
        0,
        const_cast<uint8_t*>(packet.data()),
        MSG_LEN,
        0
    );
    if (r < 0) {
        m_lastError = std::string("Transfer failed: ") + libusb_error_name(r);
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