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
- [ ] Brown-out detection to improve reliability
- [x] Watchdog module configuration to improve reliability
- [x] LM2596 step-down(buck) converter desing to supply power from Battery/Alternator (Irrigation water pump) to the system.
- [x] Mechanical coupling between Encoder and Propeller mechanism.
- [x] Diode ORing circuit for power backup (SuperCapacitor).
- [ ] Timer modules configuration for timing managment (EEPROM Storage and others)
  
### Flow Rate:
  
- [x] Averaged encoder RPMs measurement
- [x] Detection of the direction of rotation to avoid "Negative flow" measurement
- [ ] Regression between RPMs and Flow Rate from a reference flow meter (Calibration)
- [ ] Definition of upper and lower flow rate limits to avoid inaccurate measurements.

### Total volume:
  
- [x] Measuring the number of encoder turns
- [x] Detection of the direction of rotation to avoid "Negative volume" measurement
- [ ] Regression between number of turns and Total volume from a reference flow meter (Calibration)
- [ ] Implementation of the EEPROM module for "Total volume" storage with wear leveling algorithm
- [ ] Interrupt handling for EEPROM storage in Brown-out conditions (Operating with SuperCapacitor)

  
### Low Power (Further Design):
  
- [ ] Use of PIOSC as low power clock source 
- [ ] Use of control pins for Enabling/disabling LCD and Encoder power input
- [ ] Hibernation state in absence of flow
- [ ] RTC Configuration for timing managment (EEPROM Storage, Trigger hibernation and restarts, etc)
