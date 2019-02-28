# Flow-meter-indicator/recorder

Prototype for digitizing mechanical indicators and recorders used in propeller flowmeters.


ToDo List:

  General:
  
- [x] LCD 16x2 (HD44780) Driver
- [x] Interrupts configuration for QEI events (direction change, index detection and timer expiration)

  
  Flow Rate:
  
- [x] Averaged encoder RPMs measurement

  
  Total volume:
  
- [ ] Implementation of the EEPROM module for "Total volume" storage

  
  Low Power Design:
  
- [x] Use of PIOSC as low power clock source (Temporarily removed)
- [x] Use of control pins for Enabling/disabling LCD and Encoder power input (Temporarily removed)
- [ ]
- [ ] Algorithm to properly save to EEPROM without wearing it out
- [ ] Implement total flow (volume) measurment using two sensors (Reed/hall effect)
