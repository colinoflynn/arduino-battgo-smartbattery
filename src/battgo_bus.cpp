#include "battgo_bus.h"

BattGoBus* BattGoBus::_instance = nullptr;

bool BattGoBus::staticPacketHandler(uint8_t src, uint8_t dst,
                                    const uint8_t* payload, size_t len) {
  if (_instance) return _instance->handlePacket(src, dst, payload, len);
  return true;
}

bool BattGoBus::handlePacket(uint8_t src, uint8_t dst,
                             const uint8_t* payload, size_t len) {
  // Only accept packets addressed to host (0x01) from our device.
  if (dst != 1 || src != _deviceAddr) return true;

  if (len > sizeof(_replyBuf)) return true;

  memcpy(_replyBuf, payload, len);
  _replyLen = len;
  _gotReply = true;
  return true;
}

bool BattGoBus::command1(uint8_t cmd,
                         uint8_t expectedReply,
                         uint8_t* outBuf,
                         size_t& outLen,
                         uint16_t timeoutMs) {
  return commandN(&cmd, 1, expectedReply, outBuf, outLen, timeoutMs);
}

bool BattGoBus::commandN(const uint8_t* cmdBuf, size_t cmdLen,
                         uint8_t expectedReply,
                         uint8_t* outBuf,
                         size_t& outLen,
                         uint16_t timeoutMs) {
  return commandN(cmdBuf, cmdLen, expectedReply, outBuf, outLen, timeoutMs, _deviceAddr);
}

bool BattGoBus::commandN(const uint8_t* cmdBuf, size_t cmdLen,
                         uint8_t expectedReply,
                        uint8_t* outBuf,
                        size_t& outLen,
                        uint16_t timeoutMs,
                        uint8_t txdevaddr) {
  _instance = this;
  _gotReply = false;
  _replyLen = 0;

  _phy.setPacketHandler(staticPacketHandler);

  // Send to device, from host addr 0x01
  _phy.sendPacket(1, txdevaddr, cmdBuf, cmdLen);

  const uint32_t start = millis();
  while ((millis() - start) < timeoutMs) {
  _phy.poll();
  if (_gotReply) {
    if (_replyLen > 0 && _replyBuf[0] == expectedReply) {
      if (_replyLen > outLen) return false;
        memcpy(outBuf, _replyBuf, _replyLen);
          outLen = _replyLen;
          return true;
        }
      return false;
    }
  }

return false;
}
