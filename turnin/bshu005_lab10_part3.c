/*	Author: Benjamin Shu
 *  Partner(s) Name: 
 *	Lab Section: 22
 *	Assignment: Lab #10  Exercise #3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *	LInk to Demo: https://www.youtube.com/watch?v=BRoFrDxQvTE
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "timer.h"
#include "keypad.h"
#include "bit.h"
#include "scheduler.h"
#include "pwm.h"

unsigned char input = 0x00;
enum Keypad_States {Start, One, Wait1, Two, Wait2, Three, Wait3, Four, Wait4, Five, Wait5, Six, Wait6, Unlock};
enum Lock_States {Lock};
enum Doorbell_States {Begin, Play, Stop};

unsigned count = 0; //accounts for position
double note[30] = {329.63, 523.25, 329.63, 493.88, 329.63, 440, 392, 349.23, 392, 261.63, 293.66, 349.23, 440, 523.25, 493.88, 392, 329.63, 293.66, 329.63, 293.66, 261.63, 246.94, -1};
double delay[30] = {200, 900, 200, 900, 400, 500, 350, 350, 1000, 400, 320, 325, 325, 325, 325, 325, 325, 325, 1000, 220, 200, 1000, 200};

int Keypad_SM(int state) {
	input = GetKeypadKey();
	switch(state) {
		case Start:
			state = One;
			break;
		case One:
			if(input == '#') {
				state = Wait1;
			}
			else {
				state = One;
			}
			break;
		case Wait1:
			if(input == '#') {
				state = Wait1;
			}
			else {
				state = Two;
			}
			break;
		case Two:
			if(input == '1') {
				state = Wait2;
			}
			else {
				state = Two;
			}
			break;
		case Wait2:
			if(input == '1') {
				state = Wait2;
			}
			else {
				state = Three;
			}
			break;
		case Three:
			if(input == '2') {
				state = Wait3;
			}
			else {
				state = Three;
			}
			break;
		case Wait3:
			if(input == '2') {
				state = Wait3;
			}
			else {
				state = Four;
			}
			break;
		case Four:
			if(input == '3') {
				state = Wait4;
			}
			else {
				state = Four;
			}
			break;
		case Wait4:
			if(input == '3') {
				state = Wait4;
			}
			else {
				state = Five;
			}
			break;
		case Five:
			if(input == '4') {
				state = Wait5;
			}
			else {
				state = Five;
			}
			break;
		case Wait5:
			if(input == '4') {
				state = Wait5;
			}
			else {
				state = Six;
			}
			break;
		case Six:
			if(input == '5') {
				state = Wait6;
			}
			else {
				state = Six;
			}
			break;
		case Wait6:
			if(input == '5') {
				state = Wait6;
			}
			else {
				state = Unlock;
			}
			break;
		case Unlock:
			PORTB = 1;
			state = Start;
		default:
			state = Start;
			break;
	}
	switch(state) {
		case Start:
			break;
		case One:
			break;
		case Wait1:
			break;
		case Two:
			break;
		case Wait2:
			break;
		case Wait3:
			break;
		default:
			break;

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
int Doorbell_SM(int state) {
	switch(state) {
		case Begin:
			if((~PINA & 0x80) == 0x80) {
				state = Play;
			}
			else {
				state = Begin;
			}
			break;
		case Play:
			if(note[count] != -1) {
				TimerSet(1000);
				state = Play;
			}
			else if(note[count] == -1 && (~PINA & 0x01) == 0x00) {
				state = Begin;
			}
			else if(note[count] == -1 && (~PINA & 0x01) == 0x01) {
				count = 0;
				TimerSet(100);
				state = Stop;
			}
			else {
				state = Begin;
			}
			break;
		case Stop:
			if((~PINA & 0x80) == 0x80) {
				state = Stop;
			}
			else {
				state = Start;
			}
			break;
	}
	switch(state) {
		case Begin:
			set_PWM(0);
			count = 0;
			break;
		case Play:
			if(note[count] != -1) {
				TimerSet(delay[count]);
			}
			else {
				TimerSet(100);
			}
			set_PWM(note[count]);
			count++;
			break;
		case Stop:
			set_PWM(0);
			count = 0;
			break;
	}
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0x7F; PORTB = 0x80;
	DDRC = 0xF0; PORTC = 0x0F;
    /* Insert your solution below */
	static task task1, task2, task3;
	//static task task1;
	//task *tasks[] = {&task1};
	task *tasks[] = {&task1, &task2, &task3};
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
	
	//task 3 (doorbell)
	task3.state = start;
	task3.period = 50;
	task3.elapsedTime = task3.period;
	task3.TickFct = &Doorbell_SM;

	PWM_on();
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
