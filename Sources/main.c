#include <hidef.h>      /* common defines and macros */
#include <mc9s12c128.h>     /* derivative information */
#pragma LINK_INFO DERIVATIVE "mc9s12c128"


//250 micro seconds
#define PERIODMICRO 480
//5 Milli seconds
#define PERIODMILLI 9950

//pulse
int pulseShort=0;
//init pulse to indicate transmission
int beginningPulse = 4;
//boolean for transmission
int transmitting =0;
//index of bit being tranmitted
int bit = 0;
//the byte to transmit
byte transmission;


void SetupOC() {
 //setup timer channel 0 and 1 for OC
 TIOS = TIOS | 0x03;
 // Enable the timer
 TSCR1 = 0x80;
// Enable interrupts for Output Compare at channel 0 and 1.
 TIE |= 0x03;
// Specify that we will be using channel 0 and 1 for output compare.
// Note that each channel can be either configured as output compare or
// input capture (we have not studied this concept yet)
 TFLG1 |= 0x01;
// Set the Data register for Channel 0 to be after 2000 clock ticks
// This mainly means that the channel 0 interrupt will be called
// after TCNT counter reaches the value TCNT+period
 TC0 = TCNT + PERIODMICRO;
 TC1 = TCNT + PERIODMILLI;
 } 

//this is to handle the init pulse to indicated beginning of transmission
void interrupt 8 TC0handler(void) {

  // ack the interrupt
  TFLG1 |= 0x01;
  // setup the next interrupt to be after period number of clock ticks
  TC0 = TCNT + PERIODMICRO;
  if(pulseShort == 0 && beginningPulse < 4 && transmitting) {
    PORTB_BIT0 = 1;
    pulseShort= 1;
    beginningPulse++;
  } else if (pulseShort == 1 && beginningPulse < 4 && transmitting) {
    PORTB_BIT0 = 0;
    pulseShort = 0;
    beginningPulse++;
 }
 
}

//this is to handle the transmission of the byte itself
void interrupt 9 TC1handler(void) {

  // ack the interrupt
  TFLG1 |= 0x02;
  // setup the next interrupt to be after period number of clock ticks
  TC1 = TCNT + PERIODMILLI;
  if ((beginningPulse < 4) && transmitting){
  } else {
       if (transmitting && (bit < 8)){
          PORTB_BIT0 =  (transmission>>(7-bit)) & 0x1;
          bit++;        
       } if (transmitting && (bit > 8)){
          transmitting =0;
          bit =0;
          PORTB_BIT0 = 0;
       } if (transmitting && (bit == 8)){bit++;}
  }
}

//transmit the byte
void transmitByte (byte trans){

  transmission = trans;
  beginningPulse =0;
  transmitting = 1; 
  //wait for transmission to finish
  while (transmitting){PORTB_BIT3=0x1;
  };
                   PORTB_BIT3=0x0;
}

//transmit string in bytes
void SendString (char* text){
  int leng = sizeof (text) / sizeof (text[0]);
  char* temp= text;
  int i;
  for( i = 0; temp[i] != '\0'; i++){
    transmitByte (temp[i]);
  }
}

//transmit the string Test!
void TransmitTest () {
   SendString ("TEST!");
}

   
void main(void) {
  /* put your own code here */

  char* test /*= (char*)0x8A;*/ = "TEST!";
  DDRA= 0x0;
  DDRB= 0xFF;
	EnableInterrupts;              

  SetupOC();
    
  for(;;) {
    _FEED_COP(); /* feeds the dog */
    if (PORTA_BIT0 == 0x0){
      PORTB_BIT2 = 0x1;
      TransmitTest ();
      while (PORTA_BIT0 == 0x0){PORTB_BIT2=0x1;};
      PORTB_BIT2 = 0x0;

    }
  } /* loop forever */
  /* please make sure that you never leave main */
}
