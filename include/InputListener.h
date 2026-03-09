#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <string>

class InputListener {
public:
    InputListener();
    ~InputListener();

    // Set before calling start()
    std::function<void()> onBrightnessDown;  // Fn+F2 = KEY_KBDILLUMDOWN
    std::function<void()> onBrightnessUp;    // Fn+F3 = KEY_KBDILLUMUP
    std::function<void()> onAuraButton;      // Fn+F4 = KEY_PROG3

    bool start();
    void stop();

private:
    void run(const std::string& devicePath);
    std::string findKeyboardDevice();

    std::thread      m_thread;
    std::atomic_bool m_running{false};
};