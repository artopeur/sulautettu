#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <ctype.h>   // for toupper


int init_uart(void);
static void uart_task(void *, void *, void *);
static void dispatcher_task(void *, void *, void *);
int checkIfNumber(char);
void sequence_splitting(char*);
int power(int, int);

// GLOBALS
int r_delay, y_delay, g_delay;
bool Transient = false;
int position = 0;
char sequence_split[20];

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

int power(int base, int power) {
	int res = 1;
	if(power == 0) {
		power = 1;
	}
	else {
		for(int i=1; i<=power; i++) {
			res *= base ;
		}
	}
	printk("\npower: %d\n", res);
	return res;
}

int checkIfNumber(char character) {
	int number = 0;
	switch(character) {
	case '0':
		number = 0;
		break;
	case '1': 
		number = 1; 
		break;
	case '2': 
		number = 2;
		break;
	case '3': 
		number = 3; 
		break;
	case '4': 
		number = 4; 
		break;
	case '5': 
		number = 5;
		break;
	case '6': 
		number = 6;
		break;
	case '7': 
		number =7;
		break;
	case '8':
		number = 8;
		break;
	case '9':
		number = 9;
		break;
	case ',':
		number = 1;
		break;
	case '/':
		number = -1;
		break;
	default:
		number = -2;
		break;
	}
	return number;
}

void sequence_splitting(char *location) {
		char len[20] = "";
		position = 0;
		int count = 0;
		for(int i = 0; i< strlen(location); i++) {
			if(location[i] == 'r' || location[i] == 'g' || location[i] == 'y') {
				len[position] = location[i];
				position++; 
			}
			else if(location[i] == 'R' || location[i] == 'G' || location[i] == 'Y') {
				len[position] = location[i];
				position++;
			}
			else if(location[i] == ',') {
				continue;
			}
			
			else if(location[i] == 'T') {
				Transient = true;
			}
			else {
				if(len[position-1] == 'r' || len[position-1] == 'R') {
					r_delay = 0;
					
					if(count != strlen(location)) {
						int r_temp = checkIfNumber(location[i]);
						printk("%d", r_temp);
						int pow = power(10,count);
						count++;
						
						r_delay += r_temp*pow;
						
					}
					//r_delay = checkIfNumber(location[i]);
				}
				else if(len[position-1] == 'y' || len[position-1] == 'Y') {
					y_delay = checkIfNumber(location[i]);
				}
				else if(len[position-1] == 'g' || len[position-1] == 'G') {
					g_delay = checkIfNumber(location[i]);
					
				}
			}
		}
		printk("data: %s", len);
		strcpy(sequence_split,len);
	}

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
	return;
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
		
		sequence_splitting(sequence);


		printk("\nvalues: %s || r_delay: %d || y_delay: %d || g_delay: %d", sequence_split, r_delay, y_delay, g_delay);
		printk("\nTransient: %d\n", Transient);
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
