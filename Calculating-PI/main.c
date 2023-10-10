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
void vButton(void * pvParameter);

TaskHandle_t LeibnizTask;
TaskHandle_t NikalanthaTask;
TaskHandle_t ButtonTask;


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
//PI = 3.14159265358979323846264338327950288419716939937510

void vApplicationIdleHook(void)
{	
	
}


int main(void)
{
	vInitClock();
	vInitDisplay();

	evButtonEvents = xEventGroupCreate();

	xTaskCreate(vButton, (const char *) "control_tsk", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate(vLeibniz, (const char *) "Leibniz-Folge-Task", configMINIMAL_STACK_SIZE+100, NULL, 1, &LeibnizTask);
	xTaskCreate(vNikalantha, (const char *) "Nikalantha-Folge-Task", configMINIMAL_STACK_SIZE+100, NULL, 1, &NikalanthaTask);

	vTaskSuspend(LeibnizTask);
	vTaskSuspend(NikalanthaTask);
	
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
		vTaskDelay(10/portTICK_RATE_MS);
	}		
}


//Leibniz-Folge-Task	PI = 4·(1 - (1/3) + (1/5) - (1/7) + (1/9) - (1/11) + (1/13)...)
void vLeibniz(void *pvParameter){
	(void) pvParameter;
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
		}
	}
}


//Nilakantha-Folge-Task		PI=3 + 4/(2·3·4) - 4/(4·5·6) + 4/(6·7·8) - 4/(8·9·10) + 4/(10·11·12) - 4/(12·13·14)...
void vNikalantha(void *pvParameter){
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
		vDisplayWriteStringAtPos(0,0, "%s", NikalanthaString);
		if((uint32_t)(PINika*100000 == 314159)){
			TickType_t Time = xTaskGetTickCount() - starttime;
			vTaskDelay(portMAX_DELAY);
		}
	}	
}