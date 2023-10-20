/*
 * Calculating_PI.c
 *
 * Created: 03.10.2023 18:45:07
 * Author : Borbak820
 */ 

//////////////////////////////////////////////////////////////////////////
//								Includes								//
//////////////////////////////////////////////////////////////////////////

//#include <avr/io.h>
#include "avr_compiler.h"
#include "pmic_driver.h"
#include "TC_driver.h"
#include "clksys_driver.h"
#include "sleepConfig.h"

#include "port_driver.h"
#include "math.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"

#include "queue.h"
#include "event_groups.h"
#include "stack_macros.h"
#include "mem_check.h"
#include "init.h"

#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"
#include "stdio.h"
#include "avr_f64.h"

#include "ButtonHandler.h"


//////////////////////////////////////////////////////////////////////////
//							Taskhandling								//
//////////////////////////////////////////////////////////////////////////

extern void vApplicationIdleHook(void);
void vLeibniz(void *pvParameter);
void vNikalantha(void *pvParameter);
void vUserInterface(void * pvParameter);
void vButtonHandler(void* pvParameter);

TaskHandle_t LeibnizTask;
TaskHandle_t NikalanthaTask;
TaskHandle_t ButtonTask;


//////////////////////////////////////////////////////////////////////////
//							Eventhandling								//
//////////////////////////////////////////////////////////////////////////

//Define Events for start and stop functions
#define EV_START_LEIBNIZ		1<<0	//1
#define EV_STOP_LEIBNIZ			1<<1	//2
#define EV_START_NIKA			1<<2	//4
#define EV_STOP_NIKA			1<<3	//8
#define EV_STOPPED_LEIBNIZ		1<<4	//16
#define EV_STOPPED_NIKA			1<<5	//32
#define EV_RESET_LEIBNIZ		1<<6	//64
#define EV_RESET_NIKA			1<<7	//128
#define EV_CLEAR				0xFF
EventGroupHandle_t evStartStopEvents;

//Define Events for Buttonpresses
#define EVBUTTONS_S1	1<<0	//1
#define EVBUTTONS_S2	1<<1	//2
#define EVBUTTONS_S3	1<<2	//4
#define EVBUTTONS_S4	1<<3	//8
#define EVBUTTONS_L1	1<<4	//16
#define EVBUTTONS_L2	1<<5	//32
#define EVBUTTONS_L3	1<<6	//64
#define EVBUTTONS_L4	1<<7	//128
#define EVBUTTONS_CLEAR	0xFF
EventGroupHandle_t evButtonEvents;

EventBits_t Bits;


//////////////////////////////////////////////////////////////////////////
//								Strings									//
//////////////////////////////////////////////////////////////////////////

char LeibnizPiString[16];
char LeibnizTimeString[3];
char LeibnizExactTime[10];
char NikalanthaPiString[16];
char NikalanthaTimeString[3];
char NikalanthaExactTime[10];


//////////////////////////////////////////////////////////////////////////
//							Global variables							//
//////////////////////////////////////////////////////////////////////////
int Menu = 0;


//////////////////////////////////////////////////////////////////////////
//						Idle Hook Application							//
//////////////////////////////////////////////////////////////////////////

void vApplicationIdleHook(void){
	
}


//////////////////////////////////////////////////////////////////////////
//								Main									//
//////////////////////////////////////////////////////////////////////////

int main(void){
	vInitClock();
	vInitDisplay();

	evStartStopEvents = xEventGroupCreate();
	evButtonEvents = xEventGroupCreate();

	xTaskCreate(vUserInterface, (const char *) "ControlTask", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate(vButtonHandler, (const char*) "ButtonTask", configMINIMAL_STACK_SIZE+30, NULL, 2, &ButtonTask);
	xTaskCreate(vLeibniz, (const char *) "Leibniz-Folge-Task", configMINIMAL_STACK_SIZE+100, NULL, 1, &LeibnizTask);
	xTaskCreate(vNikalantha, (const char *) "Nikalantha-Folge-Task", configMINIMAL_STACK_SIZE+300, NULL, 1, &NikalanthaTask);

	vTaskStartScheduler();
	
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//						Controller-/Interface-Task						//
//////////////////////////////////////////////////////////////////////////

void vUserInterface(void* pvParameters){
	vDisplayClear();
	vTaskDelay(500);
	initButtons();
	uint32_t DisplayUpdateCounter = 50;
	for(;;){
		uint32_t ButtonState = (xEventGroupGetBits(evButtonEvents)) & 0x000000FF; //Read Button States from EventGroup
		xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR); //As the Button State is saved now, we can clear the EventGroup for new Button presses
		
		
		//////////////////////////////////////////////////////////////////////////
		//							User Interface								//
		//////////////////////////////////////////////////////////////////////////
		
		if (DisplayUpdateCounter == 0){
			if(Menu == 0){	//Start screen
				vDisplayClear();
				vDisplayWriteStringAtPos(0,0, "Pi-Calculator");
				vDisplayWriteStringAtPos(1,0, "1: Pi aus math.h");
				vDisplayWriteStringAtPos(2,0, "2: Leibniz-Serie");
				vDisplayWriteStringAtPos(3,0, "3: Nikalantha-Serie");
			}
			if(Menu == 1){	//Pi Demo screen
				char pistring[12];
				vDisplayClear();
				sprintf(&pistring[0], "PI: %.8f", M_PI);
				vDisplayWriteStringAtPos(0,0, "Pi aus math.h");
				vDisplayWriteStringAtPos(1,0, "%s", pistring);
				vDisplayWriteStringAtPos(3,0, "4: Back");
			}
			if(Menu == 2){	//Leibniz's Pi screen
				vDisplayClear();
				vDisplayWriteStringAtPos(0,0, "%s", LeibnizPiString);
				vDisplayWriteStringAtPos(0,16, "%s", LeibnizTimeString);
				vDisplayWriteStringAtPos(1,0, "1: Start");
				vDisplayWriteStringAtPos(1,10, "%s", LeibnizExactTime);
				vDisplayWriteStringAtPos(2,0, "2: Stop");
				vDisplayWriteStringAtPos(2,10, "3: Reset");
				vDisplayWriteStringAtPos(3,0, "4: ~ Nikalantha");
			}
			if(Menu == 3){	//Nikalantha's Pi screen
				vDisplayClear();
				vDisplayWriteStringAtPos(0,0, "%s", NikalanthaPiString);
				vDisplayWriteStringAtPos(0,16, "%s", NikalanthaTimeString);
				vDisplayWriteStringAtPos(1,0, "1: Start");
				vDisplayWriteStringAtPos(1,10, "%s", NikalanthaExactTime);
				vDisplayWriteStringAtPos(2,0, "2: Stop");
				vDisplayWriteStringAtPos(2,10, "3: Reset");
				vDisplayWriteStringAtPos(3,0, "4: ~ Leibniz");
			}
			DisplayUpdateCounter = 50;	//Refreshing Time (50*10ms=500ms)	
		}
		else{
			DisplayUpdateCounter --;	//Refreshing counter
		}
	
		
		//////////////////////////////////////////////////////////////////////////
		//							Menu selection								//
		//////////////////////////////////////////////////////////////////////////
		
		if (Menu == 0){
			switch(ButtonState){
				case 1:
					Menu = 1;
				break;
					
				case 2:
					Menu = 2;
				break;
					
				case 4:
					Menu = 3;
				break;
			}
		}


		//////////////////////////////////////////////////////////////////////////
		//							Pi Demo functions							//
		//////////////////////////////////////////////////////////////////////////
		
		if (Menu == 1){
			if (ButtonState == 8) {	//Leave Demo - back to Start screen
					Menu = 0;
			}
		}
		
		
		//////////////////////////////////////////////////////////////////////////
		//						Leibniz's Pi functions							//
		//////////////////////////////////////////////////////////////////////////
		
		if(Menu == 2){
			switch (ButtonState){
				case 1:
				if (Bits == 0){	//First start since Main screen
					xEventGroupSetBits(evStartStopEvents, EV_START_LEIBNIZ);	//Set startbit
					Bits = xEventGroupGetBits(evStartStopEvents);	//Save eventbits as value
				}
				else if (Bits & (EV_STOPPED_NIKA | EV_STOP_NIKA)){	//Start if switched from Nikalantha's Pi
					xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
					xEventGroupSetBits(evStartStopEvents, EV_START_LEIBNIZ);	//Set startbit
					Bits = xEventGroupGetBits(evStartStopEvents);
					vTaskResume(LeibnizTask);	//Resume where stopped
				}
				else if (Bits & (EV_STOP_LEIBNIZ | EV_STOPPED_LEIBNIZ)){
					// Restart Leibniz calculation if stopped or resetted
					xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
					xEventGroupSetBits(evStartStopEvents, EV_START_LEIBNIZ);
					Bits = xEventGroupGetBits(evStartStopEvents);
					vTaskResume(LeibnizTask);
				}
				else if (Bits & EV_RESET_LEIBNIZ){
					vTaskResume(LeibnizTask);					
				}
				break;

				case 2:
				if(Bits & (EV_START_LEIBNIZ | EV_STOP_NIKA)){
					// Stop Leibniz calculation
					xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
					xEventGroupSetBits(evStartStopEvents, EV_STOP_LEIBNIZ);
					Bits = xEventGroupGetBits(evStartStopEvents);
				}
				break;

				case 4:
				if (Bits) {
					// Reset - set Leibniz calculation to 0
					xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
					xEventGroupSetBits(evStartStopEvents, EV_RESET_LEIBNIZ);
					Bits = xEventGroupGetBits(evStartStopEvents);
						sprintf(&LeibnizPiString[0], " ");
						sprintf(&LeibnizTimeString[0], " ");
						sprintf(&LeibnizExactTime[0], " ");
				}
				break;

				case 8:
					// Stop Leibniz calculation and switch to Nikalantha calculation
					xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
					xEventGroupSetBits(evStartStopEvents, EV_STOP_LEIBNIZ);
					Bits = xEventGroupGetBits(evStartStopEvents);
					Menu = 3;
					ButtonState = xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR);
				break;
			}
		}
		
		
		//////////////////////////////////////////////////////////////////////////
		//						Nikalantha's Pi functions						//
		//////////////////////////////////////////////////////////////////////////
		
		if (Menu == 3) {
			switch (ButtonState) {
				case 1:
					if (Bits == 0){	//First start since Main screen
						xEventGroupSetBits(evStartStopEvents, EV_START_NIKA);	//Set startbit
						Bits = xEventGroupGetBits(evStartStopEvents);	//Save eventbits as value
					}
					else if (Bits & (EV_STOP_LEIBNIZ | EV_STOPPED_LEIBNIZ)){	//Start if switched from Leibniz's's Pi
						xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
						xEventGroupSetBits(evStartStopEvents, EV_START_NIKA);	//Set startbit
						Bits = xEventGroupGetBits(evStartStopEvents);	//Save eventbits as value
						vTaskResume(NikalanthaTask);	//Resume where stopped
					}
					else if (Bits & EV_STOPPED_NIKA) {
						// Restart Nikalantha calculation if stopped or stopped and reset
						xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
						xEventGroupSetBits(evStartStopEvents, EV_START_NIKA);
						Bits = xEventGroupGetBits(evStartStopEvents);
						vTaskResume(NikalanthaTask);
					} 
					else if (Bits & EV_RESET_NIKA) {
						// Restart Nikalantha calculation if reseted
						vTaskResume(NikalanthaTask);
					}
					break;

				case 2:
				if(Bits & EV_START_NIKA){
						// Stop Nikalantha calculation
						xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
						xEventGroupSetBits(evStartStopEvents, EV_STOP_NIKA);
						Bits = xEventGroupGetBits(evStartStopEvents);
					}
					break;

				case 4:
					if (Bits){
						// Reset Nikalantha calculation to 0 if reseted or stopped
						xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
						xEventGroupSetBits(evStartStopEvents, EV_RESET_NIKA);
						Bits = xEventGroupGetBits(evStartStopEvents);
						sprintf(&NikalanthaPiString[0], " ");
						sprintf(&NikalanthaTimeString[0], " ");
						sprintf(&NikalanthaExactTime[0], " ");
					}
					break;

				case 8:
						// Stop Nikalantha calculation and switch to Leibniz calculation
						xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
						xEventGroupSetBits(evStartStopEvents, EV_STOP_NIKA);
						Bits = xEventGroupGetBits(evStartStopEvents);
						Menu = 2;
						ButtonState = xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR);
					break;
			}
		}
		vTaskDelay(10/portTICK_RATE_MS);
	}	
}


//////////////////////////////////////////////////////////////////////////
//							Button-Task									//
//////////////////////////////////////////////////////////////////////////

void vButtonHandler(void* pvParamter) {
	initButtons(); //Initialize Buttonhandler
	for(;;) {
		updateButtons(); //Update Button States
		
		//Read Button State and set EventBits in EventGroup
		if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
			xEventGroupSetBits(evButtonEvents, EVBUTTONS_S1);
		}
		if(getButtonPress(BUTTON2) == SHORT_PRESSED) {
			xEventGroupSetBits(evButtonEvents, EVBUTTONS_S2);
		}
		if(getButtonPress(BUTTON3) == SHORT_PRESSED) {
			xEventGroupSetBits(evButtonEvents, EVBUTTONS_S3);
		}
		if(getButtonPress(BUTTON4) == SHORT_PRESSED) {
			xEventGroupSetBits(evButtonEvents, EVBUTTONS_S4);
		}
		if(getButtonPress(BUTTON1) == LONG_PRESSED) {
			xEventGroupSetBits(evButtonEvents, EVBUTTONS_L1);
		}
		if(getButtonPress(BUTTON2) == LONG_PRESSED) {
			xEventGroupSetBits(evButtonEvents, EVBUTTONS_L2);
		}
		if(getButtonPress(BUTTON3) == LONG_PRESSED) {
			xEventGroupSetBits(evButtonEvents, EVBUTTONS_L3);
		}
		if(getButtonPress(BUTTON4) == LONG_PRESSED) {
			xEventGroupSetBits(evButtonEvents, EVBUTTONS_L4);
		}
		vTaskDelay((1000/BUTTON_UPDATE_FREQUENCY_HZ)/portTICK_RATE_MS); //Buttonupdate Delay
	}
}


//////////////////////////////////////////////////////////////////////////
//						Leibniz calculation								//
//////////////////////////////////////////////////////////////////////////

void vLeibniz(void *pvParameter){
	(void) pvParameter;
	float32_t PI;
	float32_t Summe;
	xEventGroupWaitBits(evStartStopEvents, EV_START_LEIBNIZ, pdFALSE, pdTRUE, portMAX_DELAY);
	TickType_t Starttime;
	TickType_t Endtime; 
	
	start_here:
	PI = 0;
	Summe = 0.0;
	TickType_t lastExecutionTime = xTaskGetTickCount();
	uint32_t i = -1;
	uint32_t n = 1;	
	int Elapsedtime = 0;
	int elapsedSeconds = 0;
	uint32_t currentTime = 0;
	uint32_t Elapsedcounter = 0;
	int Codeblocker = 0;
	Starttime  = xTaskGetTickCount();
	
	for(i = 0; i < n; i ++){
		currentTime = xTaskGetTickCount();
		Elapsedcounter = currentTime - lastExecutionTime;
		if(Bits & EV_STOP_LEIBNIZ){	//Stop function
		    xEventGroupSetBits(evStartStopEvents, EV_STOPPED_LEIBNIZ);
			Bits = xEventGroupGetBits(evStartStopEvents);
			Bits &= (~EV_START_LEIBNIZ & ~EV_STOP_LEIBNIZ) ;  // EV_START_LEIBNIZ löschen
            vTaskSuspend(NULL);
		}
		else if (Bits & EV_RESET_LEIBNIZ){	//Reset function
			xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
            xEventGroupSetBits(evStartStopEvents, EV_STOPPED_LEIBNIZ);
			Bits = xEventGroupGetBits(evStartStopEvents);
			memset(LeibnizPiString, 0, sizeof(LeibnizPiString));		// Der String wird geleert.
			memset(LeibnizTimeString, 0, sizeof(LeibnizTimeString));		// Der String wird geleert.
			memset(LeibnizExactTime, 0, sizeof(LeibnizExactTime));		// Der String wird geleert.
			vTaskSuspend(NULL);
			goto start_here;
		}
		
		Summe += (i % 2 == 0 ? 1 : -1) / (2.0 * i + 1);
		PI = 4 * Summe;
		n ++;
		
		if (Elapsedcounter >= pdMS_TO_TICKS(500)) {
			sprintf(&LeibnizPiString[0], "PI is %.7f", PI);
			sprintf(&LeibnizTimeString[0], "%ds", (elapsedSeconds / 2));
			lastExecutionTime = currentTime;
			elapsedSeconds ++;
		}
		
		if(Codeblocker == 0 && (uint32_t)((PI*100000) > 314159 && (PI*100000) < 314160)){
			Endtime = xTaskGetTickCount() - Starttime;
			Elapsedtime += Endtime;
			sprintf(&LeibnizExactTime[0], "Pi in %ds", (Elapsedtime / 1000));
			Codeblocker = 1;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//						Nilakantha calculation							//
//////////////////////////////////////////////////////////////////////////

void vNikalantha(void *pvParameter){
	(void) pvParameter;
	TickType_t Starttime;
	TickType_t Endtime;
	double Zaehler;
	xEventGroupWaitBits(evStartStopEvents, EV_START_NIKA, pdFALSE, pdTRUE, portMAX_DELAY);
    while (1) {

        start_here:
		xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
        Bits = xEventGroupSetBits(evStartStopEvents, EV_START_NIKA);
		int Elapsedtime = 0;
        float32_t PI = 0;
		TickType_t lastExecutionTime = xTaskGetTickCount();
        long int i = 0;
		long int n = 2;
		int elapsedSeconds = 0;
		uint32_t currentTime = 0;
		uint32_t Elapsedcounter = 0;
		int codeblock = 0;
		Zaehler = -1;
		Starttime  = xTaskGetTickCount();
        if (i == 0) {
            // Startwert PI = 3
            PI = 3;
            Zaehler *= -1;
            PI = PI + (Zaehler * 4 / (n * (n + 1) * (n + 2)));
            n += 2;
            i += 1;
        }

        for (i = 1; i < n; i++) {
			currentTime = xTaskGetTickCount();
			Elapsedcounter = currentTime - lastExecutionTime;
            if (Bits & EV_STOP_NIKA) {
				xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
                xEventGroupSetBits(evStartStopEvents, EV_STOPPED_NIKA);
				Bits = xEventGroupGetBits(evStartStopEvents);
                vTaskSuspend(NULL);
            }
            if (Bits & EV_RESET_NIKA) {
				xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
                xEventGroupSetBits(evStartStopEvents, EV_STOPPED_NIKA);
				memset(NikalanthaPiString, 0, sizeof(NikalanthaPiString));			// Der String wird geleert.
				memset(NikalanthaTimeString, 0, sizeof(NikalanthaTimeString));		// Der String wird geleert.
				memset(NikalanthaExactTime, 0, sizeof(NikalanthaExactTime));		// Der String wird geleert.
				Bits = xEventGroupGetBits(evStartStopEvents);
				vTaskSuspend(NULL);
                goto start_here;
            }
			
			Zaehler *= -1;	
			PI = PI +(Zaehler * 4 / (n * (n + 1) * (n + 2)));
			n += 2;
			
			if (Elapsedcounter >= pdMS_TO_TICKS(500)) {
				sprintf(&NikalanthaPiString[0], "PI is %.7f", PI);
				sprintf(&NikalanthaTimeString[0], "%ds", (elapsedSeconds / 2));
				lastExecutionTime = currentTime;
				elapsedSeconds ++;
			}
			
			if(codeblock == 0 && (uint32_t)((PI*100000) > 314159 && (PI*100000) < 314160)){
				Endtime = xTaskGetTickCount() - Starttime;
				Elapsedtime += Endtime;
				sprintf(&NikalanthaExactTime[0], "Pi in %dms", Elapsedtime);
				codeblock = 1;
			}
		}
	}	
}