#include "UsbCore.h"
#include "AuraCtrl.h"
#include "InputListener.h"
#include "DBusServer.h"
#include "json.hpp"
#include <glib.h>
#include <cstdio>
#include <fstream>
#include <array>
#include <algorithm>
#include <csignal>

static UsbCore       usb;
static DBusServer    dbus;
static InputListener input;

static AuraMode  currentMode       = AuraMode::Static;
static uint8_t   currentBrightness = 2;
static uint8_t   currentSpeed      = 0xeb;
static std::array<Color, 4> zoneColors = {{
    {0xff, 0x00, 0x00},
    {0x00, 0xff, 0x00},
    {0x00, 0x00, 0xff},
    {0xff, 0xff, 0x00}
}};

static const char* STATE_PATH = "/var/lib/aura-u/state.json";

static GMainLoop* g_loop = nullptr;

static void onSignal(int) {
    if (g_loop) g_main_loop_quit(g_loop);
}

static void saveState() {
    nlohmann::json j;
    j["mode"]       = static_cast<int>(currentMode);
    j["brightness"] = currentBrightness;
    j["speed"]      = currentSpeed;
    for (int i = 0; i < 4; ++i) {
        j["zones"][i] = {
            {"r", zoneColors[i].r},
            {"g", zoneColors[i].g},
            {"b", zoneColors[i].b}
        };
    }
    std::ofstream f(STATE_PATH);
    if (f) f << j.dump(2);
    else fprintf(stderr, "Failed to save state to %s\n", STATE_PATH);
}

static void loadState() {
    std::ifstream f(STATE_PATH);
    if (!f) {
        fprintf(stdout, "No saved state found, using defaults\n");
        return;
    }
    try {
        nlohmann::json j = nlohmann::json::parse(f);
        currentMode       = static_cast<AuraMode>(j.value("mode", 0));
        currentBrightness = j.value("brightness", 2);
        currentSpeed      = j.value("speed", 0xeb);
        for (int i = 0; i < 4; ++i) {
            zoneColors[i].r = j["zones"][i].value("r", 255);
            zoneColors[i].g = j["zones"][i].value("g", 0);
            zoneColors[i].b = j["zones"][i].value("b", 0);
        }
        fprintf(stdout, "State loaded from %s\n", STATE_PATH);
    } catch (...) {
        fprintf(stderr, "Failed to parse state file, using defaults\n");
    }
}

static void applyCurrentMode() {
    switch (currentMode) {
        case AuraMode::Static:
            for (uint8_t i = 0; i < 4; ++i)
                usb.sendPacket(AuraCtrl::multiStatic(zoneColors, i), false);
            usb.sendPacket(AuraCtrl::setPacket(), false);
            usb.sendPacket(AuraCtrl::applyPacket(), false);
            break;
        case AuraMode::Breathe:
            usb.sendPacket(AuraCtrl::breathe(zoneColors[0], currentSpeed), true);
            break;
        case AuraMode::ColorCycle:
            usb.sendPacket(AuraCtrl::colorCycle(currentSpeed), true);
            break;
        case AuraMode::Rainbow:
            usb.sendPacket(AuraCtrl::rainbow(currentSpeed), true);
            break;
        default: break;
    }
}

static void cycleMode() {
    int next = (static_cast<int>(currentMode) + 1) % static_cast<int>(AuraMode::COUNT);
    currentMode = static_cast<AuraMode>(next);
    applyCurrentMode();
    dbus.emitModeChanged(static_cast<uint8_t>(currentMode));
    saveState();
    fprintf(stdout, "Mode changed to %d\n", static_cast<int>(currentMode));
}

static void setBrightness(uint8_t level) {
    currentBrightness = std::clamp<uint8_t>(level, 0, 3);
    usb.sendPacket(AuraCtrl::brightness(currentBrightness), false);
    dbus.emitBrightnessChanged(currentBrightness);
    saveState();
    fprintf(stdout, "Brightness set to %d\n", currentBrightness);
}

int main() {
    fprintf(stdout, "Aura-U daemon starting\n");

    // Connect to keyboard
    if (!usb.connect()) {
        fprintf(stderr, "Failed to connect to keyboard: %s\n", usb.lastError().c_str());
        return 1;
    }
    fprintf(stdout, "Keyboard connected\n");

    // Load previous state then apply it
    loadState();
    applyCurrentMode();
    usb.sendPacket(AuraCtrl::brightness(currentBrightness), false);

    // Wire input listener
    input.onBrightnessDown = []() {
        if (currentBrightness > 0) setBrightness(currentBrightness - 1);
    };
    input.onBrightnessUp = []() {
        if (currentBrightness < 3) setBrightness(currentBrightness + 1);
    };
    input.onAuraButton = []() {
        cycleMode();
    };
    input.start();

    // Wire D-Bus server
    dbus.onSetStaticColor = [](uint8_t r, uint8_t g, uint8_t b, uint8_t zone) {
        zoneColors[zone % 4] = {r, g, b};
        currentMode = AuraMode::Static;
        applyCurrentMode();
        saveState();
    };
    dbus.onSetBreathe = [](uint8_t r, uint8_t g, uint8_t b, uint8_t speed) {
        zoneColors[0] = {r, g, b};
        currentMode = AuraMode::Breathe;
        currentSpeed = speed;
        applyCurrentMode();
        saveState();
    };
    dbus.onSetColorCycle = [](uint8_t speed) {
        currentMode = AuraMode::ColorCycle;
        currentSpeed = speed;
        applyCurrentMode();
        saveState();
    };
    dbus.onSetRainbow = [](uint8_t speed) {
        currentMode = AuraMode::Rainbow;
        currentSpeed = speed;
        applyCurrentMode();
        saveState();
    };
    dbus.onSetBrightness = [](uint8_t level) {
        setBrightness(level);
    };
    dbus.onCycleMode = []() {
        cycleMode();
    };


    g_idle_add([](gpointer) -> gboolean {
        dbus.start();
        return G_SOURCE_REMOVE; 
    }, nullptr);

    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);
    // Run GLib main loop
    GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
    g_loop = loop;
    fprintf(stdout, "Aura-U daemon running\n");
    g_main_loop_run(loop);
    g_main_loop_unref(loop);

    input.stop();
    dbus.stop();
    usb.disconnect();
    return 0;
}