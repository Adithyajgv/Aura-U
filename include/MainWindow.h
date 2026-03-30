#pragma once
#include <adwaita.h>
#include "DBusClient.h"
#include "AuraPage.h"
#include <memory>

class MainWindow {
public:
    MainWindow(AdwApplication* app, DBusClient& client);

private:
    void buildWindow(AdwApplication* app);

    DBusClient&            m_client;
    AdwApplicationWindow*  m_window   = nullptr;
    std::unique_ptr<AuraPage> m_auraPage = std::make_unique<AuraPage>(m_client);

};