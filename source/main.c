/*	Author: Benjamin Shu
 *  Partner(s) Name: 
 *	Lab Section: 22
 *	Assignment: Lab #10  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "timer.h"
#include "keypad.h"
#include "bit.h"
#include "scheduler.h"

unsigned char input = 0x00;
unsigned char code[5] = {0,0,0,0,0};
unsigned char test[1] = {1};
unsigned char count = 0;
unsigned char num = 0;

enum Keypad_States {Start, InitRelease, Input, InputWait, CodeCheck};
enum Lock_States {Lock};
int Keypad_SM(int state) {
	input = GetKeypadKey();
	switch(state) {
		case Start:
			if(input != '#') {
				state = Start;
			}
			else {
				state = InitRelease;
			}
			break;
		case InitRelease:
			if(input == '#') {
				state = InitRelease;
			}
			else {
				state = Input;
			}
			break;
		case Input:
			if(input != '\0') {
				code[count++] = input;
				state = InputWait;	
			}
			if(count == 5) {
				state = CodeCheck;
			}
			break;
		case InputWait:
			if(input != '\0') {
				state = InputWait;
			}
			else {
				state = Input;
			}
			break;
	}
		
	switch(state) {
		case Start:
			PORTB = 1;
			break;
		case InitRelease:
			break;
		case Input:
			break;
		case InputWait:
			break;
		case CodeCheck:
			if(code[0] == '1' && code[1] == '2' && code[2] == '3' && code[3] == '4' && code[4] == '5') {
				PORTB = 0;
			}
			for(int i = 0; i < 5; i++) {
				code[i] = 0;
			}
			state = Start;
			break;
		default:
			state = Start;
	}
	return state;
}

int Lock_SM(int state) {
	switch(state) {
		case Lock:
			if((~PINB & 0x80) == 0x80) {
				PORTB = 0x00;
			}
			break;
		default:
			break;

	}
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0x7F; PORTB = 0x80;
	DDRC = 0xF0; PORTC = 0x0F;
    /* Insert your solution below */
	static task task1, task2;
	task *tasks[] = {&task1, &task2};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = 0;

	//task 1 (keypad detection)
	task1.state = start;
	task1.period = 50;
	task1.elapsedTime = task1.period;
	task1.TickFct = &Keypad_SM;
	
	//task 2 (lock mechanism)
	task2.state = start; 
	task2.period = 50;
	task2.elapsedTime = task2.period;
	task2.TickFct = &Lock_SM;


	TimerSet(50);
	TimerOn();

	unsigned short i;

    while (1) {
		for(i = 0; i < numTasks; i++) { //scheduler code
			if(tasks[i]->elapsedTime == tasks[i]->period) { //task ready to tick
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state); //set next state
				tasks[i]->elapsedTime = 0; //reset elapsed time for next tick
			}
			tasks[i]->elapsedTime += 50;
		}
		while(!TimerFlag);
		TimerFlag = 0;
    }
    return 1;
}
