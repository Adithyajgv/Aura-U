#pragma once
#include <adwaita.h>
#include "DBusClient.h"
#include "AuraCtrl.h"

class AuraPage {
public:
    AuraPage(DBusClient& client);
    GtkWidget* widget() { return m_page; }

    void syncMode(uint8_t mode);
    void syncBrightness(uint8_t level);

private:
    GtkWidget* build();
    void updateControlVisibility();
    void applyCurrentSettings();

    static void onModeChanged(GtkDropDown*, GParamSpec*, gpointer self);
    static void onSpeedChanged(GtkRange*, gpointer self);
    static void onBrightnessChanged(GtkRange* range, gpointer self);
    static void onZoneColorSet(GtkColorDialogButton*, GParamSpec*, gpointer self);
    static void onBreatheColorSet(GtkColorDialogButton*, GParamSpec*, gpointer self);
    static void onLightbarModeChanged(GtkDropDown*, GParamSpec*, gpointer self);

    DBusClient&           m_client;
    GtkWidget*            m_page            = nullptr;
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