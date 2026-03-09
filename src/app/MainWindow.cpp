#include "MainWindow.h"
#include "AuraCtrl.h"
#include <cstdio>

static const char* MODE_LABELS[] = { "Static", "Breathe", "Color Cycle", "Rainbow", nullptr };

MainWindow::MainWindow(AdwApplication* app, DBusClient& client)
    : m_client(client)
{
    buildWindow(app);

    m_client.onModeChanged = [this](uint8_t mode) {
        syncMode(mode);
    };
    m_client.onBrightnessChanged = [this](uint8_t level) {
        syncBrightness(level);
    };

    uint8_t mode, brightness, speed, lightbarMode;
    std::array<Color, 4> zones;
    if (m_client.getState(mode, brightness, speed, zones, lightbarMode)) {
        syncMode(mode);
        syncBrightness(brightness);
        g_signal_handlers_block_by_func(m_lightbarDD, (gpointer)onLightbarModeChanged, this);
        gtk_drop_down_set_selected(m_lightbarDD, lightbarMode);
        g_signal_handlers_unblock_by_func(m_lightbarDD, (gpointer)onLightbarModeChanged, this);
        g_signal_handlers_block_by_func(m_speedScale, (gpointer)onSpeedChanged, this);
        uint8_t speedStep = (speed <= 225) ? 1 : (speed <= 235) ? 2 : 3;
        gtk_range_set_value(GTK_RANGE(m_speedScale), speedStep);
        g_signal_handlers_unblock_by_func(m_speedScale, (gpointer)onSpeedChanged, this);
        for (int i = 0; i < 4; ++i) {
            GdkRGBA rgba = { zones[i].r/255.0f, zones[i].g/255.0f, zones[i].b/255.0f, 1.0f };
            gtk_color_dialog_button_set_rgba(m_zoneButtons[i], &rgba);
        }
    } else {
        fprintf(stderr, "Failed to load state from file\n");
    }
}

void MainWindow::buildWindow(AdwApplication* app) {
    m_window = ADW_APPLICATION_WINDOW(adw_application_window_new(GTK_APPLICATION(app)));
    gtk_window_set_title(GTK_WINDOW(m_window), "Aura-U");
    gtk_window_set_default_size(GTK_WINDOW(m_window), 560, 660);
    gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);

    auto* toolbar = adw_toolbar_view_new();

    auto* header = adw_header_bar_new();

    auto* logo = gtk_image_new_from_file("share/icons/hicolor/256x256/apps/rog_logo.png");
    gtk_image_set_pixel_size(GTK_IMAGE(logo), 28);
    adw_header_bar_pack_start(ADW_HEADER_BAR(header), logo);

    auto* stack    = adw_view_stack_new();
    auto* switcher = adw_view_switcher_new();
    adw_view_switcher_set_stack(ADW_VIEW_SWITCHER(switcher), ADW_VIEW_STACK(stack));
    adw_view_switcher_set_policy(ADW_VIEW_SWITCHER(switcher), ADW_VIEW_SWITCHER_POLICY_WIDE);
    adw_header_bar_set_title_widget(ADW_HEADER_BAR(header), switcher);

    adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(toolbar), header);

    adw_view_stack_add_titled_with_icon(
        ADW_VIEW_STACK(stack),
        buildAuraPage(),
        "aura", "Aura",
        "preferences-color-symbolic"
    );

    adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbar), stack);
    adw_application_window_set_content(m_window, toolbar);

    gtk_window_present(GTK_WINDOW(m_window));
}

GtkWidget* MainWindow::buildAuraPage() {
    auto* page = adw_preferences_page_new();

    //Mode ctrl
    auto* modeGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(modeGroup), "Mode");
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(page), ADW_PREFERENCES_GROUP(modeGroup));

    auto* modeList = gtk_string_list_new(MODE_LABELS);
    m_modeDD = GTK_DROP_DOWN(gtk_drop_down_new(G_LIST_MODEL(modeList), nullptr));
    gtk_widget_set_valign(GTK_WIDGET(m_modeDD), GTK_ALIGN_CENTER);

    auto* modeRow = adw_action_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(modeRow), "Effect");
    adw_action_row_add_suffix(ADW_ACTION_ROW(modeRow), GTK_WIDGET(m_modeDD));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(modeGroup), modeRow);

    g_signal_connect(m_modeDD, "notify::selected", G_CALLBACK(onModeChanged), this);

    //Brightnes ctrl
    auto* brightGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(brightGroup), "Brightness");
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(page), ADW_PREFERENCES_GROUP(brightGroup));

    m_brightnessScale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 3, 1));
    gtk_scale_set_draw_value(m_brightnessScale, TRUE);
    gtk_widget_set_size_request(GTK_WIDGET(m_brightnessScale), 200, -1);
    gtk_widget_set_valign(GTK_WIDGET(m_brightnessScale), GTK_ALIGN_CENTER);

    auto* brightRow = adw_action_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(brightRow), "Level");
    adw_action_row_add_suffix(ADW_ACTION_ROW(brightRow), GTK_WIDGET(m_brightnessScale));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(brightGroup), brightRow);

    g_signal_connect(m_brightnessScale, "value-changed", G_CALLBACK(onBrightnessChanged), this);

    auto* lightbarGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(lightbarGroup), "Lightbar");
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(page), ADW_PREFERENCES_GROUP(lightbarGroup));

    const char* lightbarLabels[] = { "Never", "AC Only", "Always", nullptr };
    auto* lightbarList = gtk_string_list_new(lightbarLabels);
    m_lightbarDD = GTK_DROP_DOWN(gtk_drop_down_new(G_LIST_MODEL(lightbarList), nullptr));
    gtk_widget_set_valign(GTK_WIDGET(m_lightbarDD), GTK_ALIGN_CENTER);
    gtk_drop_down_set_selected(m_lightbarDD, 1); // default AC only

    auto* lightbarRow = adw_action_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(lightbarRow), "Mode");
    adw_action_row_add_suffix(ADW_ACTION_ROW(lightbarRow), GTK_WIDGET(m_lightbarDD));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(lightbarGroup), lightbarRow);

    g_signal_connect(m_lightbarDD, "notify::selected", G_CALLBACK(onLightbarModeChanged), this);


    // Speed ctrl
    auto* speedGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(speedGroup), "Animation");
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(page), ADW_PREFERENCES_GROUP(speedGroup));

    m_speedScale = GTK_SCALE(gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 3, 1));
    gtk_scale_set_draw_value(m_speedScale, TRUE);
    gtk_range_set_value(GTK_RANGE(m_speedScale), 0);
    gtk_widget_set_size_request(GTK_WIDGET(m_speedScale), 200, -1);
    gtk_widget_set_valign(GTK_WIDGET(m_speedScale), GTK_ALIGN_CENTER);

    m_speedRow = adw_action_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_speedRow), "Speed");
    adw_action_row_add_suffix(ADW_ACTION_ROW(m_speedRow), GTK_WIDGET(m_speedScale));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(speedGroup), m_speedRow);

    g_signal_connect(m_speedScale, "value-changed", G_CALLBACK(onSpeedChanged), this);


    // Zone color ctrl
    m_zoneColorGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(m_zoneColorGroup), "Zone Colors");
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(page), ADW_PREFERENCES_GROUP(m_zoneColorGroup));

    auto* colorDialog = gtk_color_dialog_new();
    const char* zoneLabels[] = { "Zone 1 — Left", "Zone 2 — Center Left",
                                  "Zone 3 — Center Right", "Zone 4 — Right" };
    for (int i = 0; i < 4; ++i) {
        m_zoneButtons[i] = GTK_COLOR_DIALOG_BUTTON(gtk_color_dialog_button_new(colorDialog));
        gtk_widget_set_valign(GTK_WIDGET(m_zoneButtons[i]), GTK_ALIGN_CENTER);

        auto* row = adw_action_row_new();
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(row), zoneLabels[i]);
        adw_action_row_add_suffix(ADW_ACTION_ROW(row), GTK_WIDGET(m_zoneButtons[i]));
        adw_preferences_group_add(ADW_PREFERENCES_GROUP(m_zoneColorGroup), row);

        g_signal_connect(m_zoneButtons[i], "notify::rgba", G_CALLBACK(onZoneColorSet), this);
    }

    // Breathe color ctrl
    auto* breatheGroup = adw_preferences_group_new();
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(page), ADW_PREFERENCES_GROUP(breatheGroup));

    m_breatheColor = GTK_COLOR_DIALOG_BUTTON(gtk_color_dialog_button_new(colorDialog));
    gtk_widget_set_valign(GTK_WIDGET(m_breatheColor), GTK_ALIGN_CENTER);

    m_breatheColorRow = adw_action_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(m_breatheColorRow), "Color");
    adw_action_row_add_suffix(ADW_ACTION_ROW(m_breatheColorRow), GTK_WIDGET(m_breatheColor));
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(breatheGroup), m_breatheColorRow);

    g_signal_connect(m_breatheColor, "notify::rgba", G_CALLBACK(onBreatheColorSet), this);

    updateControlVisibility();
    return GTK_WIDGET(page);
}

void MainWindow::updateControlVisibility() {
    uint32_t mode = gtk_drop_down_get_selected(m_modeDD);
    gtk_widget_set_visible(m_speedRow, mode != 0);
    gtk_widget_set_visible(m_zoneColorGroup, mode == 0);
    gtk_widget_set_visible(m_breatheColorRow, mode == 1);
}

void MainWindow::onModeChanged(GtkDropDown*, GParamSpec*, gpointer self) {
    auto* w = static_cast<MainWindow*>(self);
    w->updateControlVisibility();
    w->applyCurrentSettings();
}

static guint s_speedTimer = 0;
void MainWindow::onSpeedChanged(GtkRange*, gpointer self) {
    if (s_speedTimer) g_source_remove(s_speedTimer);
    s_speedTimer = g_timeout_add(300, [](gpointer data) -> gboolean {
        s_speedTimer = 0;
        static_cast<MainWindow*>(data)->applyCurrentSettings();
        return G_SOURCE_REMOVE;
    }, self);
}

void MainWindow::onBrightnessChanged(GtkRange* range, gpointer self) {
    auto* w = static_cast<MainWindow*>(self);
    w->m_client.setBrightness((uint8_t)gtk_range_get_value(range));
}

void MainWindow::onLightbarModeChanged(GtkDropDown* dd, GParamSpec*, gpointer self) {
    auto* w = static_cast<MainWindow*>(self);
    w->m_client.setLightbarMode((uint8_t)gtk_drop_down_get_selected(dd));
}

static guint s_colorTimer = 0;
void MainWindow::onZoneColorSet(GtkColorDialogButton*, GParamSpec*, gpointer self) {
    if (s_colorTimer) g_source_remove(s_colorTimer);
    s_colorTimer = g_timeout_add(300, [](gpointer data) -> gboolean {
        s_colorTimer = 0;
        static_cast<MainWindow*>(data)->applyCurrentSettings();
        return G_SOURCE_REMOVE;
    }, self);
}

void MainWindow::onBreatheColorSet(GtkColorDialogButton*, GParamSpec*, gpointer self) {
    if (s_colorTimer) g_source_remove(s_colorTimer);
    s_colorTimer = g_timeout_add(300, [](gpointer data) -> gboolean {
        s_colorTimer = 0;
        static_cast<MainWindow*>(data)->applyCurrentSettings();
        return G_SOURCE_REMOVE;
    }, self);
}

void MainWindow::applyCurrentSettings() {
    if (!m_zoneButtons[0] || !m_breatheColor) return;
    uint32_t mode  = gtk_drop_down_get_selected(m_modeDD);
    uint8_t  speed = 215 + 10*(uint8_t)gtk_range_get_value(GTK_RANGE(m_speedScale));

    switch (mode) {
        case 0:
            for (int i = 0; i < 4; ++i) {
                const GdkRGBA* rgba = gtk_color_dialog_button_get_rgba(m_zoneButtons[i]);
                m_client.setStaticColor(
                    (uint8_t)(rgba->red   * 255),
                    (uint8_t)(rgba->green * 255),
                    (uint8_t)(rgba->blue  * 255),
                    i
                );
            }
            break;
        case 1: {
            const GdkRGBA* rgba = gtk_color_dialog_button_get_rgba(m_breatheColor);
            m_client.setBreathe(
                (uint8_t)(rgba->red   * 255),
                (uint8_t)(rgba->green * 255),
                (uint8_t)(rgba->blue  * 255),
                speed
            );
            break;
        }
        case 2:
            m_client.setColorCycle(speed);
            break;
        case 3:
            m_client.setRainbow(speed);
            break;
    }
}

void MainWindow::syncMode(uint8_t mode) {
    g_signal_handlers_block_by_func(m_modeDD, (gpointer)onModeChanged, this);
    gtk_drop_down_set_selected(m_modeDD, mode);
    g_signal_handlers_unblock_by_func(m_modeDD, (gpointer)onModeChanged, this);
    updateControlVisibility();
}

void MainWindow::syncBrightness(uint8_t level) {
    g_signal_handlers_block_by_func(m_brightnessScale, (gpointer)onBrightnessChanged, this);
    gtk_range_set_value(GTK_RANGE(m_brightnessScale), level);
    g_signal_handlers_unblock_by_func(m_brightnessScale, (gpointer)onBrightnessChanged, this);
}