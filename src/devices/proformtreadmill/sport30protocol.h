#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace ProFormSport30Protocol {
using AuthorizationPacket = std::array<uint8_t, 20>;
using AuthorizationPackets = std::array<AuthorizationPacket, 3>;

std::array<uint8_t, 20> makeStateCommand(uint8_t state);
std::array<uint8_t, 20> makeStartCommand(double currentInclination);
bool parseAuthorizationPackets(const std::string &input, AuthorizationPackets &packets);
} // namespace ProFormSport30Protocol
