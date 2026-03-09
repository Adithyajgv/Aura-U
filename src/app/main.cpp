#include <adwaita.h>
#include "DBusClient.h"
#include "MainWindow.h"
#include <memory>
#include <cstdio>

struct AppData {
    DBusClient client;
    std::unique_ptr<MainWindow> window;
};

static void onActivate(AdwApplication* app, gpointer userData) {
    auto* data = static_cast<AppData*>(userData);

    if (!data->client.connect()) {
        auto* dialog = adw_alert_dialog_new(
            "Daemon Not Running",
            "Could not connect to the Aura-U daemon.\n"
            "Make sure daura-u is running:\n"
            "  systemctl start daura-u"
        );
        adw_alert_dialog_add_response(ADW_ALERT_DIALOG(dialog), "ok", "OK");
        adw_dialog_present(ADW_DIALOG(dialog), nullptr);
    }

    data->window = std::make_unique<MainWindow>(app, data->client);
}

int main(int argc, char** argv) {
    AppData data;

    auto* app = adw_application_new(
        "io.AuraU",
        G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(app, "activate", G_CALLBACK(onActivate), &data);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}