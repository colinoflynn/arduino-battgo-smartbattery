/*
Distributed under the BSD 2-Clause License - see LICENSE file in repo at
https://github.com/colinoflynn/arduino-battgo-smartbattery

Copyright (c) 2026, Colin O'Flynn
This project is a derivative of the go-battgo project which is:
Copyright (c) 2021, Bertold Van den Bergh
*/

#pragma once
#include <Arduino.h>
#include <Stream.h>

typedef bool (*BattGoPacketHandler)(uint8_t src, uint8_t dst, const uint8_t* payload, size_t len);
typedef void (*BattGoPresenceHandler)(uint8_t b);

// BattGoPHY implements the framing/checksum/scramble logic
class BattGoPHY {
public:
  explicit BattGoPHY(Stream& serial) : _s(serial) {}

  void setPacketHandler(BattGoPacketHandler h) { _onPacket = h; }
  void setPresenceHandler(BattGoPresenceHandler h) { _onPresence = h; }

  // Call often from loop(); parses incoming bytes and invokes callbacks.
  void poll();

  // Sends one packet (addressed) with BattGO PHY framing.
  // `payload` = higher-layer bytes (NOT including seed).
  bool sendPacket(uint8_t addrSource, uint8_t addrDest,
                  const uint8_t* payload, size_t payloadLen,
                  bool disableScrambler=false);

  // Optional: send a "break" low for ~70ms. Hardware-specific; default returns false.
  bool sendBreakMs(uint16_t ms);

private:
  Stream& _s;

  BattGoPacketHandler _onPacket = nullptr;
  BattGoPresenceHandler _onPresence = nullptr;

  // RX state machine (matches Go)
  uint8_t _rxState = 0;
  int _rxLen = 0;
  uint8_t _addrSource = 0, _addrDest = 0;
  uint16_t _sum = 0;
  bool _isEscaped = false;

  uint8_t _payloadBuf[260];
  int _payloadPos = 0;

  // TX seed increments per packet (wrap at 0xFF)
  uint8_t _txSeed = 0;

  static void scramble(uint8_t seed, uint8_t* data, size_t len);

  void txAddByte(uint16_t& sum, uint8_t b);
};
