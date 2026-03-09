#include "DBusClient.h"
#include "json.hpp"
#include <fstream>
#include <cstdio>
#include <cstring>

DBusClient::DBusClient() = default;

DBusClient::~DBusClient() {
    if (m_signalId && m_conn)
        g_dbus_connection_signal_unsubscribe(m_conn, m_signalId);
    if (m_proxy)
        g_object_unref(m_proxy);
}

bool DBusClient::connect() {
    GError* err = nullptr;

    m_proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SYSTEM,
        G_DBUS_PROXY_FLAGS_NONE,
        nullptr,
        "io.AuraU",
        "/io/AuraU",
        "io.AuraU",
        nullptr,
        &err
    );

    if (!m_proxy) {
        fprintf(stderr, "DBusClient: failed to connect: %s\n", err->message);
        g_error_free(err);
        return false;
    }

    // Subscribe to signals from daemon
    m_conn = g_dbus_proxy_get_connection(m_proxy);
    m_signalId = g_dbus_connection_signal_subscribe(
        m_conn,
        nullptr,
        "io.AuraU",
        nullptr,
        "/io/AuraU",
        nullptr,
        G_DBUS_SIGNAL_FLAGS_NONE,
        onSignal,
        this,
        nullptr
    );

    m_connected = true;
    fprintf(stdout, "DBusClient: connected to daemon\n");
    return true;
}

bool DBusClient::isConnected() const {
    return m_connected;
}

void DBusClient::setStaticColor(uint8_t r, uint8_t g, uint8_t b, uint8_t zone) {
    fprintf(stdout, "DBusClient: setStaticColor zone=%d r=%d g=%d b=%d\n", zone, r, g, b);
    g_dbus_proxy_call(m_proxy, "SetStaticColor",
        g_variant_new("(yyyy)", r, g, b, zone),
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
}

void DBusClient::setBreathe(uint8_t r, uint8_t g, uint8_t b, uint8_t speed) {
    g_dbus_proxy_call(m_proxy, "SetBreathe",
        g_variant_new("(yyyy)", r, g, b, speed),
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
}

void DBusClient::setColorCycle(uint8_t speed) {
    g_dbus_proxy_call(m_proxy, "SetColorCycle",
        g_variant_new("(y)", speed),
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
}

void DBusClient::setRainbow(uint8_t speed) {
    g_dbus_proxy_call(m_proxy, "SetRainbow",
        g_variant_new("(y)", speed),
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
}

void DBusClient::setBrightness(uint8_t level) {
    g_dbus_proxy_call(m_proxy, "SetBrightness",
        g_variant_new("(y)", level),
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
}

void DBusClient::setLightbarMode(uint8_t mode) {
    g_dbus_proxy_call(m_proxy, "SetLightbarMode",
        g_variant_new("(y)", mode),
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
}

void DBusClient::cycleMode() {
    g_dbus_proxy_call(m_proxy, "CycleMode",
        nullptr,
        G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
}

bool DBusClient::getState(uint8_t& mode, uint8_t& brightness, uint8_t& speed, std::array<Color, 4>& zones, uint8_t& lightbarMode) {
    std::ifstream f("/var/lib/aura-u/state.json");
    if (!f) return false;
    try {
        nlohmann::json j = nlohmann::json::parse(f);
        mode         = j.value("mode", 0);
        brightness   = j.value("brightness", 2);
        speed        = j.value("speed", 0xeb);
        lightbarMode = j.value("lightbar_mode", 1);
        for (int i = 0; i < 4; ++i) {
            zones[i].r = j["zones"][i].value("r", 255);
            zones[i].g = j["zones"][i].value("g", 0);
            zones[i].b = j["zones"][i].value("b", 0);
        }
        return true;
    } catch (...) {
        return false;
    }
}


void DBusClient::onSignal(
    GDBusConnection*,
    const gchar*,
    const gchar*,
    const gchar*,
    const gchar* signalName,
    GVariant* params,
    gpointer self)
{
    auto* c = static_cast<DBusClient*>(self);
    uint8_t val;

    if (strcmp(signalName, "ModeChanged") == 0) {
        g_variant_get(params, "(y)", &val);
        if (c->onModeChanged) c->onModeChanged(val);

    } else if (strcmp(signalName, "BrightnessChanged") == 0) {
        g_variant_get(params, "(y)", &val);
        if (c->onBrightnessChanged) c->onBrightnessChanged(val);
    }
}