#include <SoftwareSerial.h>
#include <BattGoArduino.h>

// Adjust pins for your board:
static const uint8_t RX_PIN = 8;
static const uint8_t TX_PIN = 9;

// Enable inverted logic if you have a transistor driver instead of simple diode driver
SoftwareSerial battSerial(RX_PIN, TX_PIN, false); // RX, TX, inverted

BattGoPHY phy(battSerial);
BattGoBus bus(phy, 0xE2);        // set your battery address here
BattGoBattery battery(bus);

BattGoFactoryInfo fi;
BattGoCycleInfo ci;
BattGoState st;

void setup() {
  Serial.begin(9600);
  battSerial.begin(9600);
  delay(200);

  uint8_t cmd[128] = {0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t buf[128];
  size_t len = sizeof(buf);
  size_t rsplen;

  while(1){
    //During discovery device uses default address
    bus.setDeviceAddr(0x00);
    if (bus.commandN(cmd, 12, 0x03, buf, len, 150)) {
      rsplen = len;
      while(len != 1){
        buf[len] = buf[len-1];
        len--;
      }

      buf[0] = 0x02;
      buf[1] = 0xE2;

      len = sizeof(buf);
      Serial.println("Trying address");
      bus.setDeviceAddr(0xE2);
      if (bus.commandN(buf, rsplen+1, 0x03, buf, len, 150, 0)){
        Serial.println("Battery address accepted!");
        break;
      }
    } else {
      Serial.println("Waiting...");
    }
  }

  if (battery.readFactoryInfo(fi)) {
    Serial.println("FactoryInfo:");
    Serial.print("  Cells: "); Serial.println(fi.numCells);
    Serial.print("  Capacity (Ah): "); Serial.println(fi.cellCapacityAh, 3);
    Serial.print("  Vmax/cell: "); Serial.println(fi.cellChargeMaxV, 3);
    Serial.print("  Vstorage/cell: "); Serial.println(fi.cellStorageDefaultV, 3);
    Serial.print("  Icharge max (A): "); Serial.println(fi.chargeMaxCurrentA, 2);
    Serial.print("  Idischarge max (A): "); Serial.println(fi.dischargeMaxCurrentA, 2);
  } else {
    Serial.println("FactoryInfo: failed/timeout");
  }


  if (battery.readCycleInfo(ci)) {
    Serial.println("CycleInfo:");
    Serial.print("  Charge cycles: "); Serial.println(ci.chargeCycles);
    Serial.print("  Err overtemp: "); Serial.println(ci.errorOverTemperature);
    Serial.print("  Err overcharged: "); Serial.println(ci.errorOverCharged);
    Serial.print("  Err overdischarged: "); Serial.println(ci.errorOverDischarged);
  } else {
    Serial.println("CycleInfo: failed/timeout");
  }

}

void loop() {
  // Pass fi.numCells if known; otherwise omit.
  if (battery.readState(st, fi.numCells)) {
    Serial.println("State:");
    Serial.print("  Temp (C): "); Serial.println(st.temperatureC);
    Serial.print("  Cells: "); Serial.println(st.numCells);
    for (uint8_t i = 0; i < st.numCells; i++) {
      Serial.print("   V"); Serial.print(i+1); Serial.print(": ");
      Serial.println(st.cellVoltageV[i], 3);
    }
  } else {
    Serial.println("State: failed/timeout");
  }

  delay(500);
}
