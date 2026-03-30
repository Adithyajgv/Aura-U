#pragma once
#include <adwaita.h>
#include "DBusClient.h"
#include "ProfileState.h"
#include "AuraPage.h"
#include "HomePage.h"
#include "FansPage.h"
#include <memory>

class MainWindow {
public:
    MainWindow(AdwApplication* app, DBusClient& client);

private:
    void buildWindow(AdwApplication* app);
    void onProfileChanged(uint8_t profile, bool pushToDaemon);

    DBusClient&            m_client;
    AdwApplicationWindow*  m_window   = nullptr;
    GtkDropDown*           m_profileDD = nullptr;
    bool                   m_syncingProfile = false;
    uint8_t                m_currentProfile = 1;
    std::array<FanCurve, PROFILE_COUNT> m_fanCurves = defaultFanCurves();
    std::unique_ptr<HomePage> m_homePage;
    std::unique_ptr<FansPage> m_fansPage;
    std::unique_ptr<AuraPage> m_auraPage;

};