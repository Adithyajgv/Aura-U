#pragma once
#include <gio/gio.h>
#include <functional>
#include <cstdint>
#include <array>
#include "ProfileState.h"

class DBusServer {
public:
    DBusServer();
    ~DBusServer();

    std::function<void(uint8_t, uint8_t, uint8_t, uint8_t)> onSetStaticColor;
    std::function<void(uint8_t, uint8_t, uint8_t, uint8_t)> onSetBreathe;
    std::function<void(uint8_t)> onSetColorCycle;
    std::function<void(uint8_t)> onSetRainbow;
    std::function<void(uint8_t)> onSetBrightness;
    std::function<void()> onCycleMode;
    std::function<void(uint8_t)> onSetLightbarMode;
    std::function<void(uint8_t)> onSetCurrentProfile;
    std::function<void(uint8_t, const std::array<uint8_t, FAN_CURVE_POINTS>&,
                       const std::array<uint8_t, FAN_CURVE_POINTS>&)> onSetFanCurve;

    bool start();
    void stop();

    void emitModeChanged(uint8_t mode);
    void emitBrightnessChanged(uint8_t level);
    void emitCurrentProfileChanged(uint8_t profile);

    static void handleMethodCall(
        GDBusConnection* conn,
        const gchar* sender,
        const gchar* objectPath,
        const gchar* interfaceName,
        const gchar* methodName,
        GVariant* params,
        GDBusMethodInvocation* invocation,
        gpointer self);

private:
    static void onBusAcquired(GDBusConnection* conn, const gchar* name, gpointer self);
    static void onNameAcquired(GDBusConnection* conn, const gchar* name, gpointer self);
    static void onNameLost(GDBusConnection* conn, const gchar* name, gpointer self);

    GDBusConnection* m_conn      = nullptr;
    GDBusNodeInfo*   m_nodeInfo  = nullptr;
    guint            m_ownerId   = 0;
    guint            m_regId     = 0;

    static const char* INTROSPECTION_XML;
};