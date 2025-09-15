#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <ctype.h>   // for toupper

int init_uart(void);
static void uart_task(void *, void *, void *);
static void dispatcher_task(void *, void *, void *);



//extern struct k_fifo dispatcher_fifo;

/****************************
 * Remember to add line:
 * CONFIG_HEAP_MEM_POOL_SIZE=1024
 * to prj.conf
 ****************************/

// Thread initializations
#define STACKSIZE 500
#define PRIORITY 5

// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);


/********************
 * init UART
 */
int init_uart(void) {
	// UART initialization
	if (!device_is_ready(uart_dev)) {
		return 1;
	} 
	return 0;
}

/********************
 * UART task
 */
static void uart_task(void *unused1, void *unused2, void *unused3)
{
	// Received character from UART
	char rc=0;
	// Message from UART
	char uart_msg[20];
	memset(uart_msg,0,20);
	int uart_msg_cnt = 0;

	while (true) {
		// Ask UART if data available
		if (uart_poll_in(uart_dev,&rc) == 0) {
			printk("Received: %c\n",rc);
			// If character is not newline, add to UART message buffer
			if (rc != '\r') {
				uart_msg[uart_msg_cnt] = rc;
				uart_msg_cnt++;
			// Character is newline, copy dispatcher data and put to FIFO buffer
			} else {
				printk("UART msg: %s\n", uart_msg);
                
				struct data_t *buf = k_malloc(sizeof(struct data_t));

					if (buf == NULL) {
						printk("Memory alloc failed\n");
					continue;
					}
					memset(buf, 0, sizeof(struct data_t));
					strncpy(buf->msg, uart_msg, sizeof(buf->msg) - 1);

					// ✅ Put message into FIFO
					k_fifo_put(&dispatcher_fifo, buf);

				// Clear UART receive buffer
					uart_msg_cnt = 0;
					memset(uart_msg,0,20);
			}
		}
		k_msleep(10);
	}
	return 0;
}

/********************
 * Dispatcher task
 */
static void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
	while (true) {

		if (paused) {
        	k_msleep(100);   // don't spin too hard
        	continue;        // skip everything until resumed
    	}
		// Get next UART message
		struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_FOREVER);
		char sequence[20];
		memcpy(sequence, rec_item->msg, 20);
		k_free(rec_item);

		// Loop through received characters
		for (int i = 0; i < strlen(sequence); i++) {
			char c = toupper((unsigned char)sequence[i]);
			
			if (c == 'R') {
				printk("Dispatcher: RED signal\n");
				k_mutex_lock(&red_mutex, K_FOREVER);
				k_condvar_signal(&red_signal);
				k_mutex_unlock(&red_mutex);
                k_condvar_wait(&red_ready_signal, &red_ready_mutex, K_FOREVER);
			}
			else if (c == 'Y') {
				printk("Dispatcher: YELLOW signal\n");
				k_mutex_lock(&yellow_mutex, K_FOREVER);
				k_condvar_signal(&yellow_signal);
				k_mutex_unlock(&yellow_mutex);
                k_condvar_wait(&yellow_ready_signal, &yellow_ready_mutex, K_FOREVER);
			}
			else if (c == 'G') {
				printk("Dispatcher: GREEN signal\n");
				k_mutex_lock(&green_mutex, K_FOREVER);
				k_condvar_signal(&green_signal);
				k_mutex_unlock(&green_mutex);
                k_condvar_wait(&green_ready_signal, &green_ready_mutex, K_FOREVER);
			}
			else if(c == '0') {
				
				
    			printk("Dispatcher: Button 0 pressed -> %s\n", paused ? "PAUSED" : "RUNNING");

			}
			else if(c == '1') {
				printk("Dispatcher: button 1 pressed.\n");
			}
			else if(c == '2') {
				printk("Dispatcher: button 2 pressed.\n");
			}
			else if(c == '3') {
				printk("Dispatcher: button 3 pressed.\n");
			}
			else if(c == '4') {
				printk("Dispatcher: button 4 pressed.\n");
			}

		}
	}
}

K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);
