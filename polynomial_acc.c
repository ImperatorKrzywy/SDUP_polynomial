/*
 * polynomial_acc.c

*
 *  Created on: 13 VI 2023
 *      Author: MK, BP
 */

#include "polynomial_acc.h"

#include "xparameters.h"
#include "xllfifo.h"
#include "xstatus.h"
#include <math.h>
#include <stdio.h>

XLlFifo InstanceX;
XLlFifo InstanceCoefs;

#define X_FIFO_DEVICE_ID XPAR_AXI_FIFO_MM_S_0_DEVICE_ID
#define COEFS_FIFO_DEVICE_ID XPAR_AXI_FIFO_MM_S_1_DEVICE_ID
#define ACCELERATOR_LATENCY 7
#define ACCELERATOR_FIFO_LEN 2048



int polynomial_val_compute( u32* x, u32 nbr_of_x, u32* coefs, u32* result, u32 *nbr_of_results )
{

u32 results = 0;

	// Buffers longer than FIFO len are not supported
	if(ACCELERATOR_FIFO_LEN > 2048) return 0;

	//Send ACCELERATOR_LATENCY more values to push out results form accelerator
	if( send_buffer(InstanceCoefs, coefs, 4*sizeof(u32)) == XST_FAILURE )
		goto error;
	//Send ACCELERATOR_LATENCY more values to push out results form accelerator
	if( send_buffer(InstanceX, x, (nbr_of_x+ACCELERATOR_LATENCY)*sizeof(u32)) == XST_FAILURE )
		goto error;
	//Get results
    if( receive_buffer(result, nbr_of_x*sizeof(u32), &results) == XST_FAILURE )
		goto error;
    //Return number of results in bytes
    *nbr_of_results = results/4;

	return 1;

error:
	return 0;

}


/**
 * Initialize FIFOs and its driver
 */
int init_x_fifo_acc(){
XLlFifo_Config *Config;
int Status;

	/* Initialize the Device Configuration Interface driver */
	Config = XLlFfio_LookupConfig(X_FIFO_DEVICE_ID);
	if (!Config) {
		return XST_FAILURE;
	}

	Status = XLlFifo_CfgInitialize(&InstanceX, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Check for the Reset value */
	Status = XLlFifo_Status(&InstanceX);
	XLlFifo_IntClear(&InstanceX,0xffffffff);
	Status = XLlFifo_Status(&InstanceX);
	if(Status != 0x0) {
		return XST_FAILURE;
	}


	return XST_SUCCESS;

}

int init_coefs_fifo_acc(){
XLlFifo_Config *Config;
int Status;

	/* Initialize the Device Configuration Interface driver */
	Config = XLlFfio_LookupConfig(COEFS_FIFO_DEVICE_ID);
	if (!Config) {
		return XST_FAILURE;
	}

	Status = XLlFifo_CfgInitialize(&InstanceCoefs, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Check for the Reset value */
	Status = XLlFifo_Status(&InstanceCoefs);
	XLlFifo_IntClear(&InstanceCoefs,0xffffffff);
	Status = XLlFifo_Status(&InstanceCoefs);
	if(Status != 0x0) {
		return XST_FAILURE;
	}


	return XST_SUCCESS;

}


/**
 * Send reset signal to FIFOs and accelerator
 */
void reset_polynomial_acc(){
	XLlFifo_RxReset(&InstanceX);
	XLlFifo_TxReset(&InstanceX);
	XLlFifo_TxReset(&InstanceCoefs);
}

/**
 * Send data to the input FIFO and accelerator
 */
int send_buffer(XLlFifo Instance, u32* buf, u32 len){

	//Write data to the input FIFO
	XLlFifo_Write(&Instance, buf, len);
	//Initialize data transfer
 	XLlFifo_TxSetLen(&Instance, len);

 	// Check for Transmission completion
 	while( !(XLlFifo_IsTxDone(&Instance)) ){

 	}

 	return XST_SUCCESS;

}


/**
 * Receive date form the output FIFO
 */
int receive_buffer(u32* buf, u32 len, u32* received){
u32 bytes;
int Status;

	//wait for data frame ready
	while(XLlFifo_RxOccupancy(&InstanceX)==0);
	//get number of data in frame
	bytes = XLlFifo_RxGetLen(&InstanceX);
	//Expected number of elements should be ready
	if( len < bytes ) return XST_FAILURE;

    //Perform read operation form FIFO
	XLlFifo_Read(&InstanceX, buf, len);
	//Return number of data read
	*received = len;

	//Check operation status
	Status = XLlFifo_IsRxDone(&InstanceX);
	if(Status != TRUE){
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

void read5Char(char* buff)
{
	for (u32 i = 0; i<7; i++)
	{
		outbyte ( buff[i] = inbyte() );
	}
}

uint32_t stringToFixedPoint(char* str)
{
    uint32_t integerValue = 0;
    uint32_t fixedIntValue = 0;
    uint32_t fixedValue = 0;
    integerValue += (str[0] - '0') * 10;
    integerValue += str[1] - '0';
    fixedIntValue += (str[3] - '0') * 1000;
    fixedIntValue += (str[4] - '0') * 100;
    fixedIntValue += (str[5] - '0') * 10;
    fixedIntValue += (str[6] - '0');
    uint32_t divisor = 5000;
    for(int i = 0; i < 8; i++)
    {
        if(fixedIntValue/(divisor))
        {
            fixedValue += (1 << (7-i));
            fixedIntValue -= divisor;
        }
        divisor /= 2;
        if(divisor == 0)
        {
            break;
        }
    }
    return (integerValue << 8) + (fixedValue);
}
void fixedPointToString(char* str, uint32_t value)
{
    uint16_t integerVal = value >> 8;
    uint16_t fixedVal = value;
    float result = 0;
    uint32_t divisor = 1 << 7;
    for(int i = 0; i < 8; i++)
    {
        if(fixedVal&(divisor))
        {
            result += pow(0.5, i+1);
        }
        divisor >>= 1;
    }
    result += integerVal;
    sprintf(str,"%07.4f", result);
}


