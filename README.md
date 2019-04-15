# Flow-meter-indicator/recorder

Prototype for digitizing mechanical indicators and recorders used in propeller flowmeters.

## Main Parts:

* Tiva C (TM4C123GH6PM) Microcontroller evaluation board
* Omron E6B2-CWZ6C Rotary Encoder (Incremental)
* LM2596 Switching Regulator

## ToDo List:

### General:

- [x] LCD 16x2 (HD44780) customized driver
- [x] QEI configuration and interrupt handling (direction change, index detection and timer expiration)
- [x] Brown-out detection to improve reliability
- [x] Watchdog module configuration to improve reliability
- [x] LM2596 step-down(buck) converter design to supply power from Battery/Alternator (Irrigation water pump) to the system.
- [x] Mechanical coupling between Encoder and Propeller mechanism.
- [x] Diode ORing circuit for power backup (SuperCapacitor).
- [x] Timer modules configuration for timing management (EEPROM Storage and others)
- [ ] DHT11 Relative humidity sensor driver
- [ ] Interrupt driven alarm for humidity threshold
- [x] Analog comparator for power supply supervision

### Flow Rate:

- [x] Averaged encoder RPMs measurement
- [x] Detection of the direction of rotation to avoid "Negative flow" measurement
- [ ] Regression between RPMs and Flow Rate from a reference flow meter (Calibration)
- [x] Definition of upper and lower flow rate limits to avoid inaccurate measurements.
- [x] Type-casting algorithm to convert uint32_t to LCD-String type for display purposes

### Total volume:

- [x] Measuring the number of encoder turns
- [x] Detection of the direction of rotation to avoid "Negative volume" measurement
- [ ] Regression between number of turns and Total volume from a reference flow meter (Calibration)
- [x] Implementation of the EEPROM module for "Total volume" storage with wear leveling algorithm
- [x] Interrupt handling for EEPROM storage in Brown-out conditions (Operating with SuperCapacitor)
- [x] Type-casting algorithm to convert from float to LCD-String type for display purposes


### Low Power (Further Design):

- [x] Use of PIOSC as low power clock source
- [x] Use of control pins for Enabling/disabling LCD and Encoder power input
- [ ] Hibernation state in absence of flow
- [x] Full-width 64 bit timer configuration as RTC clock source
- [ ] RTC Configuration for timing managment (EEPROM Storage, Trigger hibernation and restarts, etc)

### Hardware Design

- [x] Supercapacitor charger circuit for redundancy in power supply
- [x] Reverse polarity protection circuit with TVS Diodes
- [x] Signal conditioning cirucit for open-collector outputs from encoder
- [x] Voltage divider circuit for power supply signal conditioning
- [x] Impedance coupling circuit to allow Microcontroller power supply monitoring from voltage divider circuit.
