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
#include "inc/hw_sysctl.h"


unsigned long timer;
unsigned long count;
float time = 0.1;
//float freq;
float qeiVelocidad; //# of edges/pulses detected in the specified time period
float velocidad; //Encoder velocity computed in RPM
float caudal_instantaneo; //Flow rate computed value
char str[6]; //Char to print flow rate value to LCD
int cursorPosition; // Cursor position in LCD for flow rate and volume values.
int cauda; //Flow rate value as integer
volatile uint32_t reloj; //Test
volatile uint32_t QEI_IntSource = 0; // Source of interrupt for QEI Module
volatile uint32_t QEI_DirFlag = 0; //Flag used to represents direction of encoder
volatile uint32_t QEI_TimerFlag = 0; //Timer expiration flag for Velocity capture timer
volatile uint32_t DirFlag = 0; //Another direction flag
volatile uint32_t averageCounter = 0; // Count up to 4 measurements, for further averaging
float averageValues[4]; // Array with
float qeiAvgVel = 0.0f;
volatile uint32_t showFlag = 0;

void ConfigureGPIOC(void){

      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
      GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, (GPIO_PIN_5 | GPIO_PIN_4));//Debug
      //Enable Encoder
      GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0xFF);
      //Enable LCD
      GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0xFF);

}

void ConfigureQEI(void){

    //Enable peripheral clock for GPIO Port D used by QEI0-PhA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    //Enable peripheral clock for GPIO Port F used by QEI0-PhB and QEI0-Index
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    //Enable peripheral clock for QEI0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);
    //Configure PD6 as PHA0 (QEI0)
    GPIOPinConfigure(GPIO_PD6_PHA0);
    GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_6);
    //Configure PD7 as PHB0 (QEI0)
    HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTD_BASE + GPIO_O_CR) |= 0x80;
    //GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_1);
    GPIOPinConfigure(GPIO_PD7_PHB0);
    GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_7);
    //Disable QEI module and all QEI interrupts temporarily for configuration
    QEIDisable(QEI0_BASE);
    QEIIntDisable(QEI0_BASE,QEI_INTERROR | QEI_INTDIR | QEI_INTTIMER | QEI_INTINDEX);
    //Enable and configure digital input noise filter for QEI0
    //FILTCNT_2 means Input sampled after 2 system clocks
    QEIFilterEnable(QEI0_BASE);
    QEIFilterConfigure(QEI0_BASE, QEI_FILTCNT_2);
    //Configure QEI general parameters
    //Capture both phases (2400 pulses) ergo ui32MaxPosition = 2399
    //Temporarily disabled reset on index due to lack of proper encoder
    //Quadrature mode used
    //No signals swapping before processing
    QEIConfigure(QEI0_BASE, (QEI_CONFIG_CAPTURE_A_B  | QEI_CONFIG_NO_RESET  | QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), 2399);
    //Configure the velocity capture
    //No pre-division of input (Veldiv = 1)
    //Capture last half second (Half clock rate -1, capture begin from zero)
    //SE CAMBIO LA DURACIÓN A 1/4 DE SEGUNDO PARA PROMEDIARLA
    QEIVelocityConfigure(QEI0_BASE, QEI_VELDIV_1, ((SysCtlClockGet()/4)-1));
    //Enable QEI0 module
    QEIEnable(QEI0_BASE);
    //Masking of interrupts and enabling interrupts for direction change, timer expiration and index detection
    //Uncomment: QEIIntEnable(QEI0_BASE, (QEI_INTDIR | QEI_INTINDEX | QEI_INTTIMER));
    QEIIntEnable(QEI0_BASE, (QEI_INTDIR | QEI_INTTIMER));
    //Interrupt Enable for QEI0
    IntEnable(INT_QEI0);
    //Velocity capture enabled for QEI0
    QEIVelocityEnable(QEI0_BASE);
    //Setting initial position in the middle of the encoder range
    QEIPositionSet(QEI0_BASE, 1199);
}

void manejadorInterrupcion_QEI(void) {

    //Read unmasked interrupts sources for QEI0 Module
    QEI_IntSource = QEIIntStatus(QEI0_BASE, true);
    //Clear the triggered interrupt sources
    QEIIntClear(QEI0_BASE, QEI_IntSource);
    //Wait to fully clear interrupts
    SysCtlDelay(40000);
    //Trigger flag for direction change
    if (QEI_IntSource == QEI_INTDIR){
        QEI_DirFlag = 1;
    }
    //Trigger flag for timer expiration
    if (QEI_IntSource == QEI_INTTIMER){
        QEI_TimerFlag = 1;
    }
    //Uncomment to trigger index interrupt
    /*if (QEI_IntSource == QEI_INTINDEX){
        QEI_IndexFlag = 1;
    }*/
}

int main()
{
    //SysCtlClockSet(SYSCTL_SYSDIV_8|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);


    /*
     * Clock configuration:
     * Divider: 1 (Won't change the oscillator frequency)
     * Oscillator to be used: PIOSC - Precision Internal Oscillator - running at 16MHz without PLL
     * Main Oscillator (MOSC) disables
    */
    //SysCtlClockSet(SYSCTL_SYSDIV_8|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_INT | SYSCTL_MAIN_OSC_DIS);

    //General enable of interrupts
    IntMasterEnable();

    ConfigureQEI();
    ConfigureGPIOC();

        //LCD initialization

   initLCD();

    //Print static characters
    //clearLCD();
    setCursorPositionLCD(0,0);
    printLCD("Flujo:");
    //SysCtlDelay(10000000);
    setCursorPositionLCD(1,0);
    printLCD("Acum.:");
    //SysCtlDelay(10000000);


    while(1) {

        //printLCD("Flujo:");
        //qeiVelocidad = (float)QEIVelocityGet(QEI0_BASE);


        if (QEI_DirFlag == 1){
            QEI_DirFlag = 0;
            DirFlag ^= 1;
        }
        if(QEI_TimerFlag == 1){

            QEI_TimerFlag = 0;
            if(averageCounter < 4){

                averageValues[averageCounter] = (float)QEIVelocityGet(QEI0_BASE);
                averageCounter++;

            }else{


                //qeiAvgVel = (averageValues[0]+averageValues[1]+averageValues[2]+averageValues[3])/4;
                averageCounter = 0;
                showFlag = 1;
                //averageValues[averageCounter] = (float)QEIVelocityGet(QEI0_BASE);
                //averageCounter++;
            }

        }

        //Gets encoder position ranging from 0 to 2399
        //reloj = QEIPositionGet(QEI0_BASE);
        //Converts the # of edges/pulses detected in a specified time period to velocity
        // *4 -> 4 Time periods (0.25 s) in a second
        // /2400 -> Number of edges/pulses per revolution
        // *60 -> Converts RPS to RPM
        //velocidad = (((qeiVelocidad*2)/2400)*60);
        if (showFlag == 1){
        qeiAvgVel = (averageValues[0]+averageValues[1]+averageValues[2]+averageValues[3])/4;
        velocidad = (((qeiAvgVel*4)/2400)*60);
        //Flow rate computation using RPM to Flow ratio (
        caudal_instantaneo = velocidad * 2.3529;
        //converts data to int
        cauda = (int)caudal_instantaneo;
        //cauda = (int)reloj;
        //Converts caudal value to char array


        setCursorPositionLCD(0,6);
        printLCD("             ");
        //SysCtlDelay(50);
        //SysCtlDelay(10000000);
        sprintf(str, "%d", cauda);
        cursorPosition = 16-strlen(str);
        setCursorPositionLCD(0, cursorPosition);
        printLCD(&str);
        //SysCtlDelay(10000);
        setCursorPositionLCD(1,0);
        if (QEIDirectionGet(QEI0_BASE) == 1){
            //printLCD("DERECHA");
        }else{
            //printLCD("IZQUIERDA");
        }
        showFlag = 0;
        //SysCtlDelay(100000000);
        //clearLCD();

    }
}
}
