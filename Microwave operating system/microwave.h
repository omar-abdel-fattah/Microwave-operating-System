#include <stdint.h>
#include <stdbool.h>
#include "hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "tm4c123gh6pm.h"
#include "driverlib/uart.h" 
#include "driverlib/pin_map.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

void print(char*);                                                              //print a char array
void UART0int();                                                                //UART initialization
void GPIOAint();                                                                //GPIO A initialization
void GPIOFint();                                                                //GPIO F initialization
void SysTick_Init();                                                            //initialize Systick timer
void SysTick_Handler();                                                         //systic interrupt handler
void RX_INT_Handler();                                                          //the handler function for the UART reciever
void inctimer();
void timer_0A_init();                                                           //initialize timer0A
void timer0_handler(void);                                                      //GPTM interupt handler
void Buttonrise(void);                                                          //the handler function for the GPIO interuppt of unpressing the switch
void Buttonfall(void);                                                          //the handler function for the GPIO interuppt of pressing the switch
void mode();                                                                    //the start page for the microwave
                //putty was used as serial commication of PC