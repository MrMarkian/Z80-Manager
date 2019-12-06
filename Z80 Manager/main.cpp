
#include "stdbool.h"
#include "stdint.h"
#include "string.h"
#include "tm4c123gh6pm.h"

#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"


#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif
//#define uint32_t unsigned int;

#define PA2   (*((volatile unsigned long *)0x40004010)) // data [Shift Register]
#define PA3   (*((volatile unsigned long *)0x40004020)) //CLOCK [Shift Register]
#define PA4   (*((volatile unsigned long *)0x40004040)) // latch [Shift Register]

#define PA6   (*((volatile unsigned long *)0x40004100)) // CLOCK LINE PA6 [Z80 CPU Control]

bool SRData, SRClock, SRLatch;

void Delay();
void PortA_Init();
void PortF_Init();
void PortD_Init();

void ToggleClock();
void SingleStep();

void PrintString (char *string);
char CheckChar ();

void CheckUI();
void Firstboot();

void putByte(uint8_t data, volatile unsigned int msbfirst);

unsigned long IN, IN2;
unsigned long OUT;
bool CLOCKON;
int CLOCKSPEED =0;

class TermController
{
private:
    int xPos;
    int yPos;

public:

    void SetColor(int color){
        switch (color)
        {
        case 34://blue
            PrintString("\x1b[34m");
            break;
        case 30://black
            PrintString("\x1b[30m");
            break;
        case 31://red
            PrintString("\x1b[31m");
            break;
        case 32://green
            PrintString("\x1b[32m");
            break;
        case 35://magenta
            PrintString("\x1b[35m");
            break;
        case 36://cyan
            PrintString("\x1b[36m");
            break;
        case 37://white
            PrintString("\x1b[37m");
            break;
        case 33://yellow
            PrintString("\x1b[33m");
            break;

        }

    }

    void SendNewline(){PrintString("\n\r");}

    void Home(){PrintString("\x1b[[H");}

    void ClearScreen(){PrintString("\x1b[J");}
    void ClearLine() {PrintString("\x1b[[2J");}

    void CursorUp()     {PrintString("\x1b[[A");}
    void CursorDown()   {PrintString("\x1b[[B");}
    void CursorRight()  {PrintString("\x1b[[C");}
    void CursorLeft()   {PrintString("\x1b[[D");}


};

 int main() {

    TermController tm;

       SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);
    SysCtlDelay(10);

    // Set the clocking to run directly from the crystal.


    PortA_Init();
    PortF_Init();
//  PortD_Init();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0); SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX); GPIOPinConfigure(GPIO_PA1_U0TX); GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    tm.ClearScreen();

    PrintString("Welcome to Z80 P/M");
    tm.SendNewline();
    tm.SetColor(33);
    PrintString("Please Pick an option..");

    for (uint8_t a = 0; a < 256; a++)
    {
    putByte(a,1);
    }
    tm.SendNewline();

    //while (1) {
    //if (UARTCharsAvail(UART0_BASE)) UARTCharPut(UART0_BASE, UARTCharGet(UART0_BASE)); }


    for(;;)
    {

        CheckUI();

        IN = GPIO_PORTF_DATA_R&0x10; //READ PF4 into Sw1
            IN = IN >>4;

                if (IN){
                        OUT = GPIO_PORTF_DATA_R;
                        OUT = OUT &0xFB;
                        OUT = OUT|IN;
                        GPIO_PORTF_DATA_R = OUT;
                        ToggleClock();

                }
                else
                    {
                        OUT = 0;
                        GPIO_PORTF_DATA_R = OUT;
                        SingleStep();
                    }

    }

    return 0;
}


 void putByte(uint8_t data, volatile unsigned int msbfirst) {

    uint8_t  i;
    char temp, temp2;



   if (msbfirst == 1)
   {
      for (i = 0; i < 8; i++) {
          temp = (0x01 & data);
          data = data >> 1;
          temp2 = (temp2 << 1) + temp;
        }
        data = temp2;
   }

   for (i = 0; i < 8; i++) {         // 8 bits to a char
      char bittowrite = (0x01 & data); // get bit
      data = data >> 1; // shift data left
      if (bittowrite == 1) {  // send bit
      //  DATAPIN;
         PA2 = 255;
      } else {
        //DATAPIN;
         PA2 = 0;
      }
      // pulse clockpin
      PA3 = 0;
      SysCtlDelay(1000000);
      PA3 = 255;
      SysCtlDelay(1000000);
      PA3 = 0;
    }
   PA4 = 255; //pull up latch pin
   SysCtlDelay(1000000);
   PA4 = 0; // pull down latch pin

    SysCtlDelay(100000);

 }

void Firstboot()
{

}
void CheckUI()
{

TermController tm;

    char TempChar;
    if (UARTCharsAvail(UART0_BASE))
            {
                GPIO_PORTF_DATA_R = (1<<3);
                TempChar = UARTCharGet(UART0_BASE);
                switch (TempChar)
                {
                    case 'z':
                        PrintString("You Pressed ZZZZ");
                        tm.SendNewline();
                        break;
                    case 's':
                        PrintString("SingleStep Mode..");
                        tm.SendNewline();
                        SingleStep();
                        break;
                    case '1':
                        CLOCKSPEED=0;
                        PrintString("Speed 1.");
                        tm.SendNewline();
                        break;
                    case '2':
                        CLOCKSPEED=1;
                        PrintString("Speed 2");
                        tm.SendNewline();
                        break;
                    case '3':
                        CLOCKSPEED=2;
                        PrintString("Speed 3.");
                        tm.SendNewline();
                        break;
                    case '4':
                        CLOCKSPEED=3;
                        PrintString("Speed 4");
                        tm.SendNewline();
                        break;
                    case '5':
                        CLOCKSPEED=4;
                        PrintString("Speed 5.");
                        tm.SendNewline();
                        break;
                    case '6':
                        CLOCKSPEED=5;
                        PrintString("Speed 6.");
                        tm.SendNewline();
                        break;
                    case 't':
                        if (CLOCKON)
                            {
                                tm.SetColor(31);
                                PrintString("Clock On..");
                                tm.SendNewline();
                            }
                        else
                            {
                                tm.SetColor(32);
                                PrintString("Clock Off..");
                                tm.SendNewline();
                            }

                            ToggleClock();
                            break;
                    default:
                        PrintString("Unknown Option");
                        tm.SendNewline();
                        main();
                        break;
                }
            }
}

void SingleStep()
{
    SysCtlDelay(200);
    //SysCtlDelay(200000);
    IN = GPIO_PORTF_DATA_R&0x10; //READ PF4 into Sw1
    IN = IN >>4;
    SysCtlDelay(200);
    while (IN)
        {
            SysCtlDelay(200);
            IN = GPIO_PORTF_DATA_R&0x10; //READ PF4 into Sw1
            IN = IN >>4;
            SysCtlDelay(200);
            IN2 = GPIO_PORTF_DATA_R&0x01; //READ PF4 into Sw1
            CheckUI();
            if(!IN2)
                return ;

        }
        ToggleClock();
        SingleStep();

}

void ToggleClock()
{
    if(CLOCKON)
        {
            PA6 = 0;
            CLOCKON = false;
            GPIO_PORTF_DATA_R = (1<<1);
        }
    else
    {
        PA6 = 255;
        CLOCKON = true;
        GPIO_PORTF_DATA_R = (1<<2);
    }

    switch (CLOCKSPEED)
    {
    case 0:
        SysCtlDelay(2000000);
        break;
    case 1:
        SysCtlDelay(500000);

        break;
    case 2:
        SysCtlDelay(90000);
        break;
    case 3:
        SysCtlDelay(30000);
        break;
    case 4:
        SysCtlDelay(1000);
    case 5:
        SysCtlDelay(100);
    }


}

void PrintString (char *string)
{
    while (*string)
    {
        UARTCharPut(UART0_BASE, *string++);
    }
}

char CheckChar ()
{
    char Character;

    if (UARTCharsAvail(UART0_BASE))
    {
        Character = UARTCharGet(UART0_BASE);

    }
    return Character;
}

void Delay()
{

         unsigned long volatile time;
              time = 987654*9/25;  // 0.1sec
              while(time){
                    time--;
              }



}

void PortA_Init(void)
{
    volatile unsigned long delay;
  //  SYSCTL_RCGC2_R |= 0x00000001;
//  delay = SYSCTL_RCGC2_R;           // delay
        SYSCTL_RCGC2_R = (1 << 0);
      GPIO_PORTA_CR_R = 0x1F;
      GPIO_PORTA_AMSEL_R = 0x00000000;
      GPIO_PORTA_PCTL_R = 0x00000000;
      GPIO_PORTA_DIR_R = 0xFF;
      GPIO_PORTA_AFSEL_R =0x00000000;
      GPIO_PORTA_DEN_R = 0xFF;

}

void PortF_Init(void)
{
    volatile unsigned long delay;

    //  ENABLE THE L.E.D'S AND TIVA BUTTONS.

        SYSCTL_RCGC2_R = (1 << 5);
        delay = SYSCTL_RCGC2_R;

        GPIO_PORTF_LOCK_R = 0x4C4F434B;
        GPIO_PORTF_CR_R = 0X1F;
        GPIO_PORTF_AMSEL_R = 0X00;
        GPIO_PORTF_PCTL_R = 0X00000000;
        GPIO_PORTF_DIR_R = 0X0E;
        GPIO_PORTF_AFSEL_R = 0X00;
        GPIO_PORTF_PUR_R = 0X11;
        GPIO_PORTF_DEN_R = (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4); //DIGITAL ENABLE for 3 LEDs(1,2,3) and 2 Buttons (0,4)

}

void PortD_Init(void)
{
    /*PORT D IS BEING USED FOR BI-DIRECTIONAL COMMUNICATION
     * WITH THE Z80 DATA BUS.
     */
    volatile unsigned long delay;
        SYSCTL_RCGC2_R = (1 << 3);
        delay = SYSCTL_RCGC2_R;
        GPIO_PORTD_LOCK_R = 0x4C4F434B;
        GPIO_PORTD_AMSEL_R = 0X00;
        GPIO_PORTD_PCTL_R = 0X00000000;
        GPIO_PORTD_DIR_R = 0X0E;
        GPIO_PORTD_AFSEL_R = 0X00;
        GPIO_PORTD_PUR_R = 0X00;
        GPIO_PORTD_DEN_R = (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7); //DIGITAL ENABLE FOR ALL PINS IN PORT D
}
