// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode

unsigned short currSW,prevSW,SWpressed_F;
// input from PA3, output from PA2, SysTick interrupts
void Sound_Init(void){ 
  unsigned long volatile delay;
	SYSCTL_RCGC2_R |= 0x00000001; // activate port A
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTA_AMSEL_R &= ~0x0C;      // no analog 
  GPIO_PORTA_PCTL_R &= ~0x0000FF00; // regular function
  GPIO_PORTA_DIR_R |= 0x04;     // make PA2 output, PA3 input
  GPIO_PORTA_DR8R_R |= 0x04;    // can drive up to 8mA out (PA2)
  GPIO_PORTA_AFSEL_R &= ~0x0C;  // disable alt funct on PA2, PA3
  GPIO_PORTA_DEN_R |= 0x0C;     // enable digital I/O on PA2, PA3
  NVIC_ST_CTRL_R = 0;           // disable SysTick during setup
  NVIC_ST_RELOAD_R = 90908;     // reload value for 1ms (assuming 80MHz)
  NVIC_ST_CURRENT_R = 0;        // any write to current clears it
  NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0                
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts
  EnableInterrupts();
}

// called at 880 Hz
void SysTick_Handler(void){
	
	currSW = GPIO_PORTA_DATA_R & 0x08; // get current switch state
	
	if(prevSW == 0 && currSW){	// switch press is detected (falling edge)
		SWpressed_F ^= 1;	// toggle flag for indicating how many times the switch is pressed
		if(SWpressed_F)	// switch is pressed odd times, so keep toggling output
		 GPIO_PORTA_DATA_R ^= 0x04;	// toggle output pin PA2
		else	// switch is pressed even times, so stop toggling output
		 GPIO_PORTA_DATA_R &= ~0x04; // set output low
	}else if(SWpressed_F){	// continue toggling if switch is pressed odd times but not pressed at the moment of executing the ISR
		GPIO_PORTA_DATA_R ^= 0x04;	// toggle output pin PA2
	}
	prevSW = currSW;	// store current switch state to be checked on the next time
}

int main(void){// activate grader and set system clock to 80 MHz
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2,ScopeOn); 
  Sound_Init();         
  EnableInterrupts();   // enable after all initialization are done
  while(1){
    // main program is free to perform other tasks
    // do not use WaitForInterrupt() here, it may cause the TExaS to crash
  }
}
