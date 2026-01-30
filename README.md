# ArduinoBattGoSmartBattery

Arduino library for communicating with BattGO-enabled batteries/devices (ISDT BattGO), most commonly found as part of Spektrum SMART Battery G2.

This is a port from the [go-battgo](https://github.com/BertoldVdb/go-battgo) project by Bertold Van den Bergh. You may also find the
[python-battgo-smartbattery](https://github.com/colinoflynn/python-battgo-smartbattery) implementation for sniffing/debugging useful, which is
also a port from the go project.

This library implements:
- **PHY layer**: framing/byte-stuffing, checksum, scrambler (ported from `go-battgo`).
- **Bus layer**: synchronous command + reply matching
- **Battery decoders**: FactoryInfo (`0x88/0x89`), CycleInfo (`0x4A/0x4B`), State (`0x44/0x45`).

## Wiring / inverted UART

The battery requires a single wire output.

If you use a transistor stage that inverts the logic, enable UART inversion:
- AVR: `SoftwareSerial(rxPin, txPin, true /* inverse */)`
- ESP32: `Serial2.begin(9600, SERIAL_8N1, rxPin, txPin, true /* invert */)`

More documentation coming - not yet ready for primetime.

## Example

See `examples/ReadBatteryInfo/ReadBatteryInfo.ino`.


## Disclaimers

This is provided AS-IS - you may damage your batteries or charger, this is NOT an official protocol or project and based on other open-source projects. See [LICENSE](LICENSE). You are responsible for safe usage and charging of your batteries. This is NOT associated with any manufacturer or official battery provider. Horizon Hobby, IC3, and EC3 are either claimed or registered trademarks of Horizon Hobby, LCC. Spektrum is a trademark of Bachmann Industries, Inc. This project is not associated with Horizon Hobby or Bachmann Industries, Inc. Any trademarks referenced belong to their associated owners, and those trademarks are used to identify the products, not to imply any affiliation, endorsement, or sponsorship of the trademark owners.