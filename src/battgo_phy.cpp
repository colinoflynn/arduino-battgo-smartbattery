#include "battgo_phy.h"

void BattGoPHY::scramble(uint8_t seed, uint8_t* data, size_t len) {
  // Go:
  // xor := seed + 136
  // out[i] = in[i] ^ xor
  // xor += seed
  // xor ^= seed
  uint8_t x = (uint8_t)(seed + 136);
  for (size_t i = 0; i < len; i++) {
    data[i] ^= x;
    x = (uint8_t)(x + seed);
    x ^= seed;
  }
}

void BattGoPHY::txAddByte(uint16_t& sum, uint8_t b) {
  sum = (uint16_t)(sum + b);

  // Byte-stuffing: any literal 0xAA is encoded as 0xAA 0xAA
  if (b == 0xAA) {
    _s.write((uint8_t)0xAA);
  }
  _s.write(b);
}

void BattGoPHY::poll() {
  while (_s.available() > 0) {
    uint8_t m = (uint8_t)_s.read();

    if (!_isEscaped) {
      if (m == 0xAA) {
        _isEscaped = true;
        continue;
      }
    } else {
      // we previously saw 0xAA
      _isEscaped = false;
      if (m != 0xAA) {
        // start-of-frame; m is first byte (addrSource)
        _rxState = 1;
        _sum = 0;
        // fallthrough to state machine using m
      }
    }

    switch (_rxState) {
      case 0:
        if (_onPresence) _onPresence(m);
        break;

      case 1:
        _addrSource = m;
        _sum = (uint16_t)(_sum + m);
        _rxState = 2;
        break;

      case 2:
        _addrDest = m;
        _sum = (uint16_t)(_sum + m);
        _rxState = 3;
        break;

      case 3:
        if (m > 0) {
          _payloadPos = 0;
          _rxLen = (int)m + 2; // (seed+payload) + checksum(2)
          _sum = (uint16_t)(_sum + m);
          _rxState = 4;
        } else {
          _rxState = 0;
        }
        break;

      case 4:
        if (_payloadPos < (int)sizeof(_payloadBuf)) {
          _payloadBuf[_payloadPos++] = m;
        } else {
          // overflow protection
          _rxState = 0;
          break;
        }

        if (_payloadPos == _rxLen) {
          if (_rxLen >= 3) {
            const int csumEnd = _rxLen - 2;
            const uint16_t rxCsum = (uint16_t)_payloadBuf[csumEnd] |
                                    ((uint16_t)_payloadBuf[csumEnd + 1] << 8);

            // sum includes addrSource, addrDest, length already; now add seed+payload (but not checksum bytes)
            uint16_t s = _sum;
            for (int i = 0; i < csumEnd; i++) {
              s = (uint16_t)(s + _payloadBuf[i]);
            }

            if (rxCsum == s) {
              const uint8_t seed = _payloadBuf[0];
              uint8_t* pl = &_payloadBuf[1];
              const size_t plLen = (size_t)(csumEnd - 1);

              // descramble in-place (matches Go)
              scramble(seed, pl, plLen);

              if (_onPacket) {
                _onPacket(_addrSource, _addrDest, pl, plLen);
              }
            }
          }
          _rxState = 0;
        }
        break;

      default:
        _rxState = 0;
        break;
    }
  }
}

bool BattGoPHY::sendPacket(uint8_t addrSource, uint8_t addrDest,
                           const uint8_t* payload, size_t payloadLen,
                           bool disableScrambler) {
  uint16_t sum = 0;

  // frame marker
  _s.write((uint8_t)0xAA);

  txAddByte(sum, addrSource);
  txAddByte(sum, addrDest);

  if (payloadLen > 250) return false;

  // length byte = payloadLen + 1 (seed included)
  txAddByte(sum, (uint8_t)(payloadLen + 1));

  uint8_t seed;
  if (disableScrambler) {
    seed = 120; // matches Go
  } else {
    seed = _txSeed++;
  }
  txAddByte(sum, seed);

  if (!disableScrambler && payloadLen > 0) {
    uint8_t tmp[256];
    memcpy(tmp, payload, payloadLen);
    scramble(seed, tmp, payloadLen);
    for (size_t i = 0; i < payloadLen; i++) {
      txAddByte(sum, tmp[i]);
    }
  } else {
    for (size_t i = 0; i < payloadLen; i++) {
      txAddByte(sum, payload[i]);
    }
  }

  const uint16_t finalSum = sum;
  // checksum little-endian; note addByte updates sum too, but we don't use it afterwards
  txAddByte(sum, (uint8_t)(finalSum & 0xFF));
  txAddByte(sum, (uint8_t)(finalSum >> 8));

  return true;
}

bool BattGoPHY::sendBreakMs(uint16_t ms) {
  // Hardware-specific; left as stub.
  (void)ms;
  return false;
}
