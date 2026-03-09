#pragma once
#include <adwaita.h>
#include "DBusClient.h"

class MainWindow {
public:
    MainWindow(AdwApplication* app, DBusClient& client);

private:
    void buildWindow(AdwApplication* app);
    GtkWidget* buildAuraPage();

    void updateControlVisibility();
    void applyCurrentSettings();

    static void onModeChanged(GtkDropDown* dd, GParamSpec*, gpointer self);
    static void onSpeedChanged(GtkRange* range, gpointer self);
    static void onBrightnessChanged(GtkRange* range, gpointer self);
    static void onLightbarModeChanged(GtkDropDown* dd, GParamSpec*, gpointer self);
    static void onZoneColorSet(GtkColorDialogButton* btn, GParamSpec*, gpointer self);
    static void onBreatheColorSet(GtkColorDialogButton* btn, GParamSpec*, gpointer self);

    void syncMode(uint8_t mode);
    void syncBrightness(uint8_t level);

    DBusClient& m_client;

    AdwApplicationWindow* m_window          = nullptr;
    GtkDropDown*          m_modeDD          = nullptr;
    GtkScale*             m_speedScale      = nullptr;
    GtkScale*             m_brightnessScale = nullptr;
    GtkColorDialogButton* m_zoneButtons[4]  = {};
    GtkColorDialogButton* m_breatheColor    = nullptr;
    GtkWidget*            m_speedRow        = nullptr;
    GtkWidget*            m_zoneColorGroup  = nullptr;
    GtkWidget*            m_breatheColorRow = nullptr;
    GtkDropDown*          m_lightbarDD      = nullptr;
};