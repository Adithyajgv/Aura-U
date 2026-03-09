#pragma once
#include <gio/gio.h>
#include <functional>
#include <cstdint>

class DBusClient {
public:
    DBusClient();
    ~DBusClient();

    bool connect();
    bool isConnected() const;

    // Lighting commands → sent to daemon
    void setStaticColor(uint8_t r, uint8_t g, uint8_t b, uint8_t zone);
    void setBreathe(uint8_t r, uint8_t g, uint8_t b, uint8_t speed);
    void setColorCycle(uint8_t speed);
    void setRainbow(uint8_t speed);
    void setBrightness(uint8_t level);
    void cycleMode();

    // Signals from daemon → fired when hardware state changes
    std::function<void(uint8_t)> onModeChanged;
    std::function<void(uint8_t)> onBrightnessChanged;

private:
    static void onSignal(
        GDBusConnection* conn,
        const gchar* senderName,
        const gchar* objectPath,
        const gchar* interfaceName,
        const gchar* signalName,
        GVariant* params,
        gpointer self);

    GDBusConnection* m_conn       = nullptr;
    GDBusProxy*      m_proxy      = nullptr;
    guint            m_signalId   = 0;
    bool             m_connected  = false;
};