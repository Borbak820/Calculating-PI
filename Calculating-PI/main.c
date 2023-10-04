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
void vInterface(void *Parameter);
void vLeibniz(void *Parameter);
void vNilakantha(void *Parameter);

double PIstring[1];
float pi;

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

// 3.14159265358979323846264338327950288419716939937510 = PI
//math.h nutzbar zur kontrolle

void vApplicationIdleHook( void )
{	
	
}

int main(void)
{
	vInitClock();
	vInitDisplay();

	evButtonEvents = xEventGroupCreate();
	
	xTaskCreate(vInterface, (const char*) "Interface-Task", configMINIMAL_STACK_SIZE+100, NULL, 2, NULL);
	xTaskCreate(vLeibniz, (const char *) "Leibniz-Folge-Task", configMINIMAL_STACK_SIZE+10, NULL, 1, NULL);/*
	xTaskCreate(vNilakantha, (const char *) "Nilakantha-Folge-Task", configMINIMAL_STACK_SIZE+10, NULL, 1, NULL);*/


	vTaskStartScheduler();

	return 0;
}

//Interface-Task
void vInterface(void *Parameter){
	(void) Parameter;	
		for(;;){/*
			vDisplayClear();
			vDisplayWriteStringAtPos(0,0,"Time:");
			vDisplayWriteStringAtPos(0,8,"String");
			vDisplayWriteStringAtPos(1,0,"Alarm:");
			vDisplayWriteStringAtPos(1,8,"Strin2");;
			vTaskDelay(1000);*/
			vTaskResume(vLeibniz);
			
				vDisplayWriteStringAtPos(1,0, "PI = %s", PIstring);
				sprintf(&PIstring[0], "%.6f", pi);	
				vTaskDelay(500);
	}
}
	



//Leibniz-Folge-Task
void vLeibniz(void *Parameter){
	(void) Parameter;
	for(;;){
		long int i;
		long int n;
		double Summe = 0.0;
		double Term;
		/* Applying Leibniz Formula */
		for(i=0;i< n;i++)
		{
			Term = pow(-1, i) / (2*i+1);
			Summe = Summe + Term;
			pi = 4 * Summe;
		}
		}
	}
	
	


/*
//Nilakantha-Folge-Task				?=3+4/(2·3·4)-4/(4·5·6)+4/(6·7·8)-4/(8·9·10)+4/(10·11·12)-4/(12·13·14)
void vNilakantha(void *Parameter){
	(void) Parameter;
	double calculatePI(double PI, double n,
	double sign)
	{
		// Add for 1000000 terms
		for (int i = 0; i <= 1000000; i++) {
			PI = PI + (sign * (4 / ((n) * (n + 1)
			* (n + 2))));
			
			// Addition and subtraction
			// of alternate sequences
			sign = sign * (-1);
			
			// Increment by 2 according to formula
			n += 2;
		}
		
		// Return the value of Pi
		return PI;
	}

	// Driver code
	void main()
	{
		
		// Initialise sum=3, n=2, and sign=1
		double PI = 3, n = 2, sign = 1;
		
		// Function call
		printf("The approximation of Pi is %0.8lf\n",calculatePI(PI, n, sign));
	}

	
}
*/