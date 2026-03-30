#pragma once
#include <adwaita.h>
#include <array>
#include <cstdint>
#include <functional>
#include "ProfileState.h"

class HomePage {
public:
    HomePage();
    GtkWidget* widget() { return m_page; }
    void setCurrentProfile(uint8_t profile);
    std::function<void(uint8_t)> onProfileActivated;

private:
    GtkWidget* build();
    static void onActivateClicked(GtkButton*, gpointer self);

    GtkWidget* m_page = nullptr;
    AdwActionRow* m_statusRow = nullptr;
    std::array<GtkButton*, PROFILE_COUNT> m_activateButtons{};
    uint8_t m_currentProfile = 1;
};
