#include "AuraCtrl.h"
#include <cstring>

static Packet base() {
    Packet p{};
    p[0] = 0x5d;
    p[1] = 0xb3;
    return p;
}

Packet AuraCtrl::staticColor(Color c, uint8_t zone) {
    auto p = base();
    p[2] = zone;
    p[4] = c.r;
    p[5] = c.g;
    p[6] = c.b;
    return p;
}

Packet AuraCtrl::breathe(Color c, uint8_t speed) {
    auto p = base();
    p[3] = 1;
    p[4] = c.r;
    p[5] = c.g;
    p[6] = c.b;
    p[7] = speed;
    return p;
}

Packet AuraCtrl::colorCycle(uint8_t speed) {
    auto p = base();
    p[3] = 2;
    p[4] = 0xff;
    p[7] = speed;
    return p;
}

Packet AuraCtrl::multiStatic(std::array<Color, 4> colors, uint8_t zone) {
    auto p = base();
    p[2] = zone + 1;
    p[4] = colors[zone].r;
    p[5] = colors[zone].g;
    p[6] = colors[zone].b;
    return p;
}

Packet AuraCtrl::rainbow(uint8_t speed) {
    auto p = base();
    p[3] = 3;
    p[7] = speed;
    return p;
}

Packet AuraCtrl::brightness(uint8_t level) {
    Packet p{};
    p[0] = 0x5a;
    p[1] = 0xba;
    p[2] = 0xc5;
    p[3] = 0xc4;
    p[4] = level;
    return p;
}

Packet AuraCtrl::setPacket() {
    Packet p{};
    p[0] = 0x5d;
    p[1] = 0xb5;
    return p;
}

Packet AuraCtrl::applyPacket() {
    Packet p{};
    p[0] = 0x5d;
    p[1] = 0xb4;
    return p;
}

Packet AuraCtrl::powerPacket(bool lightbar) {
    Packet p{};
    p[0] = 0x5d;
    p[1] = 0xbd;
    p[2] = 0x01;
    p[3] = 0xff;
    p[4] = lightbar ? 0x07 : 0x00; // lightbar on/off
    p[5] = 0xff;
    p[6] = 0xff;
    p[7] = 0xff;
    return p;
}

Packet AuraCtrl::initKeyboard() {
    Packet p{};
    const uint8_t init[] = {
        0x5a,0x41,0x53,0x55,0x53,0x20,0x54,0x65,
        0x63,0x68,0x2e,0x49,0x6e,0x63,0x2e,0x00
    };
    std::memcpy(p.data(), init, sizeof(init));
    return p;
}