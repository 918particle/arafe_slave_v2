/*
 * cli_commands.c
 *
 *  Created on: Nov 20, 2014
 *      Author: barawn
 *
 *
 *
 */

#include <stdio.h>
#include "cli_commands.h"
#include "usci_uart.h"

#include "arafe.h"

//==================================================================================================
// Device-specific output functions
//==================================================================================================

#include <stdio.h>

void cli_puts(char *str){
	usci_uart_puts(str);
}

void cli_putc(char chr){
    usci_uart_tx(chr);
}

void cli_print_prompt(void){
    cli_puts("\r\n>");
}

void cli_print_error(int error){
    usci_uart_printf("Returned with error code %d\r\n", error); // Example using stdio.h
}

void cli_print_notfound(char *strcmd){
    cli_puts("commands:\r\n");
    cli_puts("5v control sig trig\r\n");
}

/*
 * Command list:
 *
 * dump
 * read #
 * write #
 * flash
 * control #0 #1 where #0 = 0,1,2,3 and #1 = 0,1 (for on, off)
 * 5v # where #=0,1
 * sig #0 #1 where #0=0,1,2,3 and #1 = 0-31.
 * trig #0 #1 where #0=0,1,2,3 and #1 = 0-31.
 *
 */

/* Not used anymore now that we have all-encompassing check_arguments
 * bool check_2_arguments(uint16_t argc, char *argv[], uint8_t arg2_limit, char *errstr) {
	char *arg;
	if (argc < 3) goto err_msg;
	arafe_command = (*argv[1] - '0');
	if (arafe_command > 3) goto err_msg;
	arg = argv[2];
	arafe_argument = (arg[0] - '0');
	if (arg[1] != 0x0) {
		arafe_argument = arafe_argument * 10;
		arafe_argument += arg[1]-'0';
		if (arg[2] != 0x0) {
			if (arafe_argument < 26) {
				arafe_argument = arafe_argument * 10;
				if (0xFF - arafe_argument < arg[2]-'0') {
					goto err_msg;
				} else {
					arafe_argument += arg[2] - '0';
				}
			} else {
				goto err_msg;
			}
		}
	}
	if (arafe_argument > arg2_limit) goto err_msg;
	return true;
err_msg:
	usci_uart_puts(errstr);
	return false;
}
*/
//Begin check_arguments prototyping function writing(a universal check arguments function)
bool check_arguments(uint16_t argc, char *argv[], uint8_t num_args, uint8_t arg1_limit, uint8_t arg2_limit, char *errstr) {
	//argc -> counts the number of things in argv +1; argv -> holds each input as a slot in an array;  num_args -> number of arguments that the function string expects(NOT INCLUDING STRING),
	//arg1_limit -> limit for argv[1]; arg2_limit -> limit for argv[2]; errstr -> string for error message


	char *arg1, *arg2; //Holder value since argv[1][2] does not access the second digit in the value of argv[1]
	uint8_t temp_args;//Keeps track of arguments left to be checked
	temp_args = num_args;
	if (num_args > argc) goto err_msg;
	if (num_args < argc) goto err_msg;

	arg1 = argv[1];//Holder variable (to access digits)
	arafe_command = (arg1[0] - '0');//Look at first digit for argument #1
	if (arg1[1] != 0x0){//If 0, skip if statement
		arafe_command = arafe_command * 10;
		arafe_command += arg1[1] - '0';
	}
	if (arafe_command > arg1_limit) goto err_msg;//check that first argument is not above the function string's specific limit

	temp_args--;//lower the temporary variable for 1 argument checked
	if (temp_args == 1) return true; //Determine whether or not we need to keep checking arguments, since the chip has a awful habit of using registers and deleting the values I assign them... 1 means no more arguments

	arg2 = argv[2];//Ditto
	arafe_argument = (arg2[0] - '0');//Repeat for argument #2
	if (arg2[1] != 0x0) {
		arafe_argument = arafe_argument * 10;
		arafe_argument += arg2[1]-'0';
		if (arg2[2] != 0x0) {
			if (arafe_argument < 26) {
				arafe_argument = arafe_argument * 10;
				if (0xFF - arafe_argument < arg2[2]-'0') {
					goto err_msg;
				} else {
					arafe_argument += arg2[2] - '0';
				}
			} else {
				goto err_msg;
			}
		}
	}
	if (arafe_argument > arg2_limit) goto err_msg;//make sure argument #2 is within specified limits

	return true;//Give the go ahead
err_msg:
	usci_uart_puts(errstr);
	return false;//Implode
}

 //End function prototyping/////////////////////////////////////



int cmd_5v(uint16_t argc, char *argv[]) {
	if (argc < 2) goto err_msg;
	arafe_argument = (*argv[1] - '0');
	if (arafe_argument > 1) goto err_msg;
	arafe_command = ARAFE_COMMAND_5V;
	arafe_send_command();
	return 0;
err_msg:
	usci_uart_puts("5v 0/1\r\n");
	return 0;
}


//cmd_something(uint16_t argc, argv, numargs, arg1_limit, arg2_limit, errstr) NOTE: num_args is the total number of arguments, so the 'string' argument plus whichever others.
int cmd_control(uint16_t argc, char *argv[]) { //WORKS w/ check_arguments
	// Input checking.
	if (check_arguments(argc, argv, 3, 3, 1, "control 0-3 0/1\r\n")) {
		arafe_command |= 0x8;
		arafe_send_command();
	}
	return 0;
}

int cmd_sig(uint16_t argc, char *argv[]) {//WORKS
	if (check_arguments(argc, argv, 3, 3, 31, "sig 0-3 0-31\r\n")) arafe_send_command();
	return 0;
}


int cmd_trig(uint16_t argc, char *argv[]) {//WORKS
	if (check_arguments(argc, argv, 3, 3, 31, "trig 0-3 0-31\r\n")) {
		arafe_command |= 0x4;
		arafe_send_command();
	}
	return 0;
}

int cmd_dump(uint16_t argc, char *argv[]) {
	// This is more complicated. Deal with it later.
	return 0;
}

int cmd_flash(uint16_t argc, char *argv[]) {//Flash has no arguments, therefore it doesn't need to be checked by check_arguments
	arafe_command = 0x20;//Do not raise these bits as in other functions, since they are not checked by check_arguments
	arafe_send_command();
	return 0;
}

int cmd_read(uint16_t argc, char *argv[]) {
	if (check_arguments(argc, argv, 2, 15, 0, "read 0-15\r\n")) {
		arafe_command |= 0x40;
		arafe_send_command();
		}
	return 0;
}

int cmd_write(uint16_t argc, char *argv[]) {
	if (check_arguments(argc, argv, 3, 15, 255, "write 0-15 0-255\r\n" )){
		arafe_command |= 0x80;
		arafe_send_command();
		}
	return 0;
}

//--------------------------------------------------------------------------------------------------
/*int cmdArgList(uint16_t argc, char *argv[]){
    cli_puts("Argument List:\r\n");
    int i;
    for(i=0;i<argc;i++){
        cli_putc('[');
        cli_puts(argv[i]);
        cli_puts("]\r\n");
    }
    return(0);
}
*/
