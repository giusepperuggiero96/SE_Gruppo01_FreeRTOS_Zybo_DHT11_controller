#include "freertos_hello_world.h"

#define TIMER_ID				1
#define DELAY_10_SECONDS		10000UL
#define DELAY_4_SECONDS			4000UL
#define DELAY_2_SECONDS			2000UL
#define DELAY_1_SECOND			1000UL
#define DELAY_0_5_SECONDS		500UL
#define DELAY_0_2_SECONDS		200UL
#define TIMER_CHECK_THRESHOLD	9
#define GPIO_CHANNEL_1			1

/*-----------------------------------------------------------*/
const TickType_t x1second 	= pdMS_TO_TICKS( DELAY_1_SECOND );
const TickType_t x2seconds 	= pdMS_TO_TICKS( DELAY_2_SECONDS );
const TickType_t x4seconds 	= pdMS_TO_TICKS( DELAY_4_SECONDS );
const TickType_t x10seconds = pdMS_TO_TICKS( DELAY_10_SECONDS );
const TickType_t x05seconds = pdMS_TO_TICKS( DELAY_0_5_SECONDS );
const TickType_t x02seconds = pdMS_TO_TICKS( DELAY_0_2_SECONDS );

/*-----------------------------------------------------------*/
/* The Tx and Rx tasks as described at the top of this file. */
static void prvTask1	( void *pvParameters );
static void prvTask2	( void *pvParameters );
static void prvTask3	( void *pvParameters );
static void prvTask4	( void *pvParameters );
static void prvTask5	( void *pvParameters );
static void prvTask6	( void *pvParameters );
static void prvTask7	( void *pvParameters );

//static void vTimerCallback( TimerHandle_t pxTimer );
/*-----------------------------------------------------------*/

/* The queue used by the Tx and Rx tasks, as described at the top of this
file. */
static TaskHandle_t xTask1;
static TaskHandle_t xTask2;
static TaskHandle_t xTask3;
static TaskHandle_t xTask4;
static TaskHandle_t xTask5;
static TaskHandle_t xTask6;
static TaskHandle_t xTask7;

static QueueHandle_t xLogQueue   = NULL;
static QueueHandle_t xHumidQueue = NULL;

static SemaphoreHandle_t xProdConsMutex = NULL;
static SemaphoreHandle_t xCountMutex    = NULL;


/*-----------------------------------------------------------*/
//sezione variabili critiche
MessageRcv xProdCons[PRODCONSDIM];
volatile uint32_t uSentCtl = 0;
int uHead = 0;
int uTail = 0;
int uProdConsAvailable = 0;

/*-----------------------------------------------------------*/
//sezione variabili di utilità
volatile u8 switch_up = 0;

/*-----------------------------------------------------------*/
// FreeRTOS extern ScuGic handler
extern XScuGic xInterruptController;

/*-----------------------------------------------------------*/
// AXI_GPIO variables declarations
XGpio gpio_btn;	//0
XGpio gpio_led;	//1
XGpio gpio_sws;	//2

// DHT11 instance handle
myDHT11 dht_handle;


/*-----------------------------------------------------------*/
// Interrupt callback handlers


// Switch callback
void GpioHandlerSwitch(void *CallbackRef)
{
	static u32 oldSws = 0;
	u32 Mask, Value, Sws, changed;
	XGpio *GpioPtr = (XGpio *)CallbackRef;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	XGpio_InterruptGlobalDisable(GpioPtr);
	Mask = XGpio_InterruptGetEnabled(GpioPtr);
	XGpio_InterruptDisable(GpioPtr, Mask);
	//granularity at channel level, not bit level
	Value = XGpio_InterruptGetStatus(GpioPtr);

	Sws = XGpio_DiscreteRead(GpioPtr, GPIO_CHANNEL_1);
	changed = Sws ^ oldSws;
	oldSws = Sws;

	if ((changed & 0x0001) != 0){
		//rising edge
		if ((Sws & 0x0001) == 1){
			// Variabile usata per bloccare e sbloccare il task periodico
			switch_up = 1;
			vTaskNotifyGiveFromISR( xTask2, &xHigherPriorityTaskWoken );
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		} else {
		//falling edge
			switch_up = 0;
		}	
	}

	/* Clear the Interrupt */
	XGpio_InterruptClear(GpioPtr, Value);
	XGpio_InterruptEnable(GpioPtr, Mask);
	XGpio_InterruptGlobalEnable(GpioPtr);
}



// Button callback
void GpioHandlerBtn(void *CallbackRef)
{
	static u32 oldBtn = 0;
	u32 Mask, Value, Btn, changed;
	XGpio *GpioPtr = (XGpio *)CallbackRef;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	XGpio_InterruptGlobalDisable(GpioPtr);
	Mask = XGpio_InterruptGetEnabled(GpioPtr);
	XGpio_InterruptDisable(GpioPtr, Mask);
	Value = XGpio_InterruptGetStatus(GpioPtr);

	Btn = XGpio_DiscreteRead(GpioPtr, GPIO_CHANNEL_1);

	changed = Btn ^ oldBtn;
	oldBtn = Btn;

	if ((changed & 0x0001) != 0){
		//rising edge
		if ((Btn & 0x0001) == 1){
			// Logica relativa al task 1
			vTaskNotifyGiveFromISR( xTask1, &xHigherPriorityTaskWoken );
			/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
			should be performed to ensure the interrupt returns directly to the highest
			priority task.  The macro used for this purpose is dependent on the port in
			use and may be called portEND_SWITCHING_ISR(). */
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		}	
	}
	if ((changed & 0x0002) != 0){
		//rising edge
		if ((Btn & 0x0002) != 0){
			// Bottone premuto: 2
			// Logica relativa al task 3
			vTaskNotifyGiveFromISR( xTask3, &xHigherPriorityTaskWoken );
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		}	
	}
	
	/* Clear the Interrupt */
	XGpio_InterruptClear(GpioPtr, Value);
	XGpio_InterruptEnable(GpioPtr, Mask);
	XGpio_InterruptGlobalEnable(GpioPtr);
}



// DHT11 callback
void DHT11Handler(void* CallbackRef)
{
	myDHT11 *myDHT11Ptr = (myDHT11 *)CallbackRef;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	myDHT11_InterruptGlobalDisable(myDHT11Ptr);

	vTaskNotifyGiveFromISR( xTask4, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	
	myDHT11_InterruptAck(myDHT11Ptr);
	myDHT11_InterruptGlobalEnable(myDHT11Ptr);
}


/*-----------------------------------------------------------*/
// Main task
int main( void )
{
	xil_printf( "Hello from S-E-ntimentaloni\r\n" );

	int Status;
	/*start of DHT11 setup*/
	Status = myDHT11_Initialize(&dht_handle, XPAR_DHT11_0_DEVICE_ID);
	assert(XST_SUCCESS == Status);
	/*end of DHT11 setup*/

	/*start of gpio setup*/
	/* Initialize the GPIO driver. If an error occurs then exit */
	Status = XGpio_Initialize(&gpio_btn, XPAR_AXI_GPIO_0_DEVICE_ID);
	assert(XST_SUCCESS == Status);
	XGpio_SetDataDirection(&gpio_btn, GPIO_CHANNEL_1, 0xff);
	Status = XGpio_Initialize(&gpio_led, XPAR_AXI_GPIO_1_DEVICE_ID);
	assert(XST_SUCCESS == Status);
	XGpio_SetDataDirection(&gpio_led, GPIO_CHANNEL_1, 0x00);
	Status = XGpio_Initialize(&gpio_sws, XPAR_AXI_GPIO_2_DEVICE_ID);
	assert(XST_SUCCESS == Status);
	XGpio_SetDataDirection(&gpio_sws, GPIO_CHANNEL_1, 0xff);
	/*end of gpio setup*/


	xPortInstallInterruptHandler( XPAR_FABRIC_GPIO_0_VEC_ID, GpioHandlerBtn, &gpio_btn );
	vPortEnableInterrupt( XPAR_FABRIC_GPIO_0_VEC_ID );
	xPortInstallInterruptHandler( XPAR_FABRIC_GPIO_2_VEC_ID, GpioHandlerSwitch, &gpio_sws );
	vPortEnableInterrupt( XPAR_FABRIC_GPIO_2_VEC_ID );

	xPortInstallInterruptHandler( XPAR_FABRIC_DHT11_0_VEC_ID, DHT11Handler, &dht_handle );
	vPortEnableInterrupt( XPAR_FABRIC_DHT11_0_VEC_ID );

	/*
	 * Enable the GPIO channel interrupts so that push button can be
	 * detected and enable interrupts for the GPIO device
	 */
	XGpio_InterruptEnable(&gpio_sws, GPIO_CHANNEL_1);
	XGpio_InterruptEnable(&gpio_btn, GPIO_CHANNEL_1);
	XGpio_InterruptGlobalEnable(&gpio_sws);
	XGpio_InterruptGlobalEnable(&gpio_btn);

	/* Enable DHT11 interrupt */
	myDHT11_InterruptGlobalEnable(&dht_handle);

	/* Abilitazione degli interrupt del processing-system */
	Xil_ExceptionEnable();
	/*End of gic setup*/

	//sezione task: ATTENZIONE, LE PRIORITÀ NON SONO STATE ANCORA ASSEGNATE PER BENE, RIVEDERE
	xTaskCreate( prvTask1,
				 ( const char * ) "Task1",
				 configMINIMAL_STACK_SIZE,
				 NULL,
				 tskIDLE_PRIORITY + 7,
				 &xTask1 );

	xTaskCreate( prvTask2,
				 ( const char * ) "Task2",
				 configMINIMAL_STACK_SIZE,
				 NULL,
				 tskIDLE_PRIORITY + 6,
				 &xTask2 );

	xTaskCreate( prvTask3,
				 ( const char * ) "Task3",
				 configMINIMAL_STACK_SIZE,
				 NULL,
				 tskIDLE_PRIORITY + 2,
				 &xTask3 );

	xTaskCreate( prvTask4,
				 ( const char * ) "Task4",
				 configMINIMAL_STACK_SIZE,
				 NULL,
				 tskIDLE_PRIORITY + 5,
				 &xTask4 );
	
	xTaskCreate( prvTask5,
				 ( const char * ) "Task5",
				 configMINIMAL_STACK_SIZE,
				 NULL,
				 tskIDLE_PRIORITY + 4,
				 &xTask5 );	

	xTaskCreate( prvTask6,
				 ( const char * ) "Task6",
				 configMINIMAL_STACK_SIZE,
				 NULL,
				 tskIDLE_PRIORITY + 3,
				 &xTask6 );

	xTaskCreate( prvTask7,
				 ( const char * ) "Task7",
				 configMINIMAL_STACK_SIZE,
				 NULL,
				 tskIDLE_PRIORITY + 1,
				 &xTask7 );


	

	xLogQueue = xQueueCreate(	32,
								sizeof(MessageRcv));
								
	configASSERT( xLogQueue );

	xHumidQueue = xQueueCreate(	16,
								sizeof(float));		//check il dht11 come manda l'umidità, 8bit parte intera 8 bit parte decimale 
	configASSERT( xHumidQueue );


	xProdConsMutex = xSemaphoreCreateMutex();
	xCountMutex = xSemaphoreCreateMutex();

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	for( ;; );
}

/*-----------------------------------------------------------*/
/*
 * Task 1 - Azionato da Bottone
 * Richiesta di lettura temperatura e umidità al controller per 
 * sensore DHT11.
 */
static void prvTask1(void *pvParameters)
{
	for( ;; )
	{
		// Il task è in attesa indefinita di una notifica da parte della ISR di btn_1
		ulTaskNotifyTake( pdTRUE,  portMAX_DELAY );
		myDHT11_Start(&dht_handle);
	}

}

/*-----------------------------------------------------------*/
/*
 * Task 2 - Azionato da Switch
 * Richiesta di lettura al controller sensore DHT11.
 */
static void prvTask2(void *pvParameters)
{
	float fValue_temp;
	float fValue_hum;
	TickType_t xLastWakeTime;
	for( ;; )
	{
		ulTaskNotifyTake( pdTRUE,  portMAX_DELAY );
		myDHT11_InterruptGlobalDisable(&dht_handle);
		xLastWakeTime = xTaskGetTickCount();
		while(switch_up){

			vTaskDelayUntil( &xLastWakeTime, x2seconds);

			// Richiesta di lettura dei dati al controller dht11
			// Attesa del completamento della misurazione
			// Ricezione dei dati
			//while( myDHT11_IsBusy(&dht_handle) == MYDHT11_SET ){xil_printf("wait busy\r\n");}
			myDHT11_Start(&dht_handle);
			while( myDHT11_IsBusy(&dht_handle) == MYDHT11_SET ){}
			if (myDHT11_getErrorStatus(&dht_handle) != STATUS_NOERROR)
				xil_printf("Peripheral Error\r\n");
			fValue_temp = myDHT11_GetTemp(&dht_handle);
			fValue_hum = myDHT11_GetWet(&dht_handle);

			// Il dato di temperatura va inserito sulla coda circolare prod/cons
			if( xSemaphoreTake( xProdConsMutex,  x05seconds ) == pdTRUE ){
				//xil_printf( "Sez critica task5\r\n" );
				if(uProdConsAvailable != PRODCONSDIM)
				{
					xil_printf( "Sez critica task2, ci sono %d valori\r\n",  uProdConsAvailable);
					xProdCons[uTail].v.valueF = fValue_temp;
					uTail = (uTail + 1) % PRODCONSDIM;
					uProdConsAvailable++;
				}
				xSemaphoreGive( xProdConsMutex );
			}else{
				xil_printf("Semaforo non acquisito entro il tempo di timeout.");
			}


			// Incremento atomico della variabile che tiene traccia del numero di misurazioni
			// Si utilizza l'incremento atomico in quanto il semplice incremento di una 
			// variabile in mutua esclusione non richiede necessariamente l'acquisizione di un mutex.
			Atomic_Add_u32(&uSentCtl, 1);


			// Il dato di umidità relativa va inserito sulla coda fifo
			// Send non bloccante
			xQueueSend(xHumidQueue, &fValue_hum, 0);

		}
		myDHT11_InterruptGlobalEnable(&dht_handle);
	}
}

/*-----------------------------------------------------------*/
/*
 * Task 3 - Azionato da Bottone
 * Invio di un messaggio di log (numero di accessi al sensore
 * regolato da un mutex) sulla coda.
 */
static void prvTask3(void *pvParameters)
{
	MessageRcv xLogMsgTask3;
	xLogMsgTask3.type = 1;
	xLogMsgTask3.v.valueI = 0;
	for( ;; )
	{

		ulTaskNotifyTake( pdTRUE,  portMAX_DELAY );

		xLogMsgTask3.v.valueI = uSentCtl;


		/* Send the next value on the queue.  The queue should always be
		empty at this point so a block time of 0 is used. */
		xQueueSend( xLogQueue,			/* The queue being written to. */
					&xLogMsgTask3, /* The address of the data being sent. */
					0UL );			/* The block time. */

	}

}

/*-----------------------------------------------------------*/
/*
 * Task 4 - Azionato da Interrupt Controller DHT11
 * Invio della temperatura e dell'umidità tramite strutture
 * gestite da semafori.
 */
static void prvTask4(void *pvParameters)
{
	float fValue_temp;
	float fValue_hum;

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	for( ;; )
	{
		ulTaskNotifyTake( pdTRUE,  portMAX_DELAY );
		if( myDHT11_IsBusy(&dht_handle) == MYDHT11_RESET){
			if (myDHT11_getErrorStatus(&dht_handle) != STATUS_NOERROR)
				xil_printf("Peripheral Error\r\n");
			fValue_temp = myDHT11_GetTemp(&dht_handle);
			fValue_hum = myDHT11_GetWet(&dht_handle);
		}

		// Il dato di temperatura va inserito sulla coda circolare prod/cons
		xSemaphoreTake( xProdConsMutex,  portMAX_DELAY);
		//xil_printf( "Sez critica task5\r\n" );
		if(uProdConsAvailable != PRODCONSDIM)
		{
			xil_printf( "Sez critica task4, ci sono %d valori\r\n",  uProdConsAvailable);
			xProdCons[uTail].v.valueF = fValue_temp;
			uTail = (uTail + 1) % PRODCONSDIM;
			uProdConsAvailable++;
		}
		xSemaphoreGive( xProdConsMutex );

		Atomic_Add_u32(&uSentCtl, 1);

		// Il dato di umidità relativa va inserito sulla coda fifo
		// Send non bloccante
		xQueueSend(xHumidQueue, &fValue_hum, 0);
	}

}

/*-----------------------------------------------------------*/
/*
 * Task 5 - Task periodico
 * Si mette in attesa sul semaforo (timed wait), consuma il
 * messaggio, lo mostra sui led e lo piazza sulla log queue.
 */
static void prvTask5(void *pvParameters)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	MessageRcv LogMsgTask5;
	LogMsgTask5.type = 0;
	int consumed = 0;

    for( ;; )
    {
        // Wait for the next cycle.
        vTaskDelayUntil( &xLastWakeTime, x4seconds );
        if( xSemaphoreTake( xProdConsMutex,  x05seconds ) == pdTRUE){
			//xil_printf( "Sez critica task5\r\n" );
			if(uProdConsAvailable)
			{
				xil_printf( "Sez critica task5, ci sono %d valori\r\n",  uProdConsAvailable);
				//TempMex = xProdCons[uHead];
				LogMsgTask5.v.valueF = xProdCons[uHead].v.valueF;
				uHead = (uHead+1) % PRODCONSDIM;
				uProdConsAvailable--;
				consumed = 1;
			}
			xSemaphoreGive( xProdConsMutex );
		}
		
		if(consumed)
		{
			//mostra il messaggio sui led
			XGpio_DiscreteWrite(&gpio_led, GPIO_CHANNEL_1, ((int)(LogMsgTask5.v.valueF))&0x000f);

			//manda sulla log queue
			xQueueSend( xLogQueue,			/* The queue being written to. */
						&LogMsgTask5, /* The address of the data being sent. */
						0UL );			/* The block time. */
			consumed = 0;
		}	
    }


}

/*-----------------------------------------------------------*/
/*
 * Task 6 - Task shitpost
 * Prende il valore dell'umidità dalla coda e fa calcoli inutili.
 */
static void prvTask6(void *pvParameters)
{
	float xHumidity;
	for( ;; )
    {
		xQueueReceive( xHumidQueue,		/* The queue being read. */
				&xHumidity,			/* Data is read into this address. */
				portMAX_DELAY );	/* Wait without a timeout for data. */		
		//simula un task che fa operazioni di calcolo e viene prelazionato
		printf("Received %.2f\n",xHumidity);
		xHumidity = xHumidity*xHumidity*xHumidity*xHumidity;
		xHumidity = ((xHumidity*15)/18)+6-28;
	}
	
}

/*-----------------------------------------------------------*/
/*
 * Task 7 - Task server
 * Legge sulla log queue e manda i dati sulla seriale con xil_printf
 */
static void prvTask7(void *pvParameters)
{
	MessageRcv xReceived;
	for( ;; )
	{
		/* Delay for 1 second. */
		//vTaskDelay( x1second );
		xQueueReceive( xLogQueue,		/* The queue being read. */
				&xReceived,			/* Data is read into this address. */
				portMAX_DELAY );	/* Wait without a timeout for data. */
		if(xReceived.type == TYPE_VALUE)
			printf("Received %.2f\n",xReceived.v.valueF);
		else if(xReceived.type == TYPE_NUMBER)
			xil_printf("Received number %d\r\n", xReceived.v.valueI);
	}

}
   
/*-----------------------------END------------------------------*/