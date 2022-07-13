#include "xscugic.h"
#include "xtmrctr.h"
#include "xparameters.h"
#include <XGpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <MyDisp.h>
#include <thread>
#include <iostream>

//Defines for interrupt IDs
#define INTC_DEVICE_ID XPAR_PS7_SCUGIC_0_DEVICE_ID
#define GPIO_INT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define TIMER_INT_ID XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR

//Global variables accessible from any function
XTmrCtr timer;
XGpio input;
MYDISP display;

//Global Interrupt Controller
static XScuGic GIC;

int dir = 0; //0 = left, 1 = right, 2 = up, 3 = down
int w = 16;
int w_b = 10; //length of barrels
int cx = 8;
int cy = 301;
int vx = 16;
int vy = 1;
int vz = 10;
int dx[5] = {0,240,0,220,0}; // x-position of barrels
int dy[5] = {19,76,131,190,234}; // y-position of barrels
int vx1[5] = {16,16,16,16,16};
int vy1[5] = {1,1,1,1,1};
int vz1[5] = {10,10,10,10,10};
int lives = 3;

int switch_data;

//This function initalizes the GIC, including vector table setup and CPSR IRQ enable
void initIntrSystem(XScuGic * IntcInstancePtr) {

	XScuGic_Config *IntcConfig;
	IntcConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
	XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, (void *)IntcInstancePtr);
	Xil_ExceptionEnable();


}
void barrels_moving(){
	for(int counter = 0; counter < 5; counter++){ // barrels moving
		if (((dy[counter] > 228) && (dy[counter] <= 260) && (dx[counter] < 240-w_b/2)) || ((dy[counter] > 125) && (dy[counter] <= 149) && (dx[counter] >= 0) && (dx[counter] < 210)) || ((dy[counter] > 0) && (dy[counter] < 49) && (dx[counter] < 240-w_b/2))){ // barrels move right
			display.setForeground(clrWhite);
			display.drawEllipse(true, dx[counter]-w_b/2, dy[counter]-w_b/2, dx[counter]+w_b/2, dy[counter]+w_b/2);
			dx[counter] += vx1[counter];
			dy[counter] += vy1[counter];
			display.setForeground(clrGreen);
			display.drawEllipse(true, dx[counter]-w_b/2, dy[counter]-w_b/2, dx[counter]+w_b/2, dy[counter]+w_b/2);
		}
		 if ((((dy[counter] >= 71) && (dy[counter] <= 94)) || ((dy[counter] >= 185) && (dy[counter] <= 207)) || ((dy[counter] >= 283) && (dy[counter] <= 310))) && (dx[counter] > w_b/2)){ // barrels move left
			display.setForeground(clrWhite);
			display.drawEllipse(true, dx[counter]-w_b/2, dy[counter]-w_b/2, dx[counter]+w_b/2, dy[counter]+w_b/2);
			dx[counter] -= vx1[counter];
			dy[counter] += vy1[counter];
			display.setForeground(clrGreen);
			display.drawEllipse(true, dx[counter]-w_b/2, dy[counter]-w_b/2, dx[counter]+w_b/2, dy[counter]+w_b/2);
		}
		 if (((dx[counter] > 224) && (dx[counter] <= 240) && (dy[counter] > 0) && (dy[counter] <= 85)) || ((dx[counter] > 10) && (dx[counter] < 32) && (dy[counter] > 80) && (dy[counter] < 128)) || ((dx[counter] > 185) && (dx[counter] < 240) && (dy[counter] > 130) && (dy[counter] < 185)) || ((dx[counter] > 16) && (dx[counter] < 48) && (dy[counter] > 190) && (dy[counter] < 224)) || ((dx[counter] > 224) && (dx[counter] <= 240) && (dy[counter] > 230) && (dy[counter] <= 310))){ // barrels move down
			display.setForeground(clrWhite);
			display.drawEllipse(true, dx[counter]-w_b/2, dy[counter]-w_b/2, dx[counter]+w_b/2, dy[counter]+w_b/2);
			dy[counter] += vz1[counter];
			display.setForeground(clrGreen);
			display.drawEllipse(true, dx[counter]-w_b/2, dy[counter]-w_b/2, dx[counter]+w_b/2, dy[counter]+w_b/2);
		}
//		 for (int i = 0; i < 33000000; i++);
		 if((cy == dy[counter]) || (cx == dx[counter])){
			if(lives == 3){display.drawText((char*)"lives: 3", 150, 10);}
			lives -= 1;
			if(lives == 2){display.drawText((char*)"lives: 2", 150, 10);}
			if(lives == 1){display.drawText((char*)"lives: 1", 150, 10);}
			if(lives == 0){
				display.clearDisplay(clrWhite);
				display.setPen(penSolid);
				display.setForeground(clrRed);
				display.drawText((char*)"Game Over", 85, 160);
				for (int i = 0; i < 330000000; i++);
			}
		}
	}
}
void platforms(){
	for(int k = 0; k <= 15; k++){ //1st sladder
		display.setForeground(clrRed);
		display.drawRectangle(true, k*16, 310-k, (k+1)*16, 320-k);
	}
	for(int j = 0; j <= 12; j++){ //2nd sladder
		display.setForeground(clrRed);
		display.drawRectangle(true, j*16, 240+j, (j+1)*16, 250+j);
	}
	for(int h = 3; h <= 13; h++){ // 3rd sladder
		display.setForeground(clrRed);
		display.drawRectangle(true, h*16, 210-h, (h+1)*16, 220-h);
	}
	for(int m = 0; m <= 10; m++){ // 4th sladder
		display.setForeground(clrRed);
		display.drawRectangle(true, m*16, 138+m, (m+1)*16, 148+m);
	}
	for(int n = 2; n <= 14; n++){ // 5th sladder
		display.setForeground(clrRed);
		display.drawRectangle(true, n*16, 96-n, (n+1)*16, 106-n);
	}
	for(int p = 0; p <= 12; p++){ // 6th sladder
		display.setForeground(clrRed);
		display.drawRectangle(true, p*16, 26+p, (p+1)*16, 36+p);
	}

	for(int a = 1; a <= 5; a++){ // 1st stairs
		display.setForeground(clrBlue);
		display.drawRectangle(true, 80, 305-10*a, 96, 315-10*a);
	}
	for(int b = 1; b <= 4; b++){ // 2nd stairs
		display.setForeground(clrBlue);
		display.drawRectangle(true, 160, 250-10*b, 176, 260-10*b);
	}
	for(int c = 1; c <= 5; c++){ // 3rd stairs
		display.setForeground(clrBlue);
		display.drawRectangle(true, 96, 204-10*c, 112, 214-10*c);
	}
	for(int d = 1; d <= 4; d++){ // 4st stairs
		display.setForeground(clrBlue);
		display.drawRectangle(true, 64, 142-10*d, 80, 152-10*d);
	}
	for(int e = 1; e <= 4; e++){ // 5st stairs
		display.setForeground(clrBlue);
		display.drawRectangle(true, 160, 86-10*e, 176, 96-10*e);
	}
	for(int fb = 1; fb <= 2; fb++){ // Incomplete 2nd stairs
		display.setForeground(clrBlue);
		display.drawRectangle(true, 48, 243-10*fb, 64, 253-10*fb);
	}
	for(int fd = 1; fd <= 4; fd++){ // Incomplete 4st stairs
		display.setForeground(clrBlue);
		display.drawRectangle(true, 160, 148-10*fd, 176, 158-10*fd);
	}
}

//Weekly exercise - complete this function!
void timerInterruptHandler(void *userParam, u8 TmrCtrNumber) {

}

//Weekly exercise - complete this function!
void buttonInterruptHandler(void *instancePointer) {
	//Read the button state using XGpio_DiscreteRead
	//Set dir based on which bit is a '1'
	switch_data = XGpio_DiscreteRead(&input, 1);
	if(switch_data == 1) dir = 0;
	else if(switch_data == 2) dir = 1;
	else if(switch_data == 4) dir = 2;
	else if(switch_data == 8) dir = 3;
	
	XGpio_InterruptClear(&input, GPIO_INT_ID); //Leave this line at the end of this function
}


int main() {

	initIntrSystem(&GIC);

	//Prelab Assignment 1
	//Configure GPIO input and output and set direction as usual
	//Note that the input variable is already declared as a global variable
	XGpio_Initialize(&input, XPAR_AXI_GPIO_0_DEVICE_ID);
	XGpio_SetDataDirection(&input, 1, 0xF); //1 = input, 0 = output

	//Prelab Assignment 2
	//Configure Timer and timer interrupt as done in class, and comment every line
	XTmrCtr_Initialize (&timer, XPAR_AXI_TIMER_0_DEVICE_ID);
	XTmrCtr_SetHandler(&timer, ( XTmrCtr_Handler ) timerInterruptHandler, ( void*) 0x12345678 );
	XScuGic_Connect (&GIC, TIMER_INT_ID, ( Xil_InterruptHandler ) XTmrCtr_InterruptHandler, &timer);
	XScuGic_Enable (&GIC, TIMER_INT_ID );
	XScuGic_SetPriorityTriggerType (&GIC, TIMER_INT_ID, 0x0, 0x3 );
	XTmrCtr_SetOptions (&timer, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
	XTmrCtr_SetResetValue (&timer, 0, 0xFFFFFFFF - 33333333); // 1 Hz
	XTmrCtr_Start(&timer, 0);


	//Prelab Assignment 3
	//Configure GPIO interrupt as done in class, and comment every line
	XScuGic_Connect (&GIC, GPIO_INT_ID , ( Xil_ExceptionHandler ) buttonInterruptHandler, &input);
	XGpio_InterruptEnable (&input, XGPIO_IR_CH1_MASK);
	XGpio_InterruptGlobalEnable (&input);
	XScuGic_Enable (&GIC, GPIO_INT_ID);
	XScuGic_SetPriorityTriggerType (&GIC, GPIO_INT_ID, 0x8, 0x3);

	display.begin();
	display.clearDisplay(clrWhite);


	//In the main loop move the square based on the global direction variable
	//Set the square's color based on the global color variable
	//You can use delay in the main loop to set the speed that the square moves


	while (true) {
		//dir: 0 = left, 1 = right, 2 = up
		barrels_moving();
		platforms();

		if (dir == 0) { //Move left
			if ((((cy > 260) && (cy < 320)) ||((cy > 149) && (cy < 207)) || ((cy > 49) && (cy < 98))) && (cx > w/2)){
				display.setForeground(clrWhite);
				display.drawEllipse(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
				cx -= vx;
				cy += vy;
				display.setForeground(clrBlack);
				display.drawEllipse(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
			}
			else if ((((cy > 210) && (cy < 250)) || ((cy > 88) && (cy < 142)) || ((cy > 0) && (cy < 49))) && (cx > w/2)){
				display.setForeground(clrWhite);
				display.drawEllipse(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
				cx -= vx;
				cy -= vy;
				display.setForeground(clrBlack);
				display.drawEllipse(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
			}
		}
		else if (dir == 1) { //Move right
			if ((((cy > 260) && (cy < 320)) || ((cy > 149) && (cy < 207)) || ((cy > 49) && (cy < 98))) && (cx < 240 - w/2)){
				display.setForeground(clrWhite);
				display.drawEllipse(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
				cx += vx;
				cy -= vy;
				display.setForeground(clrBlack);
				display.drawEllipse(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
			}
			else if ((((cy > 210) && (cy < 250)) || ((cy > 88) && (cy < 142)) || ((cy > 0) && (cy < 49))) && (cx < 240 - w/2)){
				display.setForeground(clrWhite);
				display.drawEllipse(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
				cx += vx;
				cy += vy;
				display.setForeground(clrBlack);
				display.drawEllipse(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
			}
		}
		else if (dir == 2) { //Move up
			if (((cx > 80) && (cx < 96) && (cy > 240) && (cy <= 310)) || (((cx > 160) && (cx < 176)) && (cy > 200) && (cy <= 260)) || (((cx > 96) && (cx < 112)) && (cy > 144) && (cy <= 204)) || (((cx > 64) && (cx < 80)) && (cy > 91) && (cy <= 143)) || (((cx > 160) && (cx < 176)) && (cy > 36) && (cy <= 86))){
				display.setForeground(clrBlue);
				display.drawEllipse(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
				cy -= vz;
				display.setForeground(clrBlack);
				display.drawEllipse(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
				if(((cx > 80) && (cx < 96) && (cy > 245)&& (cy < 255)) || ((cx > 160) && (cx < 176) && (cy > 200) && (cy < 210)) || ((cx > 96) && (cx < 112) && (cy > 144) && (cy < 154)) || ((cx > 64) && (cx < 80) && (cy > 92) && (cy < 102)) || ((cx > 160) && (cx < 176) && (cy > 36) && (cy < 46))){
					display.setForeground(clrRed);
					display.drawRectangle(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
					cy -= vz;
					display.setForeground(clrWhite);
					display.drawRectangle(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
					display.setForeground(clrBlack);
					display.drawEllipse(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
				}
			}
		}
		if((cy > 0) && (cy <= 26) && (cx > 0) && (cx <= 16)){
			display.clearDisplay(clrWhite);
			display.setPen(penSolid);
			display.setForeground(clrRed);
			display.drawText((char*)"Winner", 85, 160);
			for (int i = 0; i < 330000000; i++);
		}
		display.setForeground(clrBlack);
		display.drawEllipse(true, cx-w/2, cy-w/2, cx+w/2, cy+w/2);
	}
}

