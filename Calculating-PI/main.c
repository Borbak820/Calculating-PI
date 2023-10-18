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
void vLeibniz(void *pvParameter);
void vNikalantha(void *pvParameter);
<<<<<<< Updated upstream
void vButton(void * pvParameter);
=======
void vUserInterface(void * pvParameter);
void vButtonHandler(void* pvParameter);

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
>>>>>>> Stashed changes

TaskHandle_t LeibnizTask;
TaskHandle_t NikalanthaTask;
TaskHandle_t ButtonTask;

<<<<<<< Updated upstream

#define EVBUTTONS_S1	1<<0
#define EVBUTTONS_S2	1<<1
#define EVBUTTONS_S3	1<<2
#define EVBUTTONS_S4	1<<3
#define EVBUTTONS_L1	1<<4
#define EVBUTTONS_L2	1<<5
#define EVBUTTONS_L3	1<<6
#define EVBUTTONS_L4	1<<7
#define EVBUTTONS_CLEAR	0xFF
EventGroupHandle_t evButtonEvents;
=======
TickType_t StarttimeL;
TickType_t EndtimeL;
TickType_t TimeL;
TickType_t StarttimeN;
TickType_t EndtimeN;
TickType_t TimeN;

EventBits_t Bits;
>>>>>>> Stashed changes

char NikalanthaString[20];
char LeibnizString[20];

int Menu = 0;
int SubMenu = 0;
double PILeibniz;
long int iL = 0;
long int nL = 1000000;																// Maximale Anzahl berechnungen
double Summe = 0.0;
double ZaehlerL = 0;
double Nenner = 0;
double PINika;
long int iN = 0;
long int nN = 2;
double ZaehlerN = -1;
<<<<<<< Updated upstream
=======
uint64_t TicksL = 0;
uint64_t TicksN = 0;
uint32_t EventState;
>>>>>>> Stashed changes
//PI = 3.14159265358979323846264338327950288419716939937510

void vApplicationIdleHook(void)
{	
	
}


int main(void)
{
	vInitClock();
	vInitDisplay();

	evButtonEvents = xEventGroupCreate();

<<<<<<< Updated upstream
	xTaskCreate(vButton, (const char *) "control_tsk", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate(vLeibniz, (const char *) "Leibniz-Folge-Task", configMINIMAL_STACK_SIZE+100, NULL, 1, &LeibnizTask);
=======
	xTaskCreate(vUserInterface, (const char *) "control_tsk", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate(vButtonHandler, (const char*) "ButtonTask", configMINIMAL_STACK_SIZE+30, NULL, 2, &ButtonTask);
	xTaskCreate(vLeibniz, (const char *) "Leibniz-Folge-Task", configMINIMAL_STACK_SIZE+200, NULL, 1, &LeibnizTask);
>>>>>>> Stashed changes
	xTaskCreate(vNikalantha, (const char *) "Nikalantha-Folge-Task", configMINIMAL_STACK_SIZE+100, NULL, 1, &NikalanthaTask);

	vTaskStartScheduler();
	
	return 0;
}



//Controller-Task
void vButton(void* pvParameters){
	vDisplayClear();
	vTaskDelay(500);
	initButtons();
	uint32_t DisplayUpdateCounter = 50;
	for(;;){
		updateButtons();
		
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
				SubMenu = 1;
			}
			if(Menu == 2){
				//Leibniz's Pi screen
				vDisplayClear();
				vDisplayWriteStringAtPos(0,0, "%s", LeibnizString);
				vDisplayWriteStringAtPos(1,0, "1: Start");
				vDisplayWriteStringAtPos(1,10, "2: Stop");
				vDisplayWriteStringAtPos(2,0, "3: Reset");
				vDisplayWriteStringAtPos(3,0, "4: --> Nikalantha");
				SubMenu = 2;
			}
			if(Menu == 3){
				//Nikalantha's Pi screen
				vDisplayClear();
				vDisplayWriteStringAtPos(0,0, "%s", NikalanthaString);
				vDisplayWriteStringAtPos(1,0, "1: Start");
				vDisplayWriteStringAtPos(1,10, "2: Stop");
				vDisplayWriteStringAtPos(2,0, "3: Reset");
				vDisplayWriteStringAtPos(3,0, "4: --> Leibniz");
				SubMenu = 3;
			}
			if(Menu == 4){
				//Easteregg screen (it's funny if you know it)
				vDisplayClear();
				vDisplayWriteStringAtPos(1,0, "The cake is a lie!");
				vDisplayWriteStringAtPos(2,0, "- 'GLaDOS'");
				SubMenu = 4;
			}
		//Refreshing Time (50*10ms=500ms)	
			DisplayUpdateCounter = 50;
		}
		else{
			DisplayUpdateCounter --;
		}
	
//Menuauswahl	
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
<<<<<<< Updated upstream
		
		//Buttons Start screen	
		if(Menu == 0 && getButtonPress(BUTTON1) == SHORT_PRESSED){
			//Pi Demo screen
			Menu = 1;
			SubMenu = 1;
		}
		if(Menu == 0 && getButtonPress(BUTTON2) == SHORT_PRESSED){
			//Leibniz's Pi screen
			Menu = 2;
			SubMenu = 2;
		}
		if(Menu == 0 && getButtonPress(BUTTON3) == SHORT_PRESSED){
			//Nikalantha's Pi screen
			Menu = 3;
			SubMenu = 3;
		}
		
		//Buttons Pi Demo screen
		if(Menu == 1 && SubMenu == 1 && getButtonPress(BUTTON4) == SHORT_PRESSED){
			//leave Demo - go to start screen
			Menu = 0;
		}
		
		//Buttons Leibniz's Pi screen		
		if(Menu == 2 && SubMenu == 2 && getButtonPress(BUTTON1) == SHORT_PRESSED){
			//start Leibniz calculation
			vTaskResume(LeibnizTask);
		}
		if(Menu == 2 && SubMenu == 2 && getButtonPress(BUTTON2) == SHORT_PRESSED){
			//stop Leibniz calculation
			vTaskSuspend(LeibnizTask);
		}
		if(Menu == 2 && SubMenu == 2 && getButtonPress(BUTTON3) == SHORT_PRESSED){
			//reset - set Leibniz calculation to 0
			vTaskSuspend(LeibnizTask);
			PILeibniz = 0;
			iL = -1;
			Summe = 0.0;
			ZaehlerL = 0;
			Nenner = 0;
			sprintf(LeibnizString, " ");
		}
		if(Menu == 2 && SubMenu == 2 && getButtonPress(BUTTON4) == SHORT_PRESSED){
			//stop Leibniz calculation and switch to Nikalantha calculation
			vTaskSuspend(LeibnizTask);
			Menu = 3;
		}
		
		//Buttons Nikalantha's Pi screen
		if(Menu == 3 && SubMenu == 3 && getButtonPress(BUTTON1) == SHORT_PRESSED){
			//start Nikalantha calculation
			vTaskResume(NikalanthaTask);
		}
		if(Menu == 3 && SubMenu == 3 && getButtonPress(BUTTON2) == SHORT_PRESSED){
			//stop Nikalantha calculation
			vTaskSuspend(NikalanthaTask);
		}
		if(Menu == 3 && SubMenu == 3 && getButtonPress(BUTTON3) == SHORT_PRESSED){
			//reset - set Nikalantha calculation to 0
			vTaskSuspend(NikalanthaTask);
			PINika = 0;
			iN = 0;
			ZaehlerN = 0;
			nN = 0;
			sprintf(NikalanthaString, " ");
		}
		if(Menu == 3 && SubMenu == 3 && getButtonPress(BUTTON4) == SHORT_PRESSED) {
			//stop Nikalantha calculation and switch to Leibniz calculation
			vTaskSuspend(NikalanthaTask);
			Menu = 2;
		}
		
		//Buttons Easteregg screen
		if(getButtonPress(BUTTON3) == LONG_PRESSED && getButtonPress(BUTTON4) == LONG_PRESSED){
			//Enter testchamber
			Menu = 4;
		}
		if(Menu == 4 && getButtonPress(BUTTON4) == SHORT_PRESSED){
			//leave Aperture Laboratories - go to start screen
			Menu = 0;
		}
=======
		}

// Pi Demo functions
		if (Menu == 1){
			if (ButtonState == 8) {
			// Demo verlassen - zum Startbildschirm wechseln
					Menu = 0;
			}
		}
		
//Leibniz's Pi functions	
		if(Menu == 2) {
			switch (ButtonState) {
				case 1:
				if ((Bits == 0) || EV_STOP_NIKA) {
					// Start Leibniz calculation
					xEventGroupSetBits(evStartStopEvents, EV_START_LEIBNIZ);
				} 
				else if (Bits & (EV_STOP_LEIBNIZ | EV_STOPPED_LEIBNIZ)){
					// Restart Leibniz calculation if stopped or resetted
					xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
					xEventGroupSetBits(evStartStopEvents, EV_START_LEIBNIZ);
					Bits = xEventGroupGetBits(evStartStopEvents);
					vTaskResume(LeibnizTask);
				}
				break;

				case 2:
				if(Bits & (EV_START_LEIBNIZ | EV_STOPPED_NIKA)){
					// Stop Leibniz calculation
					xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
					xEventGroupSetBits(evStartStopEvents, EV_STOP_LEIBNIZ);
					Bits = xEventGroupGetBits(evStartStopEvents);
				}
				break;

				case 4:
				if (Bits & (EV_START_LEIBNIZ | EV_STOP_LEIBNIZ)) {
					// Reset - set Leibniz calculation to 0
					xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
					xEventGroupSetBits(evStartStopEvents, EV_RESET_LEIBNIZ);
					Bits = xEventGroupGetBits(evStartStopEvents);
					PILeibniz = 0;
					iL = -1;
					Summe = 0.0;
					ZaehlerL = 0;
					Nenner = 0;
					TicksL = 0;
					sprintf(LeibnizString, " ");
					sprintf(TimeStringL, " ");
				}
				break;

				case 8:
					// Stop Leibniz calculation and switch to Nikalantha calculation
					xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
					xEventGroupSetBits(evStartStopEvents, EV_STOP_LEIBNIZ);
					Bits = xEventGroupGetBits(evStartStopEvents);
					vTaskSuspend(LeibnizTask);
					Menu = 3;
					ButtonState = xEventGroupClearBits(evButtonEvents, EVBUTTONS_CLEAR);
				break;

				default:
				// Hier können Sie ein Standardverhalten für andere ButtonState-Werte im Leibniz-Menü festlegen
				break;
			}
		}
		
//Nikalantha's Pi functions
		if (Menu == 3) {
			switch (ButtonState) {
				case 1:
					if ((Bits == 0) | EV_STOP_LEIBNIZ) {
						// Start Nikalantha calculation
						xEventGroupSetBits(evStartStopEvents, EV_START_NIKA);
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
		
//Easteregg-Functions
		switch (Menu) {
			case 0:
			if (ButtonState == 192) {
				// Enter testchamber
				Menu = 4;
			}
			break;

			case 4:
			if (ButtonState == 8) {
				// Leave Aperture Laboratories - go to start screen
				Menu = 0;
			}
		}
>>>>>>> Stashed changes
		vTaskDelay(10/portTICK_RATE_MS);
	}		
}


//Leibniz-Folge-Task	PI = 4·(1 - (1/3) + (1/5) - (1/7) + (1/9) - (1/11) + (1/13)...)
void vLeibniz(void *pvParameter){
	(void) pvParameter;
<<<<<<< Updated upstream
	TickType_t starttime = xTaskGetTickCount();
	for(iL = 0; iL < nL; iL ++){	
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
		if((uint32_t)((PILeibniz*100000) > 314159 && (PILeibniz*100000) < 314160)){
			TickType_t Time = xTaskGetTickCount() - starttime;
			vTaskDelay(portMAX_DELAY);
=======
	Bits = xEventGroupWaitBits(evStartStopEvents, EV_START_LEIBNIZ, pdFALSE, pdTRUE, portMAX_DELAY);
		for(iL = 0; iL < nL; iL ++){
			//Funktion wird gestoppt
			if(Bits & EV_STOP_LEIBNIZ){
                xEventGroupSetBits(evStartStopEvents, EV_STOPPED_LEIBNIZ);
                vTaskSuspend(NULL);
            }
			StarttimeL  = xTaskGetTickCount();
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
			EndtimeL = xTaskGetTickCount() - StarttimeL;
			TicksL += EndtimeL;
			sprintf(TimeStringL, "%d ms", TicksL);
			if((uint32_t)((PILeibniz*100000) > 314159 && (PILeibniz*100000) < 314160)){
				TimeL = xTaskGetTickCount() - StarttimeL;
				vTaskDelay(portMAX_DELAY);
			}
>>>>>>> Stashed changes
		}
	}

//Nilakantha-Folge-Task		PI=3 + 4/(2·3·4) - 4/(4·5·6) + 4/(6·7·8) - 4/(8·9·10) + 4/(10·11·12) - 4/(12·13·14)...
void vNikalantha(void *pvParameter){
<<<<<<< Updated upstream
	(void) pvParameter;
	TickType_t starttime = xTaskGetTickCount();
	if (iN == 0){																	// Startwert PI = 3
		PINika = 3;
	}
	for (iN = 0; iN < nN; iN++){
		ZaehlerN = pow(-1, iN);														//pow Toggelt zwischen -1 und +1 für Vorzeichen --> -1^i
		PINika = PINika +(ZaehlerN * 4 / (nN * (nN + 1) * (nN + 2)));
		if (iN == 0){																// Einstellung für den ersten Vorzeichen Toggle
			ZaehlerN = -1;
		}		
=======
	(void) pvParameter; 
	xEventGroupWaitBits(evStartStopEvents, EV_START_NIKA, pdFALSE, pdTRUE, portMAX_DELAY);
	EventBits_t bitsToWait = EV_START_NIKA | EV_STOP_NIKA | EV_RESET_NIKA;

    while (1) {
        bitsToWait &= ~EV_RESET_NIKA;  // EV_RESET_NIKA löschen

        start_here:
		xEventGroupClearBits(evStartStopEvents, EV_CLEAR);
		
        Bits = xEventGroupSetBits(evStartStopEvents, EV_START_NIKA);
        PINika = 0;
        iN = 0;
        ZaehlerN = 0;
        TicksN = 0;
        nN = 2;

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
                xEventGroupSetBits(evStartStopEvents, EV_STOPPED_NIKA);
                vTaskSuspend(NULL);
            }
            if (Bits & EV_RESET_NIKA) {
                xEventGroupSetBits(evStartStopEvents, EV_START_NIKA);
                Bits &= ~EV_RESET_NIKA;  // EV_RESET_NIKA löschen
                goto start_here;
            }
		StarttimeN  = xTaskGetTickCount();
		ZaehlerN = pow(-1, iN);	
		PINika = PINika +(ZaehlerN * 4 / (nN * (nN + 1) * (nN + 2)));
>>>>>>> Stashed changes
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
		EndtimeN = xTaskGetTickCount() - StarttimeN;
		sprintf(NikalanthaString, "PI ist %d.%d%d%d", PIint1, PIint2, PIint3, PIint4);	// Ganzzahl und Kommastellen in String einlesen
<<<<<<< Updated upstream
		vDisplayWriteStringAtPos(0,0, "%s", NikalanthaString);
		if((uint32_t)(PINika*100000 == 314159)){
			TickType_t Time = xTaskGetTickCount() - starttime;
			vTaskDelay(portMAX_DELAY);
=======
		TicksN += EndtimeN;
		sprintf(TimeStringN, "%d ms", TicksN);
>>>>>>> Stashed changes
		}
	}	
}