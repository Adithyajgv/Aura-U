#include "FansPage.h"

FansPage::FansPage() {
    m_curves = defaultFanCurves();
    m_page = build();
    refreshUi();
}

GtkWidget* FansPage::build() {
    auto* page = adw_preferences_page_new();

    auto* cpuGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(cpuGroup), "CPU Fan Curve");
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(page), ADW_PREFERENCES_GROUP(cpuGroup));

    auto* gpuGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(gpuGroup), "GPU Fan Curve");
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(page), ADW_PREFERENCES_GROUP(gpuGroup));

    const char* pointLabels[FAN_CURVE_POINTS] = {"30C", "50C", "70C", "85C", "95C"};

    for (int i = 0; i < FAN_CURVE_POINTS; ++i) {
        m_cpuScales[i] = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 20, 100, 1));
        gtk_scale_set_draw_value(m_cpuScales[i], TRUE);
        gtk_widget_set_size_request(GTK_WIDGET(m_cpuScales[i]), 220, -1);
        auto* row = adw_action_row_new();
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(row), pointLabels[i]);
        adw_action_row_add_suffix(ADW_ACTION_ROW(row), GTK_WIDGET(m_cpuScales[i]));
        adw_preferences_group_add(ADW_PREFERENCES_GROUP(cpuGroup), row);
        g_signal_connect(m_cpuScales[i], "value-changed", G_CALLBACK(onCurveValueChanged), this);

        m_gpuScales[i] = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 20, 100, 1));
        gtk_scale_set_draw_value(m_gpuScales[i], TRUE);
        gtk_widget_set_size_request(GTK_WIDGET(m_gpuScales[i]), 220, -1);
        auto* gpuRow = adw_action_row_new();
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(gpuRow), pointLabels[i]);
        adw_action_row_add_suffix(ADW_ACTION_ROW(gpuRow), GTK_WIDGET(m_gpuScales[i]));
        adw_preferences_group_add(ADW_PREFERENCES_GROUP(gpuGroup), gpuRow);
        g_signal_connect(m_gpuScales[i], "value-changed", G_CALLBACK(onCurveValueChanged), this);
    }

    return page;
}

void FansPage::onCurveValueChanged(GtkRange*, gpointer self) {
    auto* page = static_cast<FansPage*>(self);
    page->pushCurrentCurve();
}

void FansPage::pushCurrentCurve() {
    if (m_syncingUi) return;
    if (m_currentProfile >= PROFILE_COUNT) return;

    for (int i = 0; i < FAN_CURVE_POINTS; ++i) {
        m_curves[m_currentProfile].cpu[i] =
            static_cast<uint8_t>(gtk_range_get_value(GTK_RANGE(m_cpuScales[i])));
        m_curves[m_currentProfile].gpu[i] =
            static_cast<uint8_t>(gtk_range_get_value(GTK_RANGE(m_gpuScales[i])));
    }

    if (onCurveChanged) {
        onCurveChanged(m_currentProfile, m_curves[m_currentProfile]);
    }
}

void FansPage::refreshUi() {
    if (m_currentProfile >= PROFILE_COUNT) return;

    m_syncingUi = true;
    for (int i = 0; i < FAN_CURVE_POINTS; ++i) {
        g_signal_handlers_block_by_func(m_cpuScales[i], (gpointer)onCurveValueChanged, this);
        g_signal_handlers_block_by_func(m_gpuScales[i], (gpointer)onCurveValueChanged, this);
        gtk_range_set_value(GTK_RANGE(m_cpuScales[i]), m_curves[m_currentProfile].cpu[i]);
        gtk_range_set_value(GTK_RANGE(m_gpuScales[i]), m_curves[m_currentProfile].gpu[i]);
        g_signal_handlers_unblock_by_func(m_cpuScales[i], (gpointer)onCurveValueChanged, this);
        g_signal_handlers_unblock_by_func(m_gpuScales[i], (gpointer)onCurveValueChanged, this);
    }
    m_syncingUi = false;
}

void FansPage::setCurrentProfile(uint8_t profile) {
    m_currentProfile = profile % PROFILE_COUNT;
    refreshUi();
}

void FansPage::setFanCurves(const std::array<FanCurve, PROFILE_COUNT>& curves) {
    m_curves = curves;
    refreshUi();
}
