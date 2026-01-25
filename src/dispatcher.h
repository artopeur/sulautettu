#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <ctype.h>   // for toupper
#include <stdarg.h>  // for va_list
#include <string.h>  // for strlen, strncpy

// ERROR DEFINITIONS
#define TIME_LEN_ERROR      -1
#define TIME_ARRAY_ERROR    -2
#define TIME_VALUE_ERROR    -3
#define WRONG_CHARS_ERROR   -4
#define CHARACTERS_NULL_ERROR -5
#define SEQUENCE_ERROR	-6
#define SEQUENCE_FAILED -7


// Timer initializations
struct k_timer timer;
void timer_handler(struct k_timer *timer_id);
bool red_state = false;

// Debugger initialization
void debug_log(const char *fmt, ...);
void debug_task(void *unused1, void *unused2, void *unused3);
extern volatile bool debug_enabled;


int init_uart(void);
static void uart_task(void *, void *, void *);
static void dispatcher_task(void *, void *, void *);
//void run_pattern_task(void*, void *, void *);

char checkIfNumber(char);
int changeToNumber(char);
int checkChars(char *characters);
void sequence_splitting(char []);
int power(int, int);
int transformNumber(char[]);
int time_parse(char *time);
int sequence_check(char *run);
// GLOBALS
volatile int Transient = 0;
//volatile bool debug_enabled = false;

extern volatile bool debug_enabled;
int position = 0;
char sequence_split[20];
char run_sequence[20] = "";
char result_array[20] = "";

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
 * Debug system
 */
/*

/********************
 * Application code
 */

 //UART to robot_arto
// fixed the fixed the fixed sequence_check function

int sequence_check(char *run) {
    if (run == NULL) {
		printk("%dX", SEQUENCE_FAILED);
        return SEQUENCE_FAILED; // Empty input
    }

    int len = strlen(run);
    if (len < 2) {
		printk("%dX", SEQUENCE_FAILED);
        return SEQUENCE_FAILED; // too short
    }

    bool invalid = false;
    int idx = 0;
    memset(result_array, 0, sizeof(result_array));

    for (int i = 1; i < len; i++) {  // skip first char (A)
        char c = toupper((unsigned char)run[i]);
        if (c == 'X') break;
        if (c == 'R' || c == 'Y' || c == 'G') {
            result_array[idx++] = c;
        } else {
            invalid = true;
        }
		//idx = 0;
		
    }

    //if (idx == 0) return -2;      // empty
    if (invalid) return -3;       // invalid character
    return 0;                     // success
}



int time_parse(char *time) {
	if(strlen(time) != 6) {
		return TIME_ARRAY_ERROR;
	}
	
	int test = checkChars(time);
	if(test != 0) {
		return test;
	}
	
	// Parse without modifying the string
	int h = (time[0] - '0') * 10 + (time[1] - '0');
	int m = (time[2] - '0') * 10 + (time[3] - '0');
	int s = (time[4] - '0') * 10 + (time[5] - '0');
	
	if(h > 23 || m > 59 || s > 59) {
		return TIME_VALUE_ERROR;
	}
	
	int total_seconds = (h * 60 * 60) + (m * 60) + s;
	
	k_timer_init(&timer, timer_handler, NULL);
	k_timer_start(&timer, K_SECONDS(total_seconds), K_NO_WAIT);
	
	return total_seconds;
}


void timer_handler(struct k_timer *timer_id) {
	
		debug_log("Timer handler");
		k_condvar_signal(&red_signal);
		debug_log("Timer done");
}

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
	//printk("\npower: %d", res);
	debug_log("power: %d", res);
	return res;
}
// Muutettu if else joka tulosti monesti aiheuttaen extra printtiä (ei toiminut oikein)
int checkChars(char *characters) {
	if(characters == NULL) {
		return CHARACTERS_NULL_ERROR;
	}
	
	// Check that every character is a digit
	for(int i = 0; i < strlen(characters); i++) {
		char c = characters[i];
		if(c < '0' || c > '9') {
			return WRONG_CHARS_ERROR;
		}
	}
	return 0;  // All valid
}

char checkIfNumber(char character) {
	switch(character) {
		case '0': return '0';
		case '1': return '1';
		case '2': return '2';
		case '3': return '3';
		case '4': return '4';
		case '5': return '5';
		case '6': return '6';
		case '7': return '7';
		case '8': return '8';
		case '9': return '9';
		case 'A': return true;
		case 'R': return true;
		case 'Y': return true;
		case 'G': return true;
		case 'T': return true;
		case 'J': return true;
		case 'L': return true;
		case 'M': return true;
		case 'N': return true;
		case 'D':return true;
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
		//debug_log("number: %d", number);
		position--;
	}
	//printk("\nnumber: %d", number);
	//debug_log("number: %d", number);
	return number;
}

int changeToNumber(char character) {
	int number = 0;
	switch(character) {
	case '0': number = 0; break;
	case '1': number = 1; break;
	case '2': number = 2; break;
	case '3': number = 3; break;
	case '4': number = 4; break;
	case '5': number = 5; break;
	case '6': number = 6; break;
	case '7': number = 7; break;
	case '8': number = 8; break;
	case '9': number = 9; break;
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
		if(location[i] == 'r' || location[i] == 'g' || location[i] == 'y' ||
		   location[i] == 'R' || location[i] == 'G' || location[i] == 'Y') {
			len[position] = location[i];
			position++; 
		}
		else if(location[i] == ',' || location[i] == 'T') {
			continue;
		}
		else {
			if(len[position-1] == 'r' || len[position-1] == 'R') {
				if(location[i] != 't' && location[i] != 'T') {
					r_str[count] = checkIfNumber(location[i]);
					if(strlen(r_str) > 0) { r_delay = 0; }
					count++;
				}
			}
			else if(len[position-1] == 'y' || len[position-1] == 'Y') {
				if(location[i] != 't' && location[i] != 'T') {
					y_str[count] = checkIfNumber(location[i]);
					if(strlen(y_str) > 0) { y_delay = 0; }
					count++;
				}
			}
			else if(len[position-1] == 'g' || len[position-1] == 'G') {
				if(location[i] != 't' && location[i] != 'T') {
					g_str[count] = checkIfNumber(location[i]);
					if(strlen(y_str) > 0) { g_delay=0; }
					count++;
				}
			}
		}
	}
	if(r_delay == 0) {
		//printk("r_str: %s", r_str);
		//debug_log("r_str: %s", r_str);
		r_delay = transformNumber(r_str);
		//printk("r_delay: %d", r_delay);		
		//debug_log("r_delay: %d", r_delay);		
	}
	if(y_delay == 0) {
		y_delay = transformNumber(y_str);
	}
	if(g_delay == 0) {
		g_delay = transformNumber(g_str);
	}
		
	//printk("data: %s\n", len);
	//debug_log("data: %s", len);
	strcpy(sequence_split,len);
	//printk("r_str: %s | y_str: %s | g_str: %s", r_str, y_str, g_str);
	//debug_log("r_str: %s | y_str: %s | g_str: %s", r_str, y_str, g_str);
}

/********************
 * init UART
 */
int init_uart(void) {
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
    char rc = 0;
    char uart_msg[64];
    memset(uart_msg, 0, sizeof(uart_msg));
    int uart_msg_cnt = 0;

    while (true) {
        if (uart_poll_in(uart_dev, &rc) == 0) {
            debug_log("Received: %c", rc);

            if (rc == 'X') {
                // terminate message
                uart_msg[uart_msg_cnt] = '\0';
                debug_log("UART msg: %s", uart_msg);

                struct data_t *buf = k_malloc(sizeof(struct data_t));
                if (buf) {
                    memset(buf, 0, sizeof(struct data_t));
                    strncpy(buf->msg, uart_msg, sizeof(buf->msg)-1);
                    k_fifo_put(&dispatcher_fifo, buf);
                }

                // reset buffer
                uart_msg_cnt = 0;
                memset(uart_msg, 0, sizeof(uart_msg));
            }
            else if (rc != '\r' && uart_msg_cnt < (int)sizeof(uart_msg)-1) {
                uart_msg[uart_msg_cnt++] = rc;
            }
        }
        k_msleep(10);
    }
}


/********************
 * Dispatcher task
 */
static void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
	
    char sequence[20];

    while (true) {
        if (paused) {
            k_msleep(100);
            continue;
        }

        struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_MSEC(10));
        if (rec_item == NULL) {
            k_msleep(10);
            continue;
        }

      /*  memcpy(sequence, rec_item->msg, 20);
        k_free(rec_item);
		memset(result_array, 0, sizeof(result_array));*/
		// copy safely and ensure NUL termination
		strncpy(sequence, rec_item->msg, sizeof(sequence) - 1);
		sequence[sizeof(sequence) - 1] = '\0';
		k_free(rec_item);

		// clear previous result so old data doesn't leak into replies
		memset(result_array, 0, sizeof(result_array));
		//printk("Dispatcher received: %s", sequence);
		if (isdigit(sequence[0])) {
				int val = time_parse(sequence);
				printk("%dX", val);
				continue; // skip rest of the dispatcher loop
			}

        if (strlen(sequence) == 0) {
            continue; // ignore empty
        }

		char first = toupper((unsigned char)sequence[0]);

		if (first == 'A') {
			int rval = sequence_check(sequence);
			if (rval == 0) {
				printk("%sX", result_array);
			} else if (rval == -3) {

				// #define SEQUENCE_ERROR	-6
				//#define SEQUENCE_FAILED -7
				printk("%dX", SEQUENCE_ERROR);
			} else if (rval == -7) {
				printk("%dX", SEQUENCE_FAILED);
			}
		}
		 
	/*	else {
			int tval = time_parse(sequence);
			// time_parse returns positive seconds or negative error codes
			printk("%dX", tval);
		}*/

		sequence_splitting(sequence);

		if(sequence[0] != 't' ) {
			strncpy(run_sequence, sequence_split, 20);
		}

		if(Transient == 1) {
			debug_log("Running Transient.");
		}

		for (int i = 0; i <= strlen(sequence); i++) {
			char c = toupper((unsigned char)sequence[i]);
			
			if (c == 'R') {
				debug_log("Dispatcher: RED signal");
				k_mutex_lock(&red_mutex, K_FOREVER);
				k_condvar_signal(&red_signal);
				k_mutex_unlock(&red_mutex);
                k_condvar_wait(&red_ready_signal, &red_ready_mutex, K_FOREVER);
			}
			else if (c == 'Y') {
				debug_log("Dispatcher: YELLOW signal");
				k_mutex_lock(&yellow_mutex, K_FOREVER);
				k_condvar_signal(&yellow_signal);
				k_mutex_unlock(&yellow_mutex);
                k_condvar_wait(&yellow_ready_signal, &yellow_ready_mutex, K_FOREVER);
			}
			else if (c == 'G') {
				debug_log("Dispatcher: GREEN signal");
				k_mutex_lock(&green_mutex, K_FOREVER);
				k_condvar_signal(&green_signal);
				k_mutex_unlock(&green_mutex);
                k_condvar_wait(&green_ready_signal, &green_ready_mutex, K_FOREVER);
			}
			else if(c == 'J') {
    			debug_log("Dispatcher: Button 0 pressed -> %s", paused ? "PAUSED" : "RUNNING");
			}
			else if(c == 'T') {
				if(Transient == 0) {
					Transient = 1;
				}
			}
			else if(c == 'L') {
				debug_log("Dispatcher: button 2 pressed.");
			}
			else if(c == 'M') {
				debug_log("Dispatcher: button 3 pressed.");
			}
			else if(c == 'N') {
				debug_log("Dispatcher: button 4 pressed.");
			}
			else if (c == 'D') {
				debug_enabled = !debug_enabled;
				if (debug_enabled) {
					debug_log("DEBUGGING ENABLED");
				} else {
					debug_log("DEBUGGING DISABLED");
				}
			}	
		}

		while(Transient == 1) {
			struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_MSEC(200));
			if (rec_item == NULL) {
				k_msleep(10);
				continue;
			}
			
			memcpy(sequence, rec_item->msg, 20);
			k_free(rec_item);

			int check = time_parse(sequence);
			if(check > 0) {
				debug_log("Time_Parse ok.");
			}
			if(sequence[0] == 't' || sequence[0] == 'T') {
				Transient = 0;
				break;
			}
			debug_log("Running task:");
			
			int size = strlen(run_sequence);
			for(int i = 0; i < size ;i++) {
				if(run_sequence[i] == 'R' || run_sequence[i] == 'r') {
					debug_log("red");
					k_mutex_lock(&red_mutex, K_FOREVER);
					k_condvar_signal(&red_signal);
					k_mutex_unlock(&red_mutex);
					k_condvar_wait(&red_ready_signal, &red_ready_mutex, K_FOREVER);
				}
				else if(run_sequence[i] == 'y' || run_sequence[i] == 'Y') { 
					debug_log("yellow");
					k_mutex_lock(&yellow_mutex, K_FOREVER);
					k_condvar_signal(&yellow_signal);
					k_mutex_unlock(&yellow_mutex);
					k_condvar_wait(&yellow_ready_signal, &yellow_ready_mutex, K_FOREVER);
				}
				else if(run_sequence[i] == 'g' || run_sequence[i] == 'G') { 
					debug_log("green");
					k_mutex_lock(&green_mutex, K_FOREVER);
					k_condvar_signal(&green_signal);
					k_mutex_unlock(&green_mutex);
					k_condvar_wait(&green_ready_signal, &green_ready_mutex, K_FOREVER);
				}
			}
			k_yield();
		}
	}
}

K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(debug_thread, STACKSIZE, debug_task, NULL, NULL, NULL, PRIORITY, 0, 0);
