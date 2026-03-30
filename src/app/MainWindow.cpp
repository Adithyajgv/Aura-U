#include "MainWindow.h"

MainWindow::MainWindow(AdwApplication* app, DBusClient& client)
    : m_client(client)
{
    m_homePage = std::make_unique<HomePage>();
    m_fansPage = std::make_unique<FansPage>();
    m_auraPage = std::make_unique<AuraPage>(m_client);

    uint8_t persistedProfile = 1;
    m_client.getProfileState(persistedProfile, m_fanCurves);
    m_currentProfile = persistedProfile;

    m_homePage->setCurrentProfile(m_currentProfile);
    m_fansPage->setCurrentProfile(m_currentProfile);
    m_fansPage->setFanCurves(m_fanCurves);

    buildWindow(app);

    m_client.onModeChanged = [this](uint8_t mode) {
        m_auraPage->syncMode(mode);
    };
    m_client.onBrightnessChanged = [this](uint8_t level) {
        m_auraPage->syncBrightness(level);
    };
    m_client.onCurrentProfileChanged = [this](uint8_t profile) {
        onProfileChanged(profile, false);
    };

    m_homePage->onProfileActivated = [this](uint8_t profile) {
        onProfileChanged(profile, true);
    };
    m_fansPage->onCurveChanged = [this](uint8_t profile, const FanCurve& curve) {
        const uint8_t idx = profile % PROFILE_COUNT;
        m_fanCurves[idx] = curve;
        m_client.setFanCurve(idx, curve);
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

    const auto& names = profileNames();
    auto* profileList = gtk_string_list_new(names.data());
    m_profileDD = GTK_DROP_DOWN(gtk_drop_down_new(G_LIST_MODEL(profileList), nullptr));
    gtk_drop_down_set_selected(m_profileDD, m_currentProfile);
    gtk_widget_set_tooltip_text(GTK_WIDGET(m_profileDD), "Active profile");
    g_signal_connect(m_profileDD, "notify::selected", G_CALLBACK(+[](GtkDropDown* dd, GParamSpec*, gpointer self) {
        auto* w = static_cast<MainWindow*>(self);
        if (w->m_syncingProfile) return;
        w->onProfileChanged((uint8_t)gtk_drop_down_get_selected(dd), true);
    }), this);
    adw_header_bar_pack_end(ADW_HEADER_BAR(header), GTK_WIDGET(m_profileDD));

    adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(toolbar), header);

    adw_view_stack_add_titled_with_icon(
        ADW_VIEW_STACK(stack),
        m_homePage->widget(),
        "home", "Home",
        "go-home-symbolic"
    );
    adw_view_stack_add_titled_with_icon(
        ADW_VIEW_STACK(stack),
        m_fansPage->widget(),
        "fans", "Fans",
        "weather-clear-night-symbolic"
    );
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

void MainWindow::onProfileChanged(uint8_t profile, bool pushToDaemon) {
    m_currentProfile = profile % PROFILE_COUNT;
    m_homePage->setCurrentProfile(m_currentProfile);
    m_fansPage->setCurrentProfile(m_currentProfile);
    m_fansPage->setFanCurves(m_fanCurves);

    m_syncingProfile = true;
    if (m_profileDD) gtk_drop_down_set_selected(m_profileDD, m_currentProfile);
    m_syncingProfile = false;

    if (pushToDaemon) {
        m_client.setCurrentProfile(m_currentProfile);
    }
}