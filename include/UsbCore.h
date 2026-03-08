#pragma once
#include <libusb-1.0/libusb.h>
#include <vector>
#include <mutex>
#include <string>
#include "AuraCtrl.h"

static constexpr uint16_t ASUS_VENDOR_ID = 0x0b05;
static constexpr uint16_t ASUS_PRODUCT_IDS[] = {
    0x1854, 0x1869, 0x1866, 0x19b6, 0x1a30
};

class UsbCore {
public:
    UsbCore();
    ~UsbCore();

    UsbCore(const UsbCore&) = delete;
    UsbCore& operator=(const UsbCore&) = delete;

    bool connect();
    void disconnect();
    bool isConnected() const;

    bool sendPacket(const Packet& packet, bool finalize = false);
    bool sendPackets(const std::vector<Packet>& packets, bool finalize = true);

    std::string lastError() const { return m_lastError; }

private:
    bool controlTransfer(const Packet& packet);

    libusb_context*       m_ctx          = nullptr;
    libusb_device_handle* m_handle       = nullptr;
    int                   m_interfaceNum = 0;
    bool                  m_connected    = false;
    std::string           m_lastError;
    std::mutex            m_mutex;
};