#pragma once
#include "AuraCtrl.h"
#include <string>
#include <mutex>
#include <vector>

class UsbCore {
public:
    UsbCore();
    ~UsbCore();

    bool connect();
    void disconnect();
    bool isConnected() const;
    bool sendPacket(const Packet& packet, bool finalize);
    bool sendPackets(const std::vector<Packet>& packets, bool finalize);
    const std::string& lastError() const { return m_lastError; }

private:
    bool controlTransfer(const Packet& packet);
    std::string findHidraw();

    int         m_fd        = -1;
    bool        m_connected = false;
    std::string m_lastError;
    std::mutex  m_mutex;

    static constexpr uint16_t ASUS_VENDOR_ID  = 0x0b05;
    static constexpr uint16_t ASUS_PRODUCT_ID = 0x19b6;
};