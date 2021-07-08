/***************************** Include Files *******************************/
#include "dht11.h"

// Vettore delle istanze di configurazione (nel nostro caso solo 1)
myDHT11config myDHT11_ConfigTable[NUMB_DHT11]={
 	{
		XPAR_DHT11_0_DEVICE_ID,
		XPAR_DHT11_0_S00_AXI_BASEADDR,
		XPAR_DHT11_0_S00_AXI_HIGHADDR,
		TRUE
		/* Since our peripheral is interrupt enabled we write TRUE (that is 1U)*/
	}
};


int myDHT11_Initialize(myDHT11 *InstancePtr, u16 DeviceId){
	sleep(1);
	myDHT11config * ConfigPtr;
	Xil_AssertNonvoid(InstancePtr!=NULL);
	ConfigPtr = myDHT11_LookupConfig(DeviceId);
	if (ConfigPtr == (myDHT11config *) NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return myDHT11_CfgInitialize(InstancePtr, ConfigPtr,
				   ConfigPtr->BaseAddress);
};

myDHT11config* myDHT11_LookupConfig(u16 DeviceId){
	myDHT11config* CfgPtr = NULL;

	int Index;
	for (Index = 0; Index < NUMB_DHT11; Index++) {
		if (myDHT11_ConfigTable[Index].DeviceID == DeviceId) {
			CfgPtr = &myDHT11_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
};



int myDHT11_CfgInitialize(myDHT11 *InstancePtr, myDHT11config * Config,UINTPTR EffectiveAddr)
{
	Xil_AssertNonvoid(InstancePtr!=NULL);

	//Settaggio valori di default
	InstancePtr->BaseAddress = EffectiveAddr;

	InstancePtr->InterruptPresent = Config->InterruptPresent;

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	return (XST_SUCCESS);
};

u32 myDHT11_DiscreteRead(myDHT11 *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return DHT11_mReadReg(InstancePtr->BaseAddress,myDHT11_DATA_OffSet);
};

void myDHT11_InterruptGlobalEnable(myDHT11 *InstancePtr)
{
	/*pay attention: the  assert is of void type, differently from before*/
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->InterruptPresent == TRUE);

	u32 Register;
	u32 mask = 1U;

	myDHT11_InterruptAck(InstancePtr);

	Register = DHT11_mReadReg(InstancePtr->BaseAddress, myDHT11_GIES_OffSet);
	DHT11_mWriteReg(InstancePtr->BaseAddress, myDHT11_GIES_OffSet, Register | mask);
	//al posto di 1 possiamo definire la macro myDHT11_GIE_GINTR_ENABLE_MASK
};

void myDHT11_InterruptGlobalDisable(myDHT11 *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->InterruptPresent == TRUE);

	u32 Register;
	u32 mask = 1U;
	Register = DHT11_mReadReg(InstancePtr->BaseAddress, myDHT11_GIES_OffSet);
	DHT11_mWriteReg(InstancePtr->BaseAddress,myDHT11_GIES_OffSet,Register & (~mask));
	//Nuovamente queste macro vanno definite
};

/****************Funzioni aggiunte allo standard********/

void myDHT11_InterruptAck(myDHT11 *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->InterruptPresent == TRUE);

	u32 Register;
	u32 mask = 1U;
	Register = DHT11_mReadReg(InstancePtr->BaseAddress, myDHT11_INTACK_OffSet);
	DHT11_mWriteReg(InstancePtr->BaseAddress, myDHT11_INTACK_OffSet, Register | mask);
};

u32 myDHT11_IsGlobalInterruptEnabled(myDHT11 *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr!=NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->InterruptPresent == TRUE);

	u32 Register;
	Register = DHT11_mReadReg(InstancePtr->BaseAddress, myDHT11_GIES_OffSet);
	return ((Register & 1U)==0 ? MYDHT11_RESET : MYDHT11_SET);
};

u32 myDHT11_GetPendingInterrupt(myDHT11 *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr!=NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->InterruptPresent == TRUE);

	u32 Register;
	Register = DHT11_mReadReg(InstancePtr->BaseAddress, myDHT11_GIES_OffSet);
	return ((Register & 2U)==0 ? MYDHT11_RESET : MYDHT11_SET);
};

u32 myDHT11_IsBusy(myDHT11* InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr!=NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->InterruptPresent == TRUE);

	u32 Register;
	Register = DHT11_mReadReg(InstancePtr->BaseAddress, myDHT11_STATUS_OffSet);
	return ((Register & 1U) == 0 ? MYDHT11_RESET : MYDHT11_SET);
};

void myDHT11_Start(myDHT11* InstancePtr)
{
	u32 Register;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->InterruptPresent == TRUE);

	Register = DHT11_mReadReg(InstancePtr->BaseAddress, myDHT11_CONTROL_OffSet);
	u32 mask = 1U;
	/*
	* Since we mapped the start signal on the former button, we have to set the start bit
	* and then reset it through software before the end of transmission
	* (CHANGE IT)
	*/
	DHT11_mWriteReg(InstancePtr->BaseAddress, myDHT11_CONTROL_OffSet, Register | mask);

	DHT11_mWriteReg(InstancePtr->BaseAddress, myDHT11_CONTROL_OffSet, Register & (~mask));
};

float myDHT11_GetTemp(myDHT11 * InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr!=NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->InterruptPresent == TRUE);

	u32 Register;
	Register = DHT11_mReadReg(InstancePtr->BaseAddress, myDHT11_DATA_OffSet);
	u8 ITempint = 0;
	ITempint = (u8)((Register & 0x0000ff00) >> 8);
	float FTempInt = (float)ITempint;
	u8 ITempdecimale = (u8)(Register & 0x000000ff);
	float FTempFloat = ((float) (ITempdecimale))/100;
	float FTemp = FTempInt + FTempFloat;

	return FTemp;
};


float myDHT11_GetWet(myDHT11 * InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr!=NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->InterruptPresent == TRUE);

	u32 Register;
	Register = DHT11_mReadReg(InstancePtr->BaseAddress, myDHT11_DATA_OffSet);
	u8 IWetint=0;
	IWetint = (u8)((Register & 0xff000000)>>24);
	float FWetInt = (float)IWetint;
	u8 IWetdecimale =  (u8)((Register & 0x00ff0000)>>16);
	float FWetFloat = ((float) (IWetdecimale))/100;
	float FWet = FWetInt + FWetFloat;

	return FWet;
};

int myDHT11_getErrorStatus(myDHT11 * InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr!=NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->InterruptPresent == TRUE);

	u32 Register;
	Register = DHT11_mReadReg(InstancePtr->BaseAddress, myDHT11_STATUS_OffSet);
	if((Register & 2U) != 0){
		return STATUS_CHECKSUMERROR;
	} else if((Register & 4U) != 0){
		return STATUS_PARITYERROR;
	} else return STATUS_NOERROR;
}

/************************** Function Definitions ***************************/
