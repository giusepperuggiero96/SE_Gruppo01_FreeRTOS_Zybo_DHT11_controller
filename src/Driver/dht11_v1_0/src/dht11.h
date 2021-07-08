#ifndef DHT11_H
#define DHT11_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"
#include "xparameters.h"
#include "xil_assert.h"
#include "xil_types.h"
#include "xstatus.h"
#include "sleep.h"

/**************************** Defines *****************************/
#define DHT11_S00_AXI_SLV_REG0_OFFSET 0
#define DHT11_S00_AXI_SLV_REG1_OFFSET 4
#define DHT11_S00_AXI_SLV_REG2_OFFSET 8
#define DHT11_S00_AXI_SLV_REG3_OFFSET 12
#define DHT11_S00_AXI_SLV_REG4_OFFSET 16
#define DHT11_S00_AXI_SLV_REG5_OFFSET 20
#define DHT11_S00_AXI_SLV_REG6_OFFSET 24
#define DHT11_S00_AXI_SLV_REG7_OFFSET 28


/**************************** Type Definitions *****************************/
/**
 *
 * Write a value to a DHT11 register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param   BaseAddress is the base address of the DHT11device.
 * @param   RegOffset is the register offset from the base to write to.
 * @param   Data is the data written to the register.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void DHT11_mWriteReg(u32 BaseAddress, unsigned RegOffset, u32 Data)
 *
 */
#define DHT11_mWriteReg(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/**
 *
 * Read a value from a DHT11 register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param   BaseAddress is the base address of the DHT11 device.
 * @param   RegOffset is the register offset from the base to write to.
 *
 * @return  Data is the data from the register.
 *
 * @note
 * C-style signature:
 * 	u32 DHT11_mReadReg(u32 BaseAddress, unsigned RegOffset)
 *
 */
#define DHT11_mReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

/************************** Function Prototypes ****************************/
/**
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 *
 * @param   baseaddr_p is the base address of the DHT11 instance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */
XStatus DHT11_Reg_SelfTest(void * baseaddr_p);


/****************Ridefinizione delle macro ************/
#define myDHT11_DATA_OffSet DHT11_S00_AXI_SLV_REG0_OFFSET
#define myDHT11_STATUS_OffSet DHT11_S00_AXI_SLV_REG1_OFFSET
#define myDHT11_CONTROL_OffSet DHT11_S00_AXI_SLV_REG2_OFFSET
#define myDHT11_INTACK_OffSet DHT11_S00_AXI_SLV_REG3_OFFSET
#define myDHT11_GIES_OffSet DHT11_S00_AXI_SLV_REG4_OFFSET

#define STATUS_NOERROR			0U
#define STATUS_CHECKSUMERROR	1U
#define STATUS_PARITYERROR		2U
#define STATUS_UNKNOWNERROR		3U

/****************Macro di utilità**********************/
#define MYDHT11_RESET  0U
#define MYDHT11_SET    1U

#define NUMB_DHT11 1


/**************** Strutture ********************/
typedef  struct {
	u16 DeviceID;
	UINTPTR BaseAddress;
	UINTPTR HighAddress;
	int InterruptPresent;
} myDHT11config;

typedef struct{
	UINTPTR BaseAddress;
	u32 IsReady;
	int InterruptPresent;
} myDHT11;


/************InizializationFunction*******************/
int myDHT11_Initialize(myDHT11 *InstancePtr, u16 DeviceId);
myDHT11config* myDHT11_LookupConfig(u16 DeviceId);

int myDHT11_CfgInitialize(myDHT11 *InstancePtr, myDHT11config * Config,UINTPTR EffectiveAddr);
u32 myDHT11_DiscreteRead(myDHT11 *InstancePtr);

/***********Funzione delle Interrupt*******************/
void myDHT11_InterruptGlobalEnable(myDHT11 *InstancePtr);
void myDHT11_InterruptGlobalDisable(myDHT11 *InstancePtr);

/**********Funzioni aggiunte allo standard************/
void myDHT11_InterruptAck(myDHT11 *InstancePtr);

u32 myDHT11_IsBusy(myDHT11* InstancePtr);

void myDHT11_Start(myDHT11* InstancePtr);

u32 myDHT11_IsGlobalInterruptEnabled(myDHT11 *InstancePtr);
// u32 myDHT11_IsInterruptEnabled(myDHT11 *InstancePtr);

u32 myDHT11_GetPendingInterrupt(myDHT11 *InstancePtr);

float myDHT11_GetTemp(myDHT11 * InstancePtr);
float myDHT11_GetWet(myDHT11 * InstancePtr);

int myDHT11_getErrorStatus(myDHT11 * InstancePtr);



#endif // DHT11_H
