#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <ctype.h>   // for toupper


int init_uart(void);
static void uart_task(void *, void *, void *);
static void dispatcher_task(void *, void *, void *);
//void run_pattern_task(void*, void *, void *);

char checkIfNumber(char);
int changeToNumber(char);
void sequence_splitting(char []);
int power(int, int);
int transformNumber(char[]);

// GLOBALS
volatile int Transient = 0;

int position = 0;
char sequence_split[20];
char run_sequence[20] = "";

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
		return 1;
	}
	else {
		for(int i=1; i<=power; i++) {
			res *= base;
		}
	}
	printk("\npower: %d", res);
	return res;
}
char checkIfNumber(char character) {
	switch(character) {
		case '0':
			return '0';
			break;
		case '1':
			return '1';
			break;
		case '2':
			return '2';
			break;
		case '3':
			return '3';
			break;
		case '4':
			return '4';
			break;
		case '5':
			return '5';
			break;
		case '6':
			return '6';
			break;
		case '7':
			return '7';
			break;
		case '8':
			return '8';
			break;
		case '9':
			return '9';
			break;	
	}
	return -1;
}
int transformNumber(char num[]) {
	int number = 0;
	int count = strlen(num);
	int position = count-1;
	
	for(int i=0;i<count;i++) {
		char val = checkIfNumber(num[i]);
		int cval = changeToNumber(val);
		number = number + (power(10,position) * cval);
		//printk("number: %d", number);
		position--;
	}
	//printk("\nnumber: %d", number);
	return number;
}
int changeToNumber(char character) {
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
	}
	return number;
}

void sequence_splitting(char location[]) {
		char len[20] = "";
		char r_str[20] = "";
		char g_str[20] = "";
		char y_str[20] = "";
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
				continue;
			}
			else {
				if(len[position-1] == 'r' || len[position-1] == 'R') {
					if(location[i] == 't' || location[i]=='T') {
						continue;
					}
					else {
						r_str[count] = checkIfNumber(location[i]);
						if(strlen(r_str) > 0) {
							r_delay = 0;
						}
						count++;
					}
					
				}
				else if(len[position-1] == 'y' || len[position-1] == 'Y') {
					if(location[i] == 't' || location[i] == 'T') {
						continue;
					}
					else {
						y_str[count] = checkIfNumber(location[i]);
						if(strlen(y_str) > 0) {
							y_delay = 0;
						}
						count++;
					}
					
				}
				else if(len[position-1] == 'g' || len[position-1] == 'G') {
					if(location[i] == 't' || location[i] == 'T') {
						continue;
					}
					else {
						g_str[count] = checkIfNumber(location[i]);
						if(strlen(y_str) > 0) {
							g_delay=0;
						}
						count++;
					}
					
				}
			}
			
		}
		if(r_delay == 0) {
			//printk("r_str: %s", r_str);
			r_delay = transformNumber(r_str);
			//printk("r_delay: %d", r_delay);		
		}
		if(y_delay == 0) {
			y_delay = transformNumber(y_str);
		}
		if(g_delay == 0) {
			g_delay = transformNumber(g_str);
		}
		
		//printk("data: %s\n", len);
		strcpy(sequence_split,len);
		//printk("r_str: %s | y_str: %s | g_str: %s", r_str, y_str, g_str);
		
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
	char sequence[20];
	while (true) {
		if (paused) {
        	k_msleep(100);   // don't spin too hard
        	continue;        // skip everything until resumed
    	}
		// Get next UART message
		struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_MSEC(10));
		
		memcpy(sequence, rec_item->msg, 20);
		k_free(rec_item);
		
		sequence_splitting(sequence);
		if(sequence[0] != 'T' ) {
			strncpy(run_sequence, sequence_split, 20);
			
		}

		if(Transient == 1) {
			printk("Running Transient.");
		}
		
		//printk("\nvalues: %s || r_delay: %d || y_delay: %d || g_delay: %d", sequence_split, r_delay, y_delay, g_delay);
		
		//printk("\nTransient: %d\n", Transient);
		// Loop through received characters
		for (int i = 0; i <= strlen(sequence); i++) {
			char c = toupper((unsigned char)sequence[i]);
			//printk("character: %c", c);
			
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
			else if(c == 'J') {
    			printk("Dispatcher: Button 0 pressed -> %s\n", paused ? "PAUSED" : "RUNNING");
			}
			else if(c == 'T') {
				if(Transient == 0) {
					Transient = 1;
				}
			}

			else if(c == 'K') {
				printk("Dispatcher: button 1 pressed.\n");
			}
			else if(c == 'L') {
				printk("Dispatcher: button 2 pressed.\n");
			}
			else if(c == 'M') {
				printk("Dispatcher: button 3 pressed.\n");
			}
			else if(c == 'N') {
				printk("Dispatcher: button 4 pressed.\n");
			}

			

		}
		while(Transient == 1) {
			struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_MSEC(200));
			memcpy(sequence, rec_item->msg, 20);
			k_free(rec_item);
			if(sequence[0] == 't' || sequence[0] == 'T') {
				Transient = 0;
				break;
			}
			printk("Running task:\n");
			int size = strlen(run_sequence);
			
			for(int i = 0; i < size ;i++) {
				if(run_sequence[i] == 'R' || run_sequence[i] == 'r') {
					printk("red");
					k_mutex_lock(&red_mutex, K_FOREVER);
					k_condvar_signal(&red_signal);
					k_mutex_unlock(&red_mutex);
					k_condvar_wait(&red_ready_signal, &red_ready_mutex, K_FOREVER);
					//k_yield();
				}
				else if(run_sequence[i] == 'y' || run_sequence[i] == 'Y') { 
					printk("yellow");
					k_mutex_lock(&yellow_mutex, K_FOREVER);
					k_condvar_signal(&yellow_signal);
					k_mutex_unlock(&yellow_mutex);
					k_condvar_wait(&yellow_ready_signal, &yellow_ready_mutex, K_FOREVER);
					//k_yield();
				}
				else if(run_sequence[i] == 'g' || run_sequence[i] == 'G') { 
					printk("green");
					k_mutex_lock(&green_mutex, K_FOREVER);
					k_condvar_signal(&green_signal);
					k_mutex_unlock(&green_mutex);
					k_condvar_wait(&green_ready_signal, &green_ready_mutex, K_FOREVER);
					//k_yield();
				}
			}
		k_yield();
		}
	}
}

K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);

