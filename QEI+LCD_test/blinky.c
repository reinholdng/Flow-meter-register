#include "display.h"
#include "driverlib/rom_map.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/ssi.h"
#include "driverlib/rom.h"
#include "driverlib/qei.h"

unsigned long timer;
unsigned long count;
float time = 0.1;
float freq;
float qeiVelocidad;
float velocidad;
float caudal_instantaneo;
char str[6];
int cauda;

void ConfigureQEI(void){

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI1);
    ROM_GPIOPinConfigure(GPIO_PC5_PHA1);
    ROM_GPIOPinTypeQEI(GPIO_PORTC_BASE, GPIO_PIN_5);
    ROM_GPIOPinConfigure(GPIO_PC6_PHB1);
    ROM_GPIOPinTypeQEI(GPIO_PORTC_BASE, GPIO_PIN_6);
    ROM_QEIDisable(QEI1_BASE);
    ROM_QEIIntDisable(QEI1_BASE,QEI_INTERROR | QEI_INTDIR | QEI_INTTIMER | QEI_INTINDEX);
    ROM_QEIConfigure(QEI1_BASE, (QEI_CONFIG_CAPTURE_A_B  | QEI_CONFIG_NO_RESET  | QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), 2399);
    ROM_QEIVelocityConfigure(QEI1_BASE,QEI_VELDIV_1,((ROM_SysCtlClockGet()/2)-1));
    ROM_QEIEnable(QEI1_BASE);
    ROM_QEIVelocityEnable(QEI1_BASE);
    ROM_QEIPositionSet(QEI1_BASE, 1199);
}

int main()
{
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_8|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    initLCD();
    ROM_IntMasterEnable();
    ConfigureQEI();
    while(1) {
        //printLCD("Flujo:");
        qeiVelocidad = (float)ROM_QEIVelocityGet(QEI1_BASE);
        velocidad = (((qeiVelocidad*2)/2400)*60);
        caudal_instantaneo = velocidad * 2.35;
        //cauda = (int)caudal_instantaneo;
        //sprintf(str, "%f", caudal_instantaneo);
        //setCursorPositionLCD(1,0);
        //printLCD(&str);
       // SysCtlDelay(2000);
        //clearLCD();

    }
}
