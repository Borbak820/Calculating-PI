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


extern void vApplicationIdleHook( void );
void vLedBlink(void *pvParameters);

TaskHandle_t ledTask;

void vApplicationIdleHook( void )
{	
	
}

int main(void)
{
    resetReason_t reason = getResetReason();

	vInitClock();
	vInitDisplay();
	
	xTaskCreate( vLedBlink, (const char *) "ledBlink", configMINIMAL_STACK_SIZE+10, NULL, 1, &ledTask);

	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"FreeRTOS 10.0.1");
	vDisplayWriteStringAtPos(1,0,"EDUBoard 1.0");
	vDisplayWriteStringAtPos(2,0,"Template");
	vDisplayWriteStringAtPos(3,0,"ResetReason: %d", reason);
	vTaskStartScheduler();
	return 0;
}

void vLedBlink(void *pvParameters) {
	(void) pvParameters;
	PORTF.DIRSET = PIN0_bm; /*LED1*/
	PORTF.OUT = 0x01;
	for(;;) {
// 		uint32_t stack = get_mem_unused();
// 		uint32_t heap = xPortGetFreeHeapSize();
// 		uint32_t taskStack = uxTaskGetStackHighWaterMark(ledTask);
// 		vDisplayClear();
// 		vDisplayWriteStringAtPos(0,0,"Stack: %d", stack);
// 		vDisplayWriteStringAtPos(1,0,"Heap: %d", heap);
// 		vDisplayWriteStringAtPos(2,0,"TaskStack: %d", taskStack);
// 		vDisplayWriteStringAtPos(3,0,"FreeSpace: %d", stack+heap);
		PORTF.OUTTGL = 0x01;				
		vTaskDelay(100 / portTICK_RATE_MS);
	}
}


/*

Leibniz Folge:

float leibnizPi(float n){
	double pi=1.0;
	int i;
	int N;
	for (i=3, N=2*n+1; i<=N; i+=2)
	pi += ((i&2) ? -1.0 : 1.0) / i;
	return 4*pi;
}

*/

/*

Nilakantha Folge

?=3+4/(2·3·4)-4/(4·5·6)+4/(6·7·8)-4/(8·9·10)+4/(10·11·12)-4/(12·13·14)


// Function to calculate PI
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


*/