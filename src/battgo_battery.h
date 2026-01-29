#pragma once
#include <Arduino.h>
#include "battgo_bus.h"

// Battery type IDs (from go-battgo)
enum BattGoBatteryType : uint8_t {
  BattGoLiHv  = 0,
  BattGoLiPo  = 1,
  BattGoLiIon = 2,
  BattGoLiFe  = 3,
  BattGoPb    = 5,
  BattGoNiMH  = 6
};

struct BattGoFactoryInfo {
  BattGoBatteryType type;
  float cellDischargeCutoffV;
  float cellDischargeNormalV;
  float cellChargeMaxV;
  float cellStorageDefaultV;
  float cellCapacityAh;
  float chargeMaxCurrentA;
  float dischargeMaxCurrentA;
  int8_t tempUseLowC;
  int8_t tempUseHighC;
  int8_t tempStorageLowC;
  int8_t tempStorageHighC;
  bool autoDischarge;
  uint8_t numCells;
};

struct BattGoCycleInfo {
  uint16_t chargeCycles;
  uint16_t errorOverTemperature;
  uint16_t errorOverCharged;
  uint16_t errorOverDischarged;
};

struct BattGoState {
  // number of cells is derived from reply; voltages length equals numCells
  uint8_t numCells = 0;
  float cellVoltageV[16]; // up to 16 cells supported; adjust if needed
  int8_t temperatureC = 0;
};

// High-level helpers for common battery queries.
class BattGoBattery {
public:
  explicit BattGoBattery(BattGoBus& bus) : _bus(bus) {}

  bool readFactoryInfo(BattGoFactoryInfo& out, uint16_t timeoutMs = 150);
  bool readCycleInfo(BattGoCycleInfo& out, uint16_t timeoutMs = 150);

  // Read state (cell voltages + temperature).
  // If you know the cell count, pass it in to request exactly that many (1..16).
  // If unknown, pass 0; we'll request 8 cells (common default) and still decode based on reply.
  bool readState(BattGoState& out, uint8_t numCellsHint = 0, uint16_t timeoutMs = 150);

  // Decoders (exposed for offline parsing/testing)
  static bool decodeFactoryInfo(const uint8_t* buf, size_t len, BattGoFactoryInfo& out);
  static bool decodeCycleInfo(const uint8_t* buf, size_t len, BattGoCycleInfo& out);
  static bool decodeState(const uint8_t* buf, size_t len, BattGoState& out);

private:
  BattGoBus& _bus;

  static uint16_t u16le(const uint8_t* b) { return (uint16_t)b[0] | ((uint16_t)b[1] << 8); }
  static uint32_t u32le(const uint8_t* b) {
    return (uint32_t)b[0] |
           ((uint32_t)b[1] << 8) |
           ((uint32_t)b[2] << 16) |
           ((uint32_t)b[3] << 24);
  }
};
