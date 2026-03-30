#pragma once
#include <adwaita.h>
#include "ProfileState.h"
#include <array>
#include <functional>

class FansPage {
public:
    FansPage();
    GtkWidget* widget() { return m_page; }

    void setCurrentProfile(uint8_t profile);
    void setFanCurves(const std::array<FanCurve, PROFILE_COUNT>& curves);
    std::function<void(uint8_t, const FanCurve&)> onCurveChanged;

private:
    GtkWidget* build();
    void refreshUi();
    void pushCurrentCurve();

    static void onCurveValueChanged(GtkRange*, gpointer self);

    GtkWidget* m_page = nullptr;
    uint8_t m_currentProfile = 1;
    std::array<FanCurve, PROFILE_COUNT> m_curves = defaultFanCurves();
    bool m_syncingUi = false;

    std::array<GtkScale*, FAN_CURVE_POINTS> m_cpuScales{};
    std::array<GtkScale*, FAN_CURVE_POINTS> m_gpuScales{};
};
