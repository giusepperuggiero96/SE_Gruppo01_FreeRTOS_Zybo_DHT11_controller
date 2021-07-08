#ifndef _FREERTOSHELLOWORLD_H
#define _FREERTOSHELLOWORLD_H

// Includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "atomic.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xscugic.h"
#include "xgpio.h"
#include "dht11.h"
#include "sleep.h"
#include <assert.h>

#define TYPE_VALUE 0        // value è il float della temperatura
#define TYPE_NUMBER 1       // number è il numero di volte che sono stati richiesti i dati al DHT11
#define PRODCONSDIM 10

typedef union numero {
  float valueF;
  uint32_t valueI;
} unionValue;

typedef struct message {
	short type;
	unionValue v;
} MessageRcv;

#endif
