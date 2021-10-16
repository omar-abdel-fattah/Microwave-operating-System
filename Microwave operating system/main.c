#include "microwave.h"

int main()
{
UART0int();                             //uart initialization
GPIOAint();                             //GPIO A initialization
GPIOFint();                             //GPIO F initialization
SysTick_Init();                         //systick timer initialization
timer_0A_init();                        //timer 0A initialization
print("\033\143");                      //clear the screen from the last use 
mode();                                 //open the start page of the microwave
while(1)
{
   __asm("      wfi\n");                //assembly instruction to wait for interuppt
}
  return 0;
}
