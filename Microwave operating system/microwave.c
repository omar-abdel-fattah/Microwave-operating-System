#include "microwave.h"

char time[]={'0','0',':','0','0'}; //timer array
static bool i=0;

void print(char *s)                                                             //takes a string from keybard and print on the serial PC
{
  while(*s)                                                                             // pass by the character array 
  {
      UARTCharPut(UART0_BASE,*s);                                                       //print the character from the UART to the PC serial
      s++;                                                                              //increment the character array pointer
  }
}

void GPIOFint()                                                                         //initialize GPIO port F 
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);                                        //enable clock for the peripheral GPIO port F
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){}                                //wait for the clock to be ready
    GPIO_PORTF_LOCK_R = 0x4c4f434b;                                                             
    GPIO_PORTF_CR_R = 0x01f;
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);            //set the RGB LEDS to output pins
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0|GPIO_PIN_4);                        //set the switches of port F into inputs
    GPIO_PORTF_DEN_R=0x1f;                                                              //enable Digital input and output for all the pins 
    GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_4,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);// Enable weak pullup resistor for PF4
    GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);                             // Disable interrupt for PF4 (in case it was enabled)
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);                               // Clear pending interrupts for PF4
    GPIOIntRegister(GPIO_PORTF_BASE,Buttonfall);                                        // Register our handler function for port F
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0,GPIO_FALLING_EDGE);           // Configure PF4 for falling edge trigger 
    GPIOIntDisable(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0);                              // Enable interrupt for PF4
}
void Buttonrise(void)                                                                   //the handler function for the GPIO interuppt of unpressing the switch
{
   if (GPIOIntStatus(GPIO_PORTF_BASE, false) & GPIO_PIN_4)                              //check that the switch 1 is the reason for the interuppt
   {
    print("\n\rStopping operation");                                                    //output to the serial communication
    SysTickDisable();                                                                   //disable the systick interuppt to stop counting
    UARTIntEnable(UART0_BASE,UART_INT_RX);                                              //enable reciever UART interupt to wait for inputs from user
    i=1;                                                                                //flag
    GPIOIntDisable(GPIO_PORTF_BASE,GPIO_PIN_0|GPIO_PIN_4);                              //disable the switch GPIO interuppts to reconfigure a new handler
              
    TimerDisable(TIMER0_BASE , TIMER_A);                                                //disable the timer to prevent counting
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);                                          // Clear interrupt flag
    GPIOIntRegister(GPIO_PORTF_BASE, Buttonfall);                                       // Register our new handler function for port F when the button is pressed
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0,GPIO_FALLING_EDGE);           //set the interupt of the switches to be when the button is pressed not unpressed
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);                              // Configure PF4 and PF0 for rising edge trigger
   }
   if (GPIOIntStatus(GPIO_PORTF_BASE, false) & GPIO_PIN_0)                              //check that switch 0 is the reason for the interuppt
   {
      print("\n\rStarting operation");                                                  //output to the serial communication
      SysTickEnable();                                                                  //enable the systick timer to start counting
      UARTIntDisable(UART0_BASE,UART_INT_RX);                                           //disable the UART interuppt to prevent user input error
      GPIOIntDisable(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0);                            //disable the GPIO interuppt to reconfigure the handler functions
      GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);                                        // Clear interrupt flag
      GPIOIntRegister(GPIO_PORTF_BASE, Buttonfall);                                     // Register our handler function for port F
      GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0,GPIO_FALLING_EDGE);         //set the GPIO interuppt to be whrn a switch is pressed
      GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);                            // Configure PF4 for rising edge trigger
   }
}

void Buttonfall(void) {                                                                 //the handler function for the GPIO interuppt of pressing the switch
     if (GPIOIntStatus(GPIO_PORTF_BASE, false) & GPIO_PIN_4){TimerEnable(TIMER0_BASE,TIMER_A);}//if PF4 was interrupt cause,enable timer 0A
     GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);                            //disable the GPIO interuppt to reconfigure the handler functions
     GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);                              // Clear interrupt flag      
     GPIOIntRegister(GPIO_PORTF_BASE, Buttonrise);                                      // Register our handler function for port F
     GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4,GPIO_RISING_EDGE);                      // Configure PF4 & PF0 for rising edge trigger (when the button is unpressed)
      GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0,GPIO_RISING_EDGE);
     GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);                             //enable the interuppt for the 2 switches of GPIO F
}

void timer_0A_init()
{
    SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER0 );                                     //enable the clock for the timer0 peripheral
    TimerDisable(TIMER0_BASE , TIMER_A);                                                //disable Timer 0A before any configurations 
    TimerControlStall(TIMER0_BASE ,TIMER_A,true);                                       //ENABLE THE STALL WHILE DEBUGGING
    TimerConfigure(TIMER0_BASE ,TIMER_CFG_PERIODIC);                                    //PERIODIC DOWN COUNTER
    TimerLoadSet(TIMER0_BASE , TIMER_A,3*16000000);                                     //SET RELOAD REGISTER
    TimerIntRegister(TIMER0_BASE,TIMER_A,timer0_handler);                               //SET THE TIMER HANDLER FUNCTION
    TimerIntEnable(TIMER0_BASE,TIMER_TIMA_TIMEOUT );                                    //ENABLE TIMEOUT INTERUPPTS
}

void timer0_handler(void) //GPTM interupt handler
{ 
   TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);                                      //clear the interuppt for timer 0A
   GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4);                                         //disable the GPIO interuppt to reconfigure the handler functions
   GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);                                           // Clear interrupt flag
   GPIOIntRegister(GPIO_PORTF_BASE, Buttonfall);                                        // Register our handler function for port F
   GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4,GPIO_FALLING_EDGE);                       // Configure PF4 for falling edge trigger(when the switch is pressed)
   TimerDisable(TIMER0_BASE ,TIMER_A);                                                  //disable the timer to stop counting
   SysTickDisable();                                                                    //disable the systick timer
   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1,0);                                         //switch OFF all the LEDs
   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2,0);
   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3,0);  
   print("\n\rResetting");                                                              //output to the serial comm
   time[0]='0';
   time[1]='0';
   time[3]='0';
   time[4]='0'; 
   print(time);
   UARTIntEnable(UART0_BASE,UART_INT_RX);                                               //enable the reciever UART interuppt for user inputs
   GPIOIntDisable(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0);                               //disable any interuppt for the 2 switches to prevent errors
   mode();
}

void GPIOAint()                                                                         //initialize port A for use in UART
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);                                          //enable clock for the peripheral GPIO port A
while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)){}                                    // Wait for the GPIOA module to be ready.

}
void UART0int()                                                                         //initialize the UART0 for serial communication between the PC and the controller
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);                                          // Enable the UART0 module.
while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)){}                                    // Wait for the UART0 module to be ready.
GPIOAint(); 
GPIOPinConfigure(GPIO_PA0_U0RX);                                                        //identify RX
GPIOPinConfigure(GPIO_PA1_U0TX);                                                        //identify TX
GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);                              //alternative uart func
UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(),9600,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));//9600 Baud rate...word length 8.. 1 stop bit...ODD parity checker
UARTIntEnable(UART0_BASE,UART_INT_RX);                                                 // enable the reciever interuppt for the user input
UARTIntRegister(UART0_BASE,RX_INT_Handler);                                             //set the reciever interuppt handler function
UARTEnable(UART0_BASE);                                                                 //enable the UART
UARTFIFODisable(UART0_BASE);                                                            //disable the first in first out,so that the interuppt works char by char  
}
void RX_INT_Handler()
{
  
  char c;
  while(UARTCharsAvail(UART0_BASE))
  {
  c=UARTCharGet(UART0_BASE);  
  if(!i)
  {
   switch(c)
  {
  case 'm':
  case 'M':
    UARTCharPut(UART0_BASE,c);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2);
   print("\033\143");
   print("microwave \n\r");
   inctimer();
  break;
  case 'g':
  case 'G':
    UARTCharPut(UART0_BASE,c);
   GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,GPIO_PIN_3);
   print("\033\143");
   print("grill \n\r");
   inctimer();
   break;
  }
  }
  if(c=='M' ||c=='m'||c=='G'||c=='g') break;
  if(i)
  {
  switch(c)
  {
  case '1':
      time[3]++;
    break;
  case '2':
    time[1]++;
      break;
  case '3':
     time[0]+=3;
    break;
  } 
  if(time[3]>'6')
      {
        if(time[0]=='9' && time[1]=='9')
        {
          time[3]='6'; 
        break;
        }
        time[3]='0';
        time[1]++;
      }
  if(time[1]>'9')
      {
        if(time[0]=='9')
        {
          time[1]='9';
          print("\n\r Max minutes reached");
          break;
        }
        time[1]='0';
        time[0]++;
      }
    if(time[0]>'9')
      {
        time[0]='9';
      } 
  if(!(time[0]+time[1]+time[3]+time[4]==('0'*4)))
  {
    GPIOIntClear(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0); 
    GPIOIntEnable(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0);
  }
  print("\033\143");
  print(time); 
  }
}  
if(time[0]=='9' && time[1]=='9' && time[3]=='6') 
{
  print("MAX has been reached 99:60 Press Start");
}
 UARTIntClear(UART0_BASE,UART_INT_RX);
}

void inctimer()
{    
     i=1;
     print(time);
     print("\n\r Increment timer \n\r");
     print("1)10 sec \n\r");
     print("2)1 min \n\r");
     print("3)30 min \n\r");
     print("MAX timer is 99:60 \n\r");
}

void mode()                                                                             //the start page of the microwave
{
  i=0;                                                                                  //flag
  GPIOIntDisable(GPIO_PORTF_BASE,GPIO_PIN_4|GPIO_PIN_0);                                //disable GPIO interuppt to prevent user input errors
  //print("\033\143");                                                                  //clear the screen for better output in the PC
  print("Select mode \n\r");                                                            //output to the screen
  print("m)Microwave \n\r");                                                            //output to the screen
  print("g)Grill \n\r");                                                                //output to the screen
}

void SysTick_Init(){                                                                    //initialize Systick timer
  SysTickIntRegister(SysTick_Handler);                                                  //set the systic handler
  SysTickPeriodSet(16000000);                                                           //period set 1sec
  IntPrioritySet(15,0xE0);                                                              //piroirty set 7 as the least priority
  SysTickIntEnable();                                                                   //enable interrupt
}

void SysTick_Handler(void){
     time[4]--;
   if(time[4]<'0')
   { 
     time[4]='9';
     time[3]--;
   } 
  if(time[3]<'0')
   {
     time[3]='5';
     time[1]--;
   } 
   if(time[1]<'0')
   {
     time[1]='9';
     time[0]--;
   } 
 print("\033\143");
 print(time);
 if(time[0]+time[1]+time[3]+time[4]==('0'*4))
   {
   print("\033\143");
   print("DONE");
    SysTickDisable();
   //blink red
   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1,0);
   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2,0);
   GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3,0);   
   for(int i=0;i<5;i++)
    {
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
    SysCtlDelay(8000000);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1,0);
    SysCtlDelay(8000000);
    }
   mode();
   UARTIntEnable(UART0_BASE,UART_INT_RX);
  
   }
}