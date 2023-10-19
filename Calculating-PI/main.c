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
int vLeibniz(void *pvParameter);
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

char NikalanthaString[20];
char LeibnizString[20];
char TimeStringL[20];
char TimeStringN[20];


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
	xTaskCreate(vNikalantha, (const char *) "Nikalantha-Folge-Task", configMINIMAL_STACK_SIZE+100, NULL, 1, &NikalanthaTask);

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
				vDisplayWriteStringAtPos(0,0, "%s", LeibnizString);
				vDisplayWriteStringAtPos(1,0, "1: Start");
				vDisplayWriteStringAtPos(1,10, "2: Stop");
				vDisplayWriteStringAtPos(2,0, "3: Reset");
				vDisplayWriteStringAtPos(2,10, "%s", TimeStringL);
				vDisplayWriteStringAtPos(3,0, "4: --> Nikalantha");
			}
			if(Menu == 3){	//Nikalantha's Pi screen
				vDisplayClear();
				vDisplayWriteStringAtPos(0,0, "%s", NikalanthaString);
				vDisplayWriteStringAtPos(1,0, "1: Start");
				vDisplayWriteStringAtPos(1,10, "2: Stop");
				vDisplayWriteStringAtPos(2,0, "3: Reset");
				vDisplayWriteStringAtPos(2,10, "%s", TimeStringN);
				vDisplayWriteStringAtPos(3,0, "4: --> Leibniz");
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
				if (Bits == 0){	//First start
					xEventGroupSetBits(evStartStopEvents, EV_START_LEIBNIZ);	//Set startbit
					Bits = xEventGroupGetBits(evStartStopEvents);	//Save eventbits as value
				}
				else if (Bits & EV_STOP_NIKA){	//Start if switched from Nikalantha's Pi
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
					sprintf(LeibnizString, " ");
					sprintf(TimeStringL, " ");
					vTaskSuspend(LeibnizTask);
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
					if (Bits == 0){
						xEventGroupSetBits(evStartStopEvents, EV_START_NIKA);
					}
					else if (Bits & (EV_STOPPED_LEIBNIZ | EV_STOP_NIKA)) {
						// Start Nikalantha calculation
						Bits &= ~EV_STOP_NIKA;  // EV_STOP_NIKA löschen
						Bits = xEventGroupSetBits(evStartStopEvents, EV_START_NIKA);
						vTaskResume(NikalanthaTask);
					} 
					else if (Bits & (EV_STOP_NIKA | EV_STOPPED_NIKA)) {
						// Restart Nikalantha calculation if stopped or stopped and reset
						xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
						xEventGroupSetBits(evStartStopEvents, EV_START_NIKA);
						Bits = xEventGroupGetBits(evStartStopEvents);
						vTaskResume(NikalanthaTask);
					} 
					else if (Bits == EV_RESET_NIKA) {
						// Restart Nikalantha calculation if reseted
						vTaskResume(NikalanthaTask);
					}
					break;

				case 2:
				if(Bits & (EV_START_NIKA | EV_STOPPED_LEIBNIZ)){
						// Stop Nikalantha calculation
						xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
						xEventGroupSetBits(evStartStopEvents, EV_STOP_NIKA);
						Bits = xEventGroupGetBits(evStartStopEvents);
					}
					break;

				case 4:
					if (Bits & (EV_START_NIKA | EV_STOP_NIKA)) {
						// Reset Nikalantha calculation to 0 if reseted or stopped
						xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
						xEventGroupSetBits(evStartStopEvents, EV_RESET_NIKA);
						Bits = xEventGroupGetBits(evStartStopEvents);
						sprintf(NikalanthaString, " ");
						sprintf(TimeStringN, " ");
						vTaskSuspend(NikalanthaTask);
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
//							Buttontask									//
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

int vLeibniz(void *pvParameter){
	(void) pvParameter;
	float32_t PI;
	float32_t Summe;
	xEventGroupWaitBits(evStartStopEvents, EV_START_LEIBNIZ, pdFALSE, pdTRUE, portMAX_DELAY);
	TickType_t Endtime;
	start_here:
	PI = 0;
	Summe = 0.0;
	TickType_t Starttime = 0;
	uint32_t i = -1;
	uint32_t n = 1;																// Maximale Anzahl Leibniz-berechnungen
	uint64_t Ticks = 0;
		for(i = 0; i < n; i ++){
		if(Bits & EV_STOP_LEIBNIZ){	//Stop function
		    xEventGroupSetBits(evStartStopEvents, EV_STOPPED_LEIBNIZ);
			Bits = xEventGroupGetBits(evStartStopEvents);
			Bits &= (~EV_START_LEIBNIZ & ~EV_STOP_LEIBNIZ) ;  // EV_START_LEIBNIZ löschen
            vTaskSuspend(NULL);
		}
		else if (Bits & EV_RESET_LEIBNIZ){	//Reset function
			xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
            xEventGroupSetBits(evStartStopEvents, EV_START_LEIBNIZ);
			Bits = xEventGroupGetBits(evStartStopEvents);
			goto start_here;
		}
		Starttime  = xTaskGetTickCount();
		Summe += pow(-1, i) / (2 * i + 1);
		PI = 4 * Summe;
		n ++;
		sprintf(&LeibnizString[0], "PI ist %.7f", PI);		// Ganzzahl und Kommastellen in String einlesen	
		Endtime = xTaskGetTickCount() - Starttime;
		Ticks += Endtime;
		sprintf(&TimeStringL[0], "%d s", (Ticks / 1000));
		if((uint32_t)((PI*100000) > 314159 && (PI*100000) < 314160)){
			sprintf(&LeibnizString[0], "PI ist %.7f", PI);
			xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
			Bits = xEventGroupSetBits(evStartStopEvents, EV_STOPPED_LEIBNIZ);
			vTaskSuspend(NULL);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//						Nilakantha-Folge-Task							//
//////////////////////////////////////////////////////////////////////////

void vNikalantha(void *pvParameter){
	(void) pvParameter;
	TickType_t Starttime;
	TickType_t Endtime;
	double ZaehlerN = -1;
	
	xEventGroupWaitBits(evStartStopEvents, EV_START_NIKA, pdFALSE, pdTRUE, portMAX_DELAY);
	EventBits_t bitsToWait = EV_START_NIKA | EV_STOP_NIKA | EV_RESET_NIKA;
    while (1) {
        bitsToWait &= ~EV_RESET_NIKA;  // EV_RESET_NIKA löschen

        start_here:
		xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
        Bits = xEventGroupSetBits(evStartStopEvents, EV_START_NIKA);
		uint64_t Ticks = 0;
        float32_t PINika = 0;
        long int iN = 0;
		long int nN = 2;
		ZaehlerN = 0;

        if (iN == 0) {
            // Startwert PI = 3
            PINika = 3;
            ZaehlerN = pow(-1, iN);
            PINika = PINika + (ZaehlerN * 4 / (nN * (nN + 1) * (nN + 2)));
            nN += 2;
            iN += 1;
        }

        for (iN = 1; iN < nN; iN++) {
            if (Bits & EV_STOP_NIKA) {
				xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
                xEventGroupSetBits(evStartStopEvents, EV_STOPPED_NIKA);
                vTaskSuspend(NULL);
            }
            if (Bits & EV_RESET_NIKA) {
                xEventGroupSetBits(evStartStopEvents, EV_START_NIKA);
                Bits &= ~EV_RESET_NIKA;  // EV_RESET_NIKA löschen
                goto start_here;
            }
		Starttime  = xTaskGetTickCount();
		ZaehlerN = pow(-1, iN);	
		PINika = PINika +(ZaehlerN * 4 / (nN * (nN + 1) * (nN + 2)));
		nN += 2;
		int PIint1 = PINika;														// Ganzzahl ermitteln
		float PIkomma1 = PINika - PIint1;											// Die ersten vier Kommastellen ermitteln als float
		int PIint2 = PIkomma1 * 10000;												// Die ersten vier Kommastellen ermitteln als int
		float PIkomma2 = PIkomma1 * 10000;											// Die zweiten vier Kommastellen ermitteln als float
		float PIkomma3 = PIkomma2 - PIint2;											// Die zweiten vier Kommastellen ermitteln als float
		int PIint3 = PIkomma3 * 10000;												// Die zweiten vier Kommastellen ermitteln als int
		float PIkomma4 = PIkomma3 * 10000;											// Die dritten vier Kommastellen ermitteln als float
		float PIkomma5 = PIkomma4 - PIint3;											// Die dritten vier Kommastellen ermitteln als float
		int PIint4 = PIkomma5 * 10000;												// Die dritten vier Kommastellen ermitteln als int
		Endtime = xTaskGetTickCount() - Starttime;
		sprintf(NikalanthaString, "PI ist %d.%d%d%d", PIint1, PIint2, PIint3, PIint4);	// Ganzzahl und Kommastellen in String einlesen
		Ticks += Endtime;
		sprintf(TimeStringN, "%d ms", Ticks);
		}
	}	
}