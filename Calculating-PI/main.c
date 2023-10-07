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
#include "math.h"
#include "stdio.h"

#include "avr_f64.h"
#include "ButtonHandler.h"


extern void vApplicationIdleHook(void);
void vInterface(void *pvParameter);
void vLeibniz(void *pvParameter);
void vNikalantha(void *pvParameter);
void vButton(void * pvParameter);

TaskHandle_t LeibnizTask;
TaskHandle_t NikalanthaTask;
TaskHandle_t InterfaceTask;


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

double PILeibniz;
double PINika;
int Menu = 0;
//PI = 3.14159265358979323846264338327950288419716939937510

void vApplicationIdleHook(void)
{	
	
}


int main(void)
{
	vInitClock();
	vInitDisplay();

	evButtonEvents = xEventGroupCreate();

	xTaskCreate(vInterface, (const char*) "Interface-Task", configMINIMAL_STACK_SIZE+300, NULL, 2, &InterfaceTask);
	xTaskCreate(vButton, (const char *) "control_tsk", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate(vLeibniz, (const char *) "Leibniz-Folge-Task", configMINIMAL_STACK_SIZE+100, NULL, 1, &LeibnizTask);
	xTaskCreate(vNikalantha, (const char *) "Nikalantha-Folge-Task", configMINIMAL_STACK_SIZE+100, NULL, 1, &NikalanthaTask);
	vTaskSuspend(LeibnizTask);
	vTaskSuspend(NikalanthaTask);

	vTaskStartScheduler();
	return 0;
}

	
//Interface-Task
void vInterface(void *pvParameter){
	(void) pvParameter;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xFrequency = 500;													//Zeitparameter für Interface-Task-Delay
	for(;;){
		vDisplayClear();			/*													// Löschen des ganzen Displays
		vDisplayWriteStringAtPos(0,0,"Nikalantha-Serie");								// Ausgabe auf das Display
		vDisplayWriteStringAtPos(1,0,"Pi ist %s", NikalanthaString);					// Nikalantha's PI	
		vDisplayWriteStringAtPos(2,0,"Leibniz-Serie");
		vDisplayWriteStringAtPos(3,0,"Pi ist %s", LeibnizString);			*/			// Leibniz's PI
		vDisplayWriteStringAtPos(0,0, "Pi-Calculator");
		vDisplayWriteStringAtPos(1,0, "1: Pi aus math.h");
		vDisplayWriteStringAtPos(2,0, "2: Leibniz-Serie");
		vDisplayWriteStringAtPos(3,0, "3: Nikalantha-Serie");
		vTaskDelayUntil(&xLastWakeTime, xFrequency/portTICK_RATE_MS);					// TaskDelay													
	}
}

//Controller-Task
void vButton(void* pvParameters){
	initButtons();
	for(;;) {
		updateButtons();
		if(getButtonPress(BUTTON1) == SHORT_PRESSED){
			vDisplayClear();
			char pistring[12];
			sprintf(&pistring[0], "PI: %.8f", M_PI);
			vDisplayWriteStringAtPos(0,0, "Pi aus math.h");
			vDisplayWriteStringAtPos(1,0, "%s", pistring);
			vDisplayWriteStringAtPos(3,0, "Back");
			Menu = 1;
			while(Menu == 1){
				if(getButtonPress(BUTTON4) == SHORT_PRESSED) {
					Menu = 0;
				}
			}
		}
		if(getButtonPress(BUTTON2) == SHORT_PRESSED){
			vDisplayClear();
			vDisplayWriteStringAtPos(1,0, "1: Start");
			vDisplayWriteStringAtPos(1,10, "2: Stop");
			vDisplayWriteStringAtPos(2,0, "3: Reset");
			vDisplayWriteStringAtPos(3,0, "4: Leibniz <--> Nikalantha");
			Menu = 2;
			while(Menu == 2){
				if(getButtonPress(BUTTON1) == SHORT_PRESSED){
					vTaskResume(LeibnizTask);
					vDisplayWriteStringAtPos(0,0, "%s", LeibnizString);
				}
				if(getButtonPress(BUTTON2) == SHORT_PRESSED){
					vTaskSuspend(LeibnizTask);
					vDisplayWriteStringAtPos(0,0, "%s", LeibnizString);
				}
				if(getButtonPress(BUTTON3) == SHORT_PRESSED){
					PILeibniz = 0;
					vDisplayWriteStringAtPos(0,0, "%s", LeibnizString);
				}
				if(getButtonPress(BUTTON4) == SHORT_PRESSED) {
					vTaskSuspend(LeibnizTask);
					vTaskResume(InterfaceTask);
					Menu = 0;
				}
			}
		}
		if(getButtonPress(BUTTON3) == SHORT_PRESSED){
			vDisplayWriteStringAtPos(1,0, "1: Start");
			vDisplayWriteStringAtPos(1,10, "2: Stop");
			vDisplayWriteStringAtPos(2,0, "3: Reset");
			vDisplayWriteStringAtPos(3,0, "4: Nikalantha <--> Leibniz");
			Menu = 2;
			while(Menu == 2){
				if(getButtonPress(BUTTON1) == SHORT_PRESSED){
					vTaskResume(NikalanthaTask);
					vDisplayWriteStringAtPos(0,0, "%s", NikalanthaString);
				}
				if(getButtonPress(BUTTON2) == SHORT_PRESSED){
					vTaskSuspend(NikalanthaTask);
					vDisplayWriteStringAtPos(0,0, "%s", NikalanthaString);
				}
				if(getButtonPress(BUTTON3) == SHORT_PRESSED){
					PINika = 0;
					vDisplayWriteStringAtPos(0,0, "%s", NikalanthaString);
				}
				if(getButtonPress(BUTTON4) == SHORT_PRESSED) {
					vTaskSuspend(NikalanthaTask);
					vTaskResume(InterfaceTask);
					Menu = 0;
				}
			}
		}
		if(getButtonPress(BUTTON4) == SHORT_PRESSED){
			vDisplayClear();
			vDisplayWriteStringAtPos(1,0, "The cake is a lie!");
			vDisplayWriteStringAtPos(2,0, "- 'GLaDOS'");
			vTaskDelay(500);
			
		}
		if(getButtonPress(BUTTON1) == LONG_PRESSED){
			
		}
		if(getButtonPress(BUTTON2) == LONG_PRESSED){
			
		}
		if(getButtonPress(BUTTON3) == LONG_PRESSED){
			
		}
		if(getButtonPress(BUTTON4) == LONG_PRESSED){
			
		}
		vTaskDelay(10/portTICK_RATE_MS);
	}
}

//Leibniz-Folge-Task	PI = 4·(1 - (1/3) + (1/5) - (1/7) + (1/9) - (1/11) + (1/13)...)
void vLeibniz(void *pvParameter){
	(void) pvParameter;
	long int i;
	long int n = 1000000;																// Maximale Anzahl berechnungen
	double Summe = 0.0;
	double Zaehler;
	double Nenner;
	for(i = 0; i < n; i ++){	
		Zaehler = pow(-1, i);															//pow Toggelt zwischen -1 und +1 für Vorzeichen --> -1^i
		Nenner = 2*i+1;
		Summe += (Zaehler / Nenner);
		PILeibniz = 4 * Summe;
		int PIint1 = PILeibniz;															// Ganzzahl ermitteln
		float PIkomma1 = PILeibniz - PIint1;											// Die ersten vier Kommastellen ermitteln als float
		int PIint2 = PIkomma1 * 10000;													// Die ersten vier Kommastellen ermitteln als int
		float PIkomma2 = PIkomma1 * 10000;												// Die zweiten vier Kommastellen ermitteln als float
		float PIkomma3 = PIkomma2 - PIint2;												// Die zweiten vier Kommastellen ermitteln als float
		int PIint3 = PIkomma3 * 10000;													// Die zweiten vier Kommastellen ermitteln als int
		float PIkomma4 = PIkomma3 * 10000;												// Die dritten vier Kommastellen ermitteln als float
		float PIkomma5 = PIkomma4 - PIint3;												// Die dritten vier Kommastellen ermitteln als float
		int PIint4 = PIkomma5 * 10000;													// Die dritten vier Kommastellen ermitteln als int
		sprintf (LeibnizString, "%d.%d%d%d", PIint1, PIint2, PIint3, PIint4);			// Ganzzahl und Kommastellen in String einlesen	
		if(PILeibniz == 3.1415926535){													// Ermittlung für Anzahl Zyklen bis PI auf 6 Nachkommastellen genau ist
			vTaskSuspend(LeibnizTask);
		}
	}
}
		
	


//Nilakantha-Folge-Task		PI=3 + 4/(2·3·4) - 4/(4·5·6) + 4/(6·7·8) - 4/(8·9·10) + 4/(10·11·12) - 4/(12·13·14)...
void vNikalantha(void *pvParameter){
	(void) pvParameter;
	long int i = 0;
	long int n = 2;
	double Zaehler = -1;
	if (i == 0){																	// Startwert PI = 3
		PINika = 3;
	}
	for (i = 0; i < n; i++){
		Zaehler = pow(-1, i);														//pow Toggelt zwischen -1 und +1 für Vorzeichen --> -1^i
		PINika = PINika +(Zaehler * 4 / (n * (n + 1) * (n + 2)));
		if (i == 0){																// Einstellung für den ersten Vorzeichen Toggle
			Zaehler = -1;
		}		
		n += 2;
		int PIint1 = PINika;														// Ganzzahl ermitteln
		float PIkomma1 = PINika - PIint1;											// Die ersten vier Kommastellen ermitteln als float
		int PIint2 = PIkomma1 * 10000;												// Die ersten vier Kommastellen ermitteln als int
		float PIkomma2 = PIkomma1 * 10000;											// Die zweiten vier Kommastellen ermitteln als float
		float PIkomma3 = PIkomma2 - PIint2;											// Die zweiten vier Kommastellen ermitteln als float
		int PIint3 = PIkomma3 * 10000;												// Die zweiten vier Kommastellen ermitteln als int
		float PIkomma4 = PIkomma3 * 10000;											// Die dritten vier Kommastellen ermitteln als float
		float PIkomma5 = PIkomma4 - PIint3;											// Die dritten vier Kommastellen ermitteln als float
		int PIint4 = PIkomma5 * 10000;												// Die dritten vier Kommastellen ermitteln als int
		sprintf (NikalanthaString, "%d.%d%d%d", PIint1, PIint2, PIint3, PIint4);	// Ganzzahl und Kommastellen in String einlesen
		if(PINika == 3.1415926535){													// Ermittlung für Anzahl Zyklen bis PI auf 6 Nachkommastellen genau ist
			vTaskSuspend(NikalanthaTask);
		}
	}
}