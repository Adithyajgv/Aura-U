#include "DBusServer.h"
#include <cstdio>
#include <cstring>



static const GDBusInterfaceVTable INTERFACE_VTABLE = {
    DBusServer::handleMethodCall,
    nullptr,
    nullptr
};

DBusServer::DBusServer() = default;

DBusServer::~DBusServer() {
    stop();
}

bool DBusServer::start() {
    GError* err = nullptr;
    
    gchar* xml = nullptr;
    if (!g_file_get_contents("/usr/local/share/dbus-1/interfaces/aura-u.xml", &xml, nullptr, &err)) {
        fprintf(stderr, "DBusServer: failed to read XML: %s\n", err->message);
        g_error_free(err);
        return false;
    }
    
    m_nodeInfo = g_dbus_node_info_new_for_xml(xml, &err);
    g_free(xml);
    if (!m_nodeInfo) {
        fprintf(stderr, "DBusServer: failed to parse XML: %s\n", err->message);
        g_error_free(err);
        return false;
    }

    // Connect synchronously
    m_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &err);
    if (!m_conn) {
        fprintf(stderr, "DBusServer: failed to connect to system bus: %s\n", err->message);
        g_error_free(err);
        return false;
    }
    fprintf(stdout, "DBusServer: connected to system bus\n");

    // Register object
    m_regId = g_dbus_connection_register_object(
        m_conn,
        "/io/AuraU",
        m_nodeInfo->interfaces[0],
        &INTERFACE_VTABLE,
        this,
        nullptr,
        &err
    );
    if (!m_regId) {
        fprintf(stderr, "DBusServer: register object failed: %s\n", err->message);
        g_error_free(err);
        return false;
    }
    fprintf(stdout, "DBusServer: object registered\n");

    // Own name
    m_ownerId = g_bus_own_name_on_connection(
        m_conn,
        "io.AuraU",
        G_BUS_NAME_OWNER_FLAGS_NONE,
        onNameAcquired,
        onNameLost,
        this,
        nullptr
    );
    fprintf(stdout, "DBusServer: name requested\n");
    return true;
}

void DBusServer::stop() {
    if (m_ownerId) {
        g_bus_unown_name(m_ownerId);
        m_ownerId = 0;
    }
    if (m_nodeInfo) {
        g_dbus_node_info_unref(m_nodeInfo);
        m_nodeInfo = nullptr;
    }
}

void DBusServer::onNameAcquired(GDBusConnection*, const gchar* name, gpointer) {
    fprintf(stdout, "DBusServer: acquired name %s\n", name);
}

void DBusServer::onNameLost(GDBusConnection*, const gchar* name, gpointer) {
    fprintf(stderr, "DBusServer: lost name %s\n", name);
}

void DBusServer::handleMethodCall(
    GDBusConnection*,
    const gchar*,
    const gchar*,
    const gchar*,
    const gchar* methodName,
    GVariant* params,
    GDBusMethodInvocation* invocation,
    gpointer self)
{
    fprintf(stdout, "DBusServer: received method call: %s\n", methodName);
    auto* s = static_cast<DBusServer*>(self);
    uint8_t a, b, c, d;

    if (strcmp(methodName, "SetStaticColor") == 0) {
        g_variant_get(params, "(yyyy)", &a, &b, &c, &d);
        if (s->onSetStaticColor) s->onSetStaticColor(a, b, c, d);

    } else if (strcmp(methodName, "SetBreathe") == 0) {
        g_variant_get(params, "(yyyy)", &a, &b, &c, &d);
        if (s->onSetBreathe) s->onSetBreathe(a, b, c, d);

    } else if (strcmp(methodName, "SetColorCycle") == 0) {
        g_variant_get(params, "(y)", &a);
        if (s->onSetColorCycle) s->onSetColorCycle(a);

    } else if (strcmp(methodName, "SetRainbow") == 0) {
        g_variant_get(params, "(y)", &a);
        if (s->onSetRainbow) s->onSetRainbow(a);

    } else if (strcmp(methodName, "SetBrightness") == 0) {
        g_variant_get(params, "(y)", &a);
        if (s->onSetBrightness) s->onSetBrightness(a);

    } else if (strcmp(methodName, "CycleMode") == 0) {
        if (s->onCycleMode) s->onCycleMode();
    } else if (strcmp(methodName, "SetLightbarMode") == 0) {
        g_variant_get(params, "(y)", &a);
        if (s->onSetLightbarMode) s->onSetLightbarMode(a);
    }

    g_dbus_method_invocation_return_value(invocation, nullptr);
}

void DBusServer::emitModeChanged(uint8_t mode) {
    if (!m_conn) return;
    g_dbus_connection_emit_signal(
        m_conn,
        nullptr,
        "/io/AuraU",
        "io.AuraU",
        "ModeChanged",
        g_variant_new("(y)", mode),
        nullptr
    );
}

void DBusServer::emitBrightnessChanged(uint8_t level) {
    if (!m_conn) return;
    g_dbus_connection_emit_signal(
        m_conn,
        nullptr,
        "/io/AuraU",
        "io.AuraU",
        "BrightnessChanged",
        g_variant_new("(y)", level),
        nullptr
    );
}