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
float velocidad; //Encoder velocity computed in RPM
float caudal_instantaneo; //Flow rate computed value
char str[6]; //Char to print flow rate value to LCD
char str2[6]; //Char to print volume to LCD
uint8_t str3[10];
uint8_t str1[10];
int cursorPosition; // Cursor position in LCD for flow rate and volume values.
int cauda; //Flow rate value as integer
uint32_t cauda2;
uint32_t cauda1;
uint8_t acumLen;
volatile uint32_t QEI_IntSource = 0; // Source of interrupt for QEI Module
volatile uint32_t QEI_DirFlag = 0; //Flag used to represents direction of encoder
volatile uint32_t QEI_TimerFlag = 0; //Timer expiration flag for Velocity capture timer
volatile uint32_t QEI_IndexFlag = 0; // Flag used to detect index pulse
int IndexCounter = 0; // Revolution Counter
volatile uint32_t DirFlag = 0; //Another direction flag
volatile uint32_t averageCounter = 0; // Count up to 4 measurements, for further averaging
//float averageValues[4]; // Array used to save flow values to average.
uint32_t qeiAvgVel = 0;
long double totalVolume = 9876543.21f;
float volumecopy = 9876543.21f;
uint32_t intVolume = 0;
double volume = 0.00f;
double tmpvol;
uint32_t globalVal;

void ConfigureQEI(void){

    //Enable peripheral clock for GPIO Port D used by QEI0-PhA and QEI0-PhB.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
    {
    }
    //Enable peripheral clock for GPIO Port F used by QEI0-Index
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }
    //Enable peripheral clock for QEI0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_QEI0))
    {
    }
    //Configure PD6 as PHA0
    GPIOPinConfigure(GPIO_PD6_PHA0);
    GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_6);
    //Unlock and configure PD7 as PHB0
    HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTD_BASE + GPIO_O_CR) |= 0x80;
    GPIOPinConfigure(GPIO_PD7_PHB0);
    GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_7);
    //Configure PF4 as IDX0
    GPIOPinConfigure(GPIO_PF4_IDX0);
    GPIOPinTypeQEI(GPIO_PORTF_BASE, GPIO_PIN_4);
    //Disable QEI module and all QEI interrupts temporarily for configuration
    QEIDisable(QEI0_BASE);
    QEIIntDisable(QEI0_BASE,QEI_INTERROR | QEI_INTDIR | QEI_INTTIMER | QEI_INTINDEX);
    //Enable and configure digital input noise filter for QEI0
    //FILTCNT_2 means Input sampled after 2 system clocks
    QEIFilterEnable(QEI0_BASE);
    QEIFilterConfigure(QEI0_BASE, QEI_FILTCNT_2);
    //Configure QEI general parameters
    //Capture both phases (600 ppr * 4 = 2400 pulses) ergo ui32MaxPosition = 2399
    //Quadrature mode used, No signals swapping before processing
    QEIConfigure(QEI0_BASE, (QEI_CONFIG_CAPTURE_A_B  | QEI_CONFIG_RESET_IDX  | QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), 2399);
    //Configure the velocity capture
    //No pre-division of input (Veldiv = 1)
    //Capture last quarter of second (quarter of clock rate -1, count is zero based)
    QEIVelocityConfigure(QEI0_BASE, QEI_VELDIV_1, ((SysCtlClockGet()/4)-1));
    //Enable QEI0 module
    QEIEnable(QEI0_BASE);
    //Masking and enabling interrupts for direction change, velocity timer expiration and index detection
    QEIIntEnable(QEI0_BASE, (QEI_INTDIR | QEI_INTINDEX | QEI_INTTIMER));
    //Interrupt Enable for QEI0
    IntEnable(INT_QEI0);
    //Velocity capture enabled for QEI0
    QEIVelocityEnable(QEI0_BASE);
    //Setting initial position in the middle of the encoder range
    //QEIPositionSet(QEI0_BASE, 1199);
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
    //Trigger flag for index interrupt
    if (QEI_IntSource == QEI_INTINDEX){
        QEI_IndexFlag = 1;
    }
}

void sendStr (int fila, uint32_t val)
{
    globalVal = val;
    if(val == 0){
        if(fila == 0){
            setCursorPositionLCD(fila,15);
            sendByte('0', TRUE);
        }else{
            setCursorPositionLCD(fila,12);
            sendByte('0', TRUE);
            sendByte('.', TRUE);
            sendByte('0', TRUE);
            sendByte('0', TRUE);
        }
        return;
    }

    uint32_t v = val;
    uint32_t vOut[10]; // 10 digits max for flowrate
    uint32_t strCnt = 0;
    int i;


    while (v > 0){
        vOut[strCnt++] = (uint32_t)v%10;
        v /= 10;
        if(fila == 1){
            if (val < 100){
                if(strCnt == 1 && val < 10){
                    vOut[strCnt++] = '0'-48;
                    vOut[strCnt++] = '.'-48;
                    vOut[strCnt++] = '0'-48;

                }
                if(strCnt == 2 && val >= 10){
                    vOut[strCnt++] = '.'-48;
                    vOut[strCnt++] = '0'-48;
                }

            }else{
                if(strCnt == 2){
                    vOut[strCnt++] = '.'-48;
                }
            }
        }








    }






    cursorPosition = 16-(int)strCnt;
    setCursorPositionLCD(fila, cursorPosition);

    for(i = --strCnt; i >= 0; i--){
            sendByte(vOut[i]+48, TRUE);
    }

}

void getDecStr (uint8_t* str, uint8_t len, uint32_t val)
{
  uint8_t i;

  for(i=1; i<=len; i++)
  {
    str[len-i] = (uint8_t) ((val % 10UL) + '0');
    val/=10;
  }

  str[i-1] = '\0';
}

int main()
{
    //FPU Enabled in LazyStacking mode
    FPUEnable();
    FPULazyStackingEnable();
    //General clock configuration (Currently: 25 MHz)
    SysCtlClockSet(SYSCTL_SYSDIV_8|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    IntMasterEnable();//General enable of interrupts
    ConfigureQEI();
    //LCD Initialization and static text printing
    initLCD();
    setCursorPositionLCD(0,0);
    printLCD("Flujo:");
    setCursorPositionLCD(1,0);
    printLCD("Acum.:");

    while(1) {

        if (QEI_DirFlag == 1){
            QEI_DirFlag = 0;
            DirFlag ^= 1;
        }
        if (QEI_IndexFlag == 1){
            QEI_IndexFlag = 0;

            if (QEIDirectionGet(QEI0_BASE) == 1){
                IndexCounter+=1;
                volume += 0.009463525f;
            }else{
                IndexCounter--;
            }
        }
        if(QEI_TimerFlag == 1){
            QEI_TimerFlag = 0;
            if(averageCounter < 4){
                //averageValues[averageCounter] = (float)QEIVelocityGet(QEI0_BASE);
                qeiAvgVel += QEIVelocityGet(QEI0_BASE);
                averageCounter++;
            }else{
                averageCounter = 0;
                //qeiAvgVel = (averageValues[0]+averageValues[1]+averageValues[2]+averageValues[3])/4;
                //Updated: No need to divide values by 4 to average, since velocity captures occurs every 1/4 of second
                //and the value returned by QEIVelocityGet (qeiAvgVel) is the number of pulses detected in this period;
                //so to obtain the number of revolutions per second, this value should be multiplied by the number of
                //time periods per second (4) and then divided by the number of pulses per revolution.
                //The result is then multiplied by 60 to obtain the measurement in RPMs
                //Long equation -> velocidad = (((((float)qeiAvgVel/4)*4)/2400)*60)
                //Simplified equation -> velocidad = ((float)qeiAvgVel*0.025);
                velocidad = ((float)qeiAvgVel*0.025);
                qeiAvgVel = 0;
                //Flow rate computation using RPM to Flow ratio
                // This equation should be changed with the relationship found with linear regression analysis
                // of RPMs and Flow rate from reference meter.
                caudal_instantaneo = velocidad * 2.3529;
                //converts flow rate value to integer for displaying purposes
                cauda1 = (uint32_t)caudal_instantaneo;
                //Set cursor position for actual flow value, clear previous value, and print actual
                setCursorPositionLCD(0,6);
                printLCD("             ");

                //getDecStr(str1, 10, cauda1);


                //getDecStr2(str1, cauda1);
                //cursorPosition = 16-strlen(str1);
                //setCursorPositionLCD(0, 6);
                sendStr(0, cauda1);
                //printLCD(&str1);



                //Set cursor position for actual flow value, clear previous value, and print actual
                setCursorPositionLCD(1,6);
                printLCD("             ");
                cauda2 = (uint32_t)IndexCounter;

                //acumLen = sizeof(cauda2)/sizeof(uint8_t);
                getDecStr(str3, 10, cauda2);
                //cursorPosition = 16-strlen(str3);
                setCursorPositionLCD(1, 6);


                // use pun.to
                tmpvol = volume*100.00f;
                float tmpvolcopy = volumecopy*100.00f;
                uint32_t intvolcopy = (uint32_t)tmpvolcopy;
                intVolume = (uint32_t)(tmpvol);
                sendStr(1, intVolume);

                //printLCD(&str3);

            }
        }

        //Gets encoder position ranging from 0 to 2399
        //reloj = QEIPositionGet(QEI0_BASE);
        //Converts the # of edges/pulses detected in a specified time period to velocity
        // *4 -> 4 Time periods (0.25 s) in a second
        // /2400 -> Number of edges/pulses per revolution
        // *60 -> Converts RPS to RPM
        //velocidad = (((qeiVelocidad*2)/2400)*60);

    }
}