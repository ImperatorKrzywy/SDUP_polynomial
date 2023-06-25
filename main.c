/*
 * main.c: polynomial pipeline application
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "polynomial_acc.h"

#define X_BUFFER_LENGTH 500

u32 x[X_BUFFER_LENGTH];
u32 x_len = X_BUFFER_LENGTH;
u32 coefs[4];
u32 result[X_BUFFER_LENGTH];
u32 nbr_of_results = 0;
u32 begin, end, step;
char inputData[7];

int main()
{

	init_platform();
	if ( init_x_fifo_acc() == XST_FAILURE )
		goto error;
	if ( init_coefs_fifo_acc() == XST_FAILURE )
		goto error;
	while(1)
	{
		print("\n\r\n\r===========================================================================\n\r");
		print("-------------------------- POLYNOMIAL CALCULATOR --------------------------\n\r");
		print("--------- Compute values of polynomial f(x) = ax^3 + bx^2 + cx + d --------\n\r");
		print("\n\rEnter coefficients: \n\r");
		print("a (7 digits XX.XXXX): ");
		read5Char(inputData);
		coefs[0] = stringToFixedPoint(inputData);
		print("\n\rb (7 digits XX.XXXX): ");
		read5Char(inputData);
		coefs[1] = stringToFixedPoint(inputData);
		print("\n\rc (7 digits XX.XXXX): ");
		read5Char(inputData);
		coefs[2] = stringToFixedPoint(inputData);
		print("\n\rd (7 digits XX.XXXX): ");
		read5Char(inputData);
		coefs[3] = stringToFixedPoint(inputData);

		print("\n\r\n\rEnter x range: \n\r");
		print("Beginning (7 digits XX.XXXX): ");
		read5Char(inputData);
		begin = stringToFixedPoint(inputData);
		print("\n\rEnd (7 digits XX.XXXX): ");
		read5Char(inputData);
		end = stringToFixedPoint(inputData);
		print("\n\rStep (7 digits XX.XXXX): ");
		read5Char(inputData);
		step = stringToFixedPoint(inputData);
		print("\n\r\n\r");

		if(step == 0)
		{
			print("Step value must be greater than 0!\n\r");
			continue;
		}

		x_len = 1 + (end - begin)/step;

		if(begin >= end)
		{
			print("End value must be greater than beginning value!\n\r");
			continue;
		}
		else if (x_len > X_BUFFER_LENGTH)
		{
			print("Too many input values!\n\r");
			continue;
		}


		for (u32 i = 0; i <= x_len; i++)
		{
			x[i] = begin+i*step;
		}



		xil_printf("Given amount of x: %d \n\r", x_len);

		polynomial_val_compute( x, x_len, coefs, result, &nbr_of_results);

		xil_printf("Number of results: %d \n\r\n\r", nbr_of_results);

		char float_val[20] = {0};
		char float_result[20] = {0};
		uint8_t overflow_flag = 0;

		for (u32 i = 0; i< nbr_of_results; i++)
		{
			fixedPointToString(float_val, begin + i*step);
			fixedPointToString(float_result, result[i]);
			if(i>0 && overflow_flag == 0)
			{
				if(result[i]<result[i-1])
					{
						overflow_flag = 1;
					}
			}
			xil_printf("%d. Value of f(%s): %s overflow: %d\n\r", i+1 , float_val , float_result, overflow_flag);
			memset(float_val,0,20);
			memset(float_result,0,20);
		}
		memset(x,0,500);
		memset(result,0,500);
		nbr_of_results = 0;
		reset_polynomial_acc();

	}
error:
	reset_polynomial_acc();
    cleanup_platform();
    while(1);
}
