# Flow-meter-indicator/recorder

Prototype for digitizing mechanical indicators and recorders used in propeller flowmeters.


ToDo List:

- [x] LCD Driver
- [x] Instant flow measurment using average
- [x] Interrupts for encoder direction change and timer expiration
- [x] Use of PIOSC as low power clock source
- [x] Use of control outputs for Enable/disable LCD and Encoder (This feature will be removed due to hardware changes)
- [ ] Implement the EEPROM module to save Total flow (Volume)
- [ ] Algorithm to properly save to EEPROM without wearing it out
- [ ] Implement total flow (volume) measurment using two sensors (Reed/hall effect)
