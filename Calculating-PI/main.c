/*
 * Calculating_PI.c
 *
 * Created: 20.03.2018 18:32:07
 * Author : chaos
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


extern void vApplicationIdleHook( void );
void vInterface(void *pvParameter);
void vLeibniz(void *pvParameter);
void vNilakantha(void *pvParameter);
void float64TestTask(void *pvParameters);

TaskHandle_t LeibnizTask;
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

double PI;

// 3.14159265358979323846264338327950288419716939937510 = PI

void vApplicationIdleHook( void )
{	
	
}


int main(void)
{
	vInitClock();
	vInitDisplay();

	evButtonEvents = xEventGroupCreate();
	/*
	xTaskCreate(vInterface, (const char*) "Interface-Task", configMINIMAL_STACK_SIZE+100, NULL, 2, &InterfaceTask);*/
	xTaskCreate(vLeibniz, (const char *) "Leibniz-Folge-Task", configMINIMAL_STACK_SIZE+300, NULL, 1, &LeibnizTask);
	xTaskCreate(vNilakantha, (const char *) "Nilakantha-Folge-Task", configMINIMAL_STACK_SIZE+300, NULL, 1, NULL);

	vTaskStartScheduler();

	return 0;
}

	
//Interface-Task
void vInterface(void *pvParameter){
	(void) pvParameter;
	for(;;){
		vDisplayClear();
			
		vTaskDelay(1000);
	}
}


//Leibniz-Folge-Task
void vLeibniz(void *pvParameter){
	(void) pvParameter;
	long int i;
	int n = 10000;
	double Summe = 0.0;
	double Term;
	double Zaehler;
	double Nenner;
	for(i = 0; i < n; i ++){	
		Zaehler = pow(-1, i);							//pow Toggelt zwischen -1 und +1 für Vorzeichen --> -1^i
		Nenner = 2*i+1;						
		Term = Zaehler / Nenner;
		Summe = Summe + Term;
		PI = 4 * Summe;
		char str[100];
		int PIint1 = PI;											// Ganzzahl ermitteln
		float PIkomma = PI - PIint1;								// Die ersten vier Kommastellen ermitteln als float
		int PIint2 = PIkomma * 10000;								// Die ersten vier Kommastellen ermitteln als int
		float PIkomma2 = PIkomma * 10000;							// Die zweiten vier Kommastellen ermitteln als float
		float PIkomma3 = PIkomma2 - PIint2;							// Die zweiten vier Kommastellen ermitteln als float
		int PIint3 = PIkomma3 * 10000;								// Die zweiten vier Kommastellen ermitteln als int
		float PIkomma4 = PIkomma3 * 10000;							// Die dritten vier Kommastellen ermitteln als float
		float PIkomma5 = PIkomma4 - PIint3;							// Die dritten vier Kommastellen ermitteln als float
		int PIint4 = PIkomma5 * 10000;								// Die dritten vier Kommastellen ermitteln als int
		sprintf (str, "%d.%d%d%d", PIint1, PIint2, PIint3, PIint4);	// Ganzzahl und Kommastellen in String einlesen
		vDisplayClear();											// Löschen des ganzen Displays
		vDisplayWriteStringAtPos(0,0,"Leibniz-Serie");				// Ausgabe auf das Display
		vDisplayWriteStringAtPos(1,0,"Pi ist %s", str);					
		vTaskDelay(400 / portTICK_RATE_MS);
		}
}
		
	


//Nilakantha-Folge-Task				PI=3+4/(2·3·4)-4/(4·5·6)+4/(6·7·8)-4/(8·9·10)+4/(10·11·12)-4/(12·13·14)...
void vNilakantha(void *pvParameter){
	(void) pvParameter;
	long int i;
	int n = 10000;
	double Summe = 0.0;
	double Term;
	double Zaehler = 1;
	double Nenner;
	for (int i = 0; i < n; i++) {
		PI = PI + (Zaehler * (4 / ((n) * (n + 1) * (n + 2))));
		Zaehler = pow(-1, i);
		n += 2;
		}
	}


