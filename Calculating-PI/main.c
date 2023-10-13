/*
 * Calculating_PI.c
 *
 * Created: 03.10.2023 18:45:07
 * Author : Borbak820
 */ 

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


extern void vApplicationIdleHook(void);
int vLeibniz(void *pvParameter);
void vNikalantha(void *pvParameter);
void vUserInterface(void * pvParameter);
void vButtonHandler(void* pvParameter);

#define evStartLeibniz		1<<0	//1
#define evStopLeibniz		1<<1	//2
#define evStartNika			1<<2	//4
#define evStopNika			1<<3	//8
#define evStoppedLeibniz	1<<4	//16
#define evStoppedNika		1<<5	//32
#define evResetLeibniz		1<<6	//64
#define evResetNika			1<<7	//128
#define evClear				0xFF
EventGroupHandle_t evStartStopEvents;

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

TaskHandle_t LeibnizTask;
TaskHandle_t NikalanthaTask;
TaskHandle_t ButtonTask;

TickType_t Starttime;
TickType_t Endtime;
TickType_t Time;

char NikalanthaString[20];
char LeibnizString[20];
char TimeStringL[20];
char TimeStringN[20];

int Menu = 0;
double PILeibniz;
long int iL = 0;
long int nL = 1000000;																// Maximale Anzahl Leibniz-berechnungen
double Summe = 0.0;
double ZaehlerL = 0;
double Nenner = 0;
double PINika;
long int iN = 0;
long int nN = 2;
double ZaehlerN = -1;
long int TicksL = 0;
long int TicksN = 0;
uint32_t EventState;
//PI = 3.14159265358979323846264338327950288419716939937510

void vApplicationIdleHook(void){
	
}


int main(void){
	vInitClock();
	vInitDisplay();

	evStartStopEvents = xEventGroupCreate();
	evButtonEvents = xEventGroupCreate();

	xTaskCreate(vUserInterface, (const char *) "control_tsk", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate(vButtonHandler, (const char*) "ButtonTask", configMINIMAL_STACK_SIZE+30, NULL, 2, &ButtonTask);
	xTaskCreate(vLeibniz, (const char *) "Leibniz-Folge-Task", configMINIMAL_STACK_SIZE+100, NULL, 1, &LeibnizTask);
	xTaskCreate(vNikalantha, (const char *) "Nikalantha-Folge-Task", configMINIMAL_STACK_SIZE+100, NULL, 1, &NikalanthaTask);

	vTaskSuspend(LeibnizTask);
	vTaskSuspend(NikalanthaTask);
	
	vTaskStartScheduler();
	
	return 0;
}


//Controller-Task
void vUserInterface(void* pvParameters){
	vDisplayClear();
	vTaskDelay(500);
	initButtons();
	uint32_t DisplayUpdateCounter = 50;
	for(;;){
		uint32_t ButtonState = (xEventGroupGetBits(evButtonEvents)) & 0x000000FF; //Read Button States from EventGroup
		xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR); //As the Button State is saved now, we can clear the EventGroup for new Button presses
		uint32_t EventState = (xEventGroupGetBits(evStartStopEvents));
		//User Interface
		if (DisplayUpdateCounter == 0){
			if(Menu == 0){
				//Start screen
				vDisplayClear();
				vDisplayWriteStringAtPos(0,0, "Pi-Calculator");
				vDisplayWriteStringAtPos(1,0, "1: Pi aus math.h");
				vDisplayWriteStringAtPos(2,0, "2: Leibniz-Serie");
				vDisplayWriteStringAtPos(3,0, "3: Nikalantha-Serie");
			}
			if(Menu == 1){
				//Pi Demo screen
				char pistring[12];
				vDisplayClear();
				sprintf(&pistring[0], "PI: %.8f", M_PI);
				vDisplayWriteStringAtPos(0,0, "Pi aus math.h");
				vDisplayWriteStringAtPos(1,0, "%s", pistring);
				vDisplayWriteStringAtPos(3,0, "4: Back");
			}
			if(Menu == 2){
				//Leibniz's Pi screen
				vDisplayClear();
				vDisplayWriteStringAtPos(0,0, "%s", LeibnizString);
				vDisplayWriteStringAtPos(1,0, "1: Start");
				vDisplayWriteStringAtPos(1,10, "2: Stop");
				vDisplayWriteStringAtPos(2,0, "3: Reset");
				vDisplayWriteStringAtPos(2,10, "%s", TimeStringL);
				vDisplayWriteStringAtPos(3,0, "4: --> Nikalantha");
			}
			if(Menu == 3){
				//Nikalantha's Pi screen
				vDisplayClear();
				vDisplayWriteStringAtPos(0,0, "%s", NikalanthaString);
				vDisplayWriteStringAtPos(1,0, "1: Start");
				vDisplayWriteStringAtPos(1,10, "2: Stop");
				vDisplayWriteStringAtPos(2,0, "3: Reset");
				vDisplayWriteStringAtPos(2,10, "%s", TimeStringN);
				vDisplayWriteStringAtPos(3,0, "4: --> Leibniz");
			}
			if(Menu == 4){
				//Easteregg screen (it's funny if you know it)
				vDisplayClear();
				vDisplayWriteStringAtPos(1,0, "The cake is a lie!");
				vDisplayWriteStringAtPos(2,0, "- 'GLaDOS'");
			}
		//Refreshing Time (50*10ms=500ms)	
			DisplayUpdateCounter = 50;
		}
		else{
			DisplayUpdateCounter --;
			}
		
		//Buttons Start screen	
		if(Menu == 0 && ButtonState == 1){
			//Pi Demo screen
			Menu = 1;
		}
		if(Menu == 0 && ButtonState == 2){
			//Leibniz's Pi screen
			Menu = 2;
		}
		if(Menu == 0 && ButtonState == 4){
			//Nikalantha's Pi screen
			Menu = 3;
		}
		
		//Buttons Pi Demo screen
		if(Menu == 1 && ButtonState == 8){
			//leave Demo - go to start screen
			Menu = 0;
		}
		
		//Buttons Leibniz's Pi screen		
		if(Menu == 2 && ButtonState == 1 && EventState != 1){
			//start Leibniz calculation
			vTaskResume(LeibnizTask);
			xEventGroupClearBits(evStartStopEvents, EVBUTTONS_CLEAR);
			EventState = xEventGroupSetBits(evStartStopEvents, evStartLeibniz);
			xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR);
			ButtonState =  xEventGroupGetBits(evButtonEvents);
			vTaskResume(LeibnizTask);
		}
		if(Menu == 2 && ButtonState == 2 && EventState == 1){
			//stop Leibniz calculation
			xEventGroupClearBits(evStartStopEvents, EVBUTTONS_CLEAR);
			xEventGroupSetBits(evStartStopEvents, evStopLeibniz);
			EventState = xEventGroupGetBits(evStartStopEvents);
			xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR);
			ButtonState =  xEventGroupGetBits(evButtonEvents);
		}
		if(Menu == 2 && ButtonState == 4 && (EventState == 1 || EventState == 18)){
			//reset - set Leibniz calculation to 0
			xEventGroupClearBits(evStartStopEvents, evClear);
			EventState = xEventGroupSetBits(evStartStopEvents, evStopLeibniz);
			PILeibniz = 0;
			iL = -1;
			Summe = 0.0;
			ZaehlerL = 0;
			Nenner = 0;
			TicksL = 0;
			sprintf(LeibnizString, " ");
			sprintf(TimeStringL, " ");
			xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR);
			ButtonState =  xEventGroupGetBits(evButtonEvents);
		}
		if(Menu == 2 && ButtonState == 8){
			//stop Leibniz calculation and switch to Nikalantha calculation
			vTaskSuspend(LeibnizTask);
			Menu = 3;
			EventState = xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR);
			ButtonState =  xEventGroupGetBits(evButtonEvents);
		}
		
		//Buttons Nikalantha's Pi screen
		if(Menu == 3 && ButtonState == 1 && EventState !=4){
			//start Nikalantha calculation
			vTaskResume(NikalanthaTask);
			xEventGroupClearBits(evStartStopEvents, EVBUTTONS_CLEAR);
			EventState = xEventGroupSetBits(evStartStopEvents, evStartNika);
			xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR);
			ButtonState =  xEventGroupGetBits(evButtonEvents);
		}
		if(Menu == 3 && ButtonState == 2 && EventState == 4){
			//stop Nikalantha calculation
			xEventGroupClearBits(evStartStopEvents, EVBUTTONS_CLEAR);
			xEventGroupSetBits(evStartStopEvents, evStopNika);
			EventState = xEventGroupGetBits(evStartStopEvents);
			xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR);
			ButtonState =  xEventGroupGetBits(evButtonEvents);
		}
		if(Menu == 3 && ButtonState == 4 && (EventState == 4 || EventState == 40)){
			//reset - set Nikalantha calculation to 0
			xEventGroupClearBits(evStartStopEvents, evClear);
			EventState = xEventGroupSetBits(evStartStopEvents, evStopNika);
			PINika = 0;
			iN = 0;
			ZaehlerN = 0;
			nN = 0;
			sprintf(NikalanthaString, " ");
			sprintf(TimeStringN, " ");
			xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR);
			ButtonState =  xEventGroupGetBits(evButtonEvents);
		}
		if(Menu == 3 && ButtonState == 8) {
			//stop Nikalantha calculation and switch to Leibniz calculation
			vTaskSuspend(NikalanthaTask);
			Menu = 2;
			EventState = xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR);
			ButtonState =  xEventGroupGetBits(evButtonEvents);
		}
		
		//Buttons Easteregg screen
		if(Menu == 0 && ButtonState == 192){
			//Enter testchamber
			Menu = 4;
		}
		if(Menu == 4 && ButtonState == 8){
			//leave Aperture Laboratories - go to start screen
			Menu = 0;
		}
		if(EventState == 16){
			xEventGroupClearBits(evStartStopEvents, evClear);
			EventState = xEventGroupSetBits(evStartStopEvents, evResetLeibniz);
			vTaskSuspend(LeibnizTask);
			Menu = 2;
		}
		if(EventState == 32 && Menu == 3){
			xEventGroupClearBits(evStartStopEvents, evClear);
			xEventGroupSetBits(evStartStopEvents, evStopNika);
			xEventGroupSetBits(evStartStopEvents, evStoppedNika);
			EventState = xEventGroupGetBits(evStartStopEvents);
			xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR);
			ButtonState = xEventGroupGetBits(evButtonEvents);
			vTaskSuspend(NikalanthaTask);
			Menu = 3;
		}		
		vTaskDelay(10/portTICK_RATE_MS);
	}	
}

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

//Leibniz-Folge-Task	PI = 4·(1 - (1/3) + (1/5) - (1/7) + (1/9) - (1/11) + (1/13)...)
int vLeibniz(void *pvParameter){
	(void) pvParameter;
	for(iL = 0; iL < nL; iL ++){EventState = xEventGroupGetBits(evStartStopEvents);
		if(EventState == 2){
			while(EventState == 2){
				xEventGroupClearBits(evStartStopEvents, evClear);
				xEventGroupSetBits(evStartStopEvents, evStoppedNika);
				EventState = xEventGroupGetBits(evStartStopEvents);
			}
		}
		Starttime  = xTaskGetTickCount();
		ZaehlerL = pow(-1, iL);															// pow Toggelt zwischen -1 und +1 für Vorzeichen --> -1^i
		Nenner = 2*iL+1;
		Summe += (ZaehlerL / Nenner);
		PILeibniz = 4 * Summe;
		uint32_t x =  PILeibniz*100000;
		int PIint1 = PILeibniz;															// Ganzzahl ermitteln
		float PIkomma1 = PILeibniz - PIint1;											// Die ersten vier Kommastellen ermitteln als float
		int PIint2 = PIkomma1 * 10000;													// Die ersten vier Kommastellen ermitteln als int
		float PIkomma2 = PIkomma1 * 10000;												// Die zweiten vier Kommastellen ermitteln als float
		float PIkomma3 = PIkomma2 - PIint2;												// Die zweiten vier Kommastellen ermitteln als float
		int PIint3 = PIkomma3 * 10000;													// Die zweiten vier Kommastellen ermitteln als int
		float PIkomma4 = PIkomma3 * 10000;												// Die dritten vier Kommastellen ermitteln als float
		float PIkomma5 = PIkomma4 - PIint3;												// Die dritten vier Kommastellen ermitteln als float
		int PIint4 = PIkomma5 * 10000;													// Die dritten vier Kommastellen ermitteln als int
		sprintf(LeibnizString, "PI ist %d.%d%d%d", PIint1, PIint2, PIint3, PIint4);		// Ganzzahl und Kommastellen in String einlesen	
		Endtime = xTaskGetTickCount() - Starttime;
		TicksL += Endtime;
		sprintf(TimeStringL, "%d ms", TicksL);
		if((uint32_t)((PILeibniz*100000) > 314159 && (PILeibniz*100000) < 314160)){
			Time = xTaskGetTickCount() - Starttime;
			vTaskDelay(portMAX_DELAY);
		}
	}
}


//Nilakantha-Folge-Task		PI=3 + 4/(2·3·4) - 4/(4·5·6) + 4/(6·7·8) - 4/(8·9·10) + 4/(10·11·12) - 4/(12·13·14)...
void vNikalantha(void *pvParameter){
	(void) pvParameter;
	nN = 2;
	if (iN == 0){																	// Startwert PI = 3
		PINika = 3;
		ZaehlerN = pow(-1, iN);														//pow Toggelt zwischen -1 und +1 für Vorzeichen --> -1^i
		PINika = PINika +(ZaehlerN * 4 / (nN * (nN + 1) * (nN + 2)));
		nN += 2;
		iN += 1;
	}
	for (iN = 1; iN < nN; iN++){
		EventState = xEventGroupGetBits(evStartStopEvents);
		if(EventState == 8){
			while(EventState == 8){
				xEventGroupClearBits(evStartStopEvents, evClear);
				xEventGroupSetBits(evStartStopEvents, evStoppedNika);
				EventState = xEventGroupGetBits(evStartStopEvents);
			}
		}
		Starttime = xTaskGetTickCount();
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
		sprintf(NikalanthaString, "PI ist %d.%d%d%d", PIint1, PIint2, PIint3, PIint4);	// Ganzzahl und Kommastellen in String einlesen
		Endtime = xTaskGetTickCount() - Starttime;
		TicksN += Endtime;
		sprintf(TimeStringN, "%d ms", TicksN);
	}	
}