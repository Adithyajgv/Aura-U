#pragma once
#include <cstdint>
#include <array>

constexpr int MSG_LEN = 17;
using Packet = std::array<uint8_t, MSG_LEN>;

struct Color { uint8_t r, g, b; };

enum class AuraMode {
    Static = 0,
    Breathe,
    ColorCycle,
    Rainbow,
    COUNT
};

class AuraCtrl {
public:
    static Packet staticColor(Color c, uint8_t zone = 0);
    static Packet breathe(Color c, uint8_t speed = 0xeb);
    static Packet colorCycle(uint8_t speed = 0xeb);
    static Packet multiStatic(std::array<Color, 4> colors, uint8_t zone);
    static Packet rainbow(uint8_t speed);
    static Packet brightness(uint8_t level);
    static Packet setPacket();
    static Packet applyPacket();
    static Packet powerPacket(bool lightbar = true);
    static Packet initKeyboard();
};