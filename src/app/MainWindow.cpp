#include "MainWindow.h"

MainWindow::MainWindow(AdwApplication* app, DBusClient& client)
    : m_client(client)
{
    buildWindow(app);

    m_client.onModeChanged = [this](uint8_t mode) {
        m_auraPage->syncMode(mode);
    };
    m_client.onBrightnessChanged = [this](uint8_t level) {
        m_auraPage->syncBrightness(level);
    };
}

void MainWindow::buildWindow(AdwApplication* app) {
    m_window = ADW_APPLICATION_WINDOW(adw_application_window_new(GTK_APPLICATION(app)));
    gtk_window_set_title(GTK_WINDOW(m_window), "Aura-U");
    gtk_window_set_default_size(GTK_WINDOW(m_window), 560, 660);
    gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);

    auto* toolbar = adw_toolbar_view_new();
    auto* header  = adw_header_bar_new();

    auto* logo = gtk_image_new_from_file(INSTALL_PREFIX "/share/icons/hicolor/256x256/apps/rog_logo.png");
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
        m_auraPage->widget(),
        "aura", "Aura",
        "preferences-color-symbolic"
    );

    adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbar), stack);
    adw_application_window_set_content(m_window, toolbar);
    gtk_window_present(GTK_WINDOW(m_window));
}