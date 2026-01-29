#pragma once
#include <Arduino.h>
#include "battgo_phy.h"

// BattGoBus implements a minimal synchronous command/reply layer.
// It mirrors the essentials of go-battgo's controller layer for a single device:
// - Host address is 0x01
// - Send command payload to device address
// - Wait for reply from that device to address 0x01
class BattGoBus {
public:
  BattGoBus(BattGoPHY& phy, uint8_t deviceAddr) : _phy(phy), _deviceAddr(deviceAddr) {}

  void setDeviceAddr(uint8_t addr) { _deviceAddr = addr; }
  uint8_t deviceAddr() const { return _deviceAddr; }

  // Send `cmd` (1 byte) and wait for a reply whose first payload byte equals `expectedReply`.
  // On success, copies reply payload into outBuf and sets outLen.
  bool command1(uint8_t cmd,
                uint8_t expectedReply,
                uint8_t* outBuf,
                size_t& outLen,
                uint16_t timeoutMs = 150);

  // Send an arbitrary command buffer and wait for reply.
  bool commandN(const uint8_t* cmdBuf, size_t cmdLen,
                uint8_t expectedReply,
                uint8_t* outBuf,
                size_t& outLen,
                uint16_t timeoutMs);

    // Send an arbitrary command buffer and wait for reply
  bool commandN(const uint8_t* cmdBuf, size_t cmdLen,
                uint8_t expectedReply,
                uint8_t* outBuf,
                size_t& outLen,
                uint16_t timeoutMs,
                uint8_t txdevaddr);

private:
  BattGoPHY& _phy;
  uint8_t _deviceAddr;

  volatile bool _gotReply = false;
  uint8_t _replyBuf[256];
  size_t _replyLen = 0;

  bool handlePacket(uint8_t src, uint8_t dst,
                    const uint8_t* payload, size_t len);

  static bool staticPacketHandler(uint8_t src, uint8_t dst,
                                  const uint8_t* payload, size_t len);

  static BattGoBus* _instance;
};
