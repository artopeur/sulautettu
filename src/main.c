/* 

	Viikon 2 tehtävä kuvauksessa osa 1 tehty
	Viikon 2 tehtävän kuvauksen keskeytys +1p tehty
	viikon 2 tehtävän lisäpiste2 valmis. +1p
 	Viikon 3 tehtävän perusosa aloitettu
 	Viikon 3 Refactoring aloitettu
  		- tehty leds ja buttons header tiedostot ja syötetty funktiot sinne
		- main.c osiossa vain m ja FIFO määritykset tässä vaiheessa
		- signaalit valoille ja vastasignaalit tehty.
		- Refaktorointi siis tehty - 1p.
		- Dispatcher tehty. - 1p. - Vastaanottaa datan sarjaportista.
	Yritetään vielä seuraavaa 2p. suoritusta
		- Sekvenssin ajastus - 1p - Tehty
		- Sekvenssin toisto - 1p (Kesken)
*/

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>


// Create dispatcher FIFO buffer
K_FIFO_DEFINE(dispatcher_fifo);


K_FIFO_DEFINE(debug_fifo);


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
/*************************
// Add fifo_reserved below
*************************/
struct data_t {
	
	void *fifo_reserved;
	char msg[20];
};

// * Debug system
 

struct debug_msg_t {
    void *fifo_reserved;     // 1st word reserved for FIFO
    char msg[128];           // debug message
};



void debug_log(const char *fmt, ...)
{
    va_list args;
    struct debug_msg_t *buf = k_malloc(sizeof(struct debug_msg_t));
    if (!buf) return; // drop if no memory

    va_start(args, fmt);
    vsnprintk(buf->msg, sizeof(buf->msg), fmt, args);
    va_end(args);

    k_fifo_put(&debug_fifo, buf);
}

static void debug_task(void *unused1, void *unused2, void *unused3)
{
    while (true) {
        struct debug_msg_t *dbg = k_fifo_get(&debug_fifo, K_FOREVER);
        printk("%s\n", dbg->msg);
        k_free(dbg);
    }
}
/*
//debug data
struct debug_msg_t {
    void *fifo_reserved;     
    char msg[20];            //size might need adjusting
};

 
//Debug task kerää yhä fifoon vaikkei ois päällä
static void debug_task(void *unused1, void *unused2, void *unused3)
{
    while (true) {
        struct debug_msg_t *dbg = k_fifo_get(&debug_fifo, K_FOREVER);

        if (debug_enabled) {
            printk("%s\n", dbg->msg);
        }

        k_free(dbg);
    }
}



//printk log

void debug_log(const char *fmt, ...)
{
    va_list args;
    struct debug_msg_t *buf = k_malloc(sizeof(struct debug_msg_t));
    if (!buf) return; // drop if no memory

    va_start(args, fmt);
    vsnprintk(buf->msg, sizeof(buf->msg), fmt, args);
    va_end(args);

    k_fifo_put(&debug_fifo, buf);
}
*/
// GLOBAL VARS


volatile bool debug_enabled = false;

// Pause flag
volatile bool paused = false;
int ontime = 0;
int r_delay, y_delay, g_delay;

//int manual = 0;
//int led_state = 0;
//int prev = 0;


// led thread initialization
#define STACKSIZE 500
#define PRIORITY 5


// Headers
#include "./leds.h"
#include "./buttons.h"
#include "./dispatcher.h"


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
	init_uart();

	while(1) {
		/*
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
		*/
		k_msleep(100);
	}

	return 0;
}

