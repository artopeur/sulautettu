/* 
Arto Peurasaari ja Atte Kankkunen 
	Viikon 2 tehtävä kuvauksessa osa 1 tehty
	Viikon 2 tehtävän kuvauksen keskeytys +1p tehty
	viikon 2 tehtävän lisäpiste2 valmis. +1p
 	Viikon 3 tehtävän perusosa aloitettu
 	Viikon 3 Refactoring aloitettu
  		- tehty leds ja buttons header tiedostot ja syötetty funktiot sinne
		- main.c osiossa vain m ja FIFO määritykset tässä vaiheessa
		- signaalit valoille.
		Dispatcher puuttuu vielä.
*/

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>


// Create dispatcher FIFO buffer
K_FIFO_DEFINE(dispatcher_fifo);

// Condition Variables
K_MUTEX_DEFINE(red_mutex);
K_CONDVAR_DEFINE(red_signal);
K_MUTEX_DEFINE(yellow_mutex);
K_CONDVAR_DEFINE(yellow_signal);
K_MUTEX_DEFINE(green_mutex);
K_CONDVAR_DEFINE(green_signal);

// Condition return variables
K_MUTEX_DEFINE(red_ready_mutex);
K_CONDVAR_DEFINE(red_ready_signal);
K_MUTEX_DEFINE(yellow_ready_mutex);
K_CONDVAR_DEFINE(yellow_ready_signal);
K_MUTEX_DEFINE(green_ready_mutex);
K_CONDVAR_DEFINE(green_ready_signal);

// FIFO dispatcher data type
struct data_t {
	/*************************
	// Add fifo_reserved below
	*************************/
	void *fifo_reserved;
	char msg[20];
};

// GLOBAL VARS

int manual = 0;
int led_state = 0;
int prev = 0;


// led thread initialization
#define STACKSIZE 500
#define PRIORITY 5


// Headers

#include "./dispatcher.h"
#include "./leds.h"
#include "./buttons.h"



// Main program
int main(void)
{
	init_led();
	int ret = init_button();
	if(ret < 0) {
		return 0;
	}

	int ret_uart = init_uart();
	if (ret_uart != 0) {
		printk("UART initialization failed!\n");
		return ret_uart;
	}


	while(1) {

		k_msleep(100);
		k_condvar_broadcast(&red_signal);
		k_condvar_wait(&red_ready_signal, &red_ready_mutex, K_FOREVER);
		k_msleep(100);
		k_condvar_broadcast(&yellow_signal);
		k_condvar_wait(&yellow_ready_signal, &yellow_ready_mutex, K_FOREVER);
		k_msleep(100);
		k_condvar_broadcast(&green_signal);
		k_condvar_wait(&green_ready_signal, &green_ready_mutex, K_FOREVER);
		k_msleep(100);
		k_condvar_broadcast(&yellow_signal);
		k_condvar_wait(&yellow_ready_signal, &yellow_ready_mutex, K_FOREVER);
		
	}

	return 0;
}

