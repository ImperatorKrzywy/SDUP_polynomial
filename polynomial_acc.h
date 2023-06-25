/*
 * polynomial_acc.h
 *
 *  Created on: 13 VI 2023
 *      Author: MK, BP
 */

#ifndef POLYNOMIAL_ACC_H_
#define POLYNOMIAL_ACC_H_

#include "xstatus.h"
#include "xllfifo.h"


// Driver user functions
int polynomial_val_compute(u32* x, u32 nbr_of_x, u32* coefs, u32* result, u32 *nbr_of_results);
int init_x_fifo_acc();
int init_coefs_fifo_acc();
void reset_polynomial_acc();

// Lower level driver function
int send_buffer(XLlFifo Instance, u32* buf, u32 len);
int receive_buffer(u32* buf, u32 len, u32* received);

void read5Char(char* buff);
uint32_t stringToFixedPoint(char* str);
void fixedPointToString(char* str, uint32_t value);

#endif /* POLYNOMIAL_ACC_H_ */
