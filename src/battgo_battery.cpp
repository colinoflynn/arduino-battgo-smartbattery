#include "battgo_battery.h"

bool BattGoBattery::readFactoryInfo(BattGoFactoryInfo& out, uint16_t timeoutMs) {
  uint8_t buf[64];
  size_t len = sizeof(buf);
  if (!_bus.command1(0x88, 0x89, buf, len, timeoutMs)) return false;
  return decodeFactoryInfo(buf, len, out);
}

bool BattGoBattery::readCycleInfo(BattGoCycleInfo& out, uint16_t timeoutMs) {
  uint8_t buf[64];
  size_t len = sizeof(buf);
  if (!_bus.command1(0x4A, 0x4B, buf, len, timeoutMs)) return false;
  return decodeCycleInfo(buf, len, out);
}

bool BattGoBattery::readState(BattGoState& out, uint8_t numCellsHint, uint16_t timeoutMs) {
  // Go sends: []byte{0x44, 0, byte(numCells-1)}
  // If unknown, Go often assumes 8 cells.
  uint8_t n = numCellsHint;
  if (n == 0) n = 8;
  if (n < 1) n = 1;
  if (n > 16) n = 16;

  uint8_t cmd[3] = {0x44, 0x00, (uint8_t)(n - 1)};

  uint8_t buf[128];
  size_t len = sizeof(buf);
  if (!_bus.commandN(cmd, sizeof(cmd), 0x45, buf, len, timeoutMs)) return false;
  return decodeState(buf, len, out);
}

bool BattGoBattery::decodeFactoryInfo(const uint8_t* buf, size_t len, BattGoFactoryInfo& o) {
  // Go: len >= 24, reply 0x89
  if (len < 24 || buf[0] != 0x89) return false;

  o.type = (BattGoBatteryType)buf[1];
  o.cellDischargeCutoffV   = u16le(buf+2)  / 1000.0f;
  o.cellDischargeNormalV   = u16le(buf+4)  / 1000.0f;
  o.cellChargeMaxV         = u16le(buf+6)  / 1000.0f;
  o.cellStorageDefaultV    = u16le(buf+8)  / 1000.0f;
  o.cellCapacityAh         = u32le(buf+10) / 1000.0f;

  o.chargeMaxCurrentA      = (u16le(buf+14) / 10.0f) * o.cellCapacityAh;
  o.dischargeMaxCurrentA   = (u16le(buf+16) / 10.0f) * o.cellCapacityAh;

  o.tempUseLowC            = (int8_t)buf[18];
  o.tempUseHighC           = (int8_t)buf[19];
  o.tempStorageLowC        = (int8_t)buf[20];
  o.tempStorageHighC       = (int8_t)buf[21];

  o.autoDischarge          = buf[22] != 0;
  o.numCells               = buf[23];
  return true;
}

bool BattGoBattery::decodeCycleInfo(const uint8_t* buf, size_t len, BattGoCycleInfo& o) {
  // Go: len >= 12, reply 0x4B
  if (len < 12 || buf[0] != 0x4B) return false;

  o.chargeCycles           = u16le(buf + 1);
  o.errorOverTemperature   = u16le(buf + 6);
  o.errorOverCharged       = u16le(buf + 8);
  o.errorOverDischarged    = u16le(buf + 10);
  return true;
}

bool BattGoBattery::decodeState(const uint8_t* buf, size_t len, BattGoState& o) {
  // Go: len >= 6, reply 0x45, buf[1] == 0
  if (len < 6 || buf[0] != 0x45 || buf[1] != 0) return false;

  const uint8_t numCells = (uint8_t)(buf[2] + 1);
  if (numCells < 1 || numCells > 16) return false;

  const size_t need = 3 + 1 + (2 * (size_t)numCells); // matches Go check
  if (len < need) return false;

  o.numCells = numCells;

  size_t idx = 3;
  for (uint8_t i = 0; i < numCells; i++) {
    const uint16_t mv = u16le(buf + idx);
    o.cellVoltageV[i] = mv / 1000.0f;
    idx += 2;
  }

  o.temperatureC = (int8_t)buf[idx];
  return true;
}
