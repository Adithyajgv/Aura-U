#include "HomePage.h"

HomePage::HomePage() {
    m_page = build();
}

GtkWidget* HomePage::build() {
    auto* page = adw_preferences_page_new();

    auto* profileGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(profileGroup), "Profiles");
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(page), ADW_PREFERENCES_GROUP(profileGroup));

    auto* statusRow = adw_action_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(statusRow), "Current profile");
    adw_action_row_set_subtitle(ADW_ACTION_ROW(statusRow), profileNames()[m_currentProfile]);
    m_statusRow = ADW_ACTION_ROW(statusRow);
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(profileGroup), statusRow);

    for (uint8_t i = 0; i < PROFILE_COUNT; ++i) {
        auto* row = adw_action_row_new();
        adw_preferences_row_set_title(ADW_PREFERENCES_ROW(row), profileNames()[i]);
        adw_action_row_set_subtitle(ADW_ACTION_ROW(row), "Performance and fan settings profile");

        auto* button = gtk_button_new_with_label("Activate");
        gtk_widget_set_valign(button, GTK_ALIGN_CENTER);
        g_object_set_data(G_OBJECT(button), "profile-index", GUINT_TO_POINTER(i));
        g_signal_connect(button, "clicked", G_CALLBACK(onActivateClicked), this);
        adw_action_row_add_suffix(ADW_ACTION_ROW(row), button);
        adw_action_row_set_activatable_widget(ADW_ACTION_ROW(row), button);
        m_activateButtons[i] = GTK_BUTTON(button);

        adw_preferences_group_add(ADW_PREFERENCES_GROUP(profileGroup), row);
    }

    auto* helpGroup = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(helpGroup), "How it works");
    adw_preferences_page_add(ADW_PREFERENCES_PAGE(page), ADW_PREFERENCES_GROUP(helpGroup));

    auto* infoRow = adw_action_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(infoRow), "Profile switching");
    adw_action_row_set_subtitle(
        ADW_ACTION_ROW(infoRow),
        "Use the top-right dropdown to change the active profile. Home and Fans stay in sync.");
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(helpGroup), infoRow);

    setCurrentProfile(m_currentProfile);
    return page;
}

void HomePage::onActivateClicked(GtkButton* button, gpointer self) {
    auto* page = static_cast<HomePage*>(self);
    uint8_t profile = static_cast<uint8_t>(GPOINTER_TO_UINT(
        g_object_get_data(G_OBJECT(button), "profile-index")));
    if (profile < PROFILE_COUNT && page->onProfileActivated) {
        page->onProfileActivated(profile);
    }
}

void HomePage::setCurrentProfile(uint8_t profile) {
    if (profile >= PROFILE_COUNT) return;
    m_currentProfile = profile;

    if (m_statusRow) {
        adw_action_row_set_subtitle(ADW_ACTION_ROW(m_statusRow), profileNames()[m_currentProfile]);
    }
    for (uint8_t i = 0; i < PROFILE_COUNT; ++i) {
        if (!m_activateButtons[i]) continue;
        gtk_widget_set_sensitive(GTK_WIDGET(m_activateButtons[i]), i != m_currentProfile);
    }
}
