#pragma once
#include <array>
#include <cstdint>

constexpr uint8_t PROFILE_COUNT = 3;
constexpr uint8_t FAN_CURVE_POINTS = 5;

struct FanCurve {
    std::array<uint8_t, FAN_CURVE_POINTS> cpu{};
    std::array<uint8_t, FAN_CURVE_POINTS> gpu{};
};

inline const std::array<const char*, PROFILE_COUNT>& profileNames() {
    static const std::array<const char*, PROFILE_COUNT> names = {
        "Silent",
        "Balanced",
        "Turbo"
    };
    return names;
}

inline std::array<FanCurve, PROFILE_COUNT> defaultFanCurves() {
    std::array<FanCurve, PROFILE_COUNT> curves{};

    curves[0].cpu = {20, 30, 45, 65, 85};
    curves[0].gpu = {20, 30, 45, 65, 85};

    curves[1].cpu = {25, 40, 60, 80, 100};
    curves[1].gpu = {25, 40, 60, 80, 100};

    curves[2].cpu = {35, 55, 75, 90, 100};
    curves[2].gpu = {35, 55, 75, 90, 100};

    return curves;
}
