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
struct debug_msg_t {
    void *fifo_reserved;     // 1st word reserved for FIFO
    char msg[128];           // debug message
};

K_FIFO_DEFINE(debug_fifo);

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
*/
/********************
 * Application code
 */

 //UART to robot_arto
// fixed the fixed the fixed sequence_check function
int sequence_check(char *run) {
    if (run == NULL) {
        return SEQUENCE_ERROR; // -6
    }

    int len = strlen(run);
    if (len < 2) {
        return SEQUENCE_FAILED; // -7
    }

    int result_idx = 0;
    bool invalid_char_found = false;
    memset(result_array, 0, sizeof(result_array));

    // Start at index 1 to skip first character (type identifier)
    for (int i = 1; i < len; i++) {
        char c = toupper((unsigned char)run[i]);

        if (c == 'R' || c == 'Y' || c == 'G') {
            result_array[result_idx++] = c;
        } else if (c == 'X') {
            break; // termination character
        } else {
            invalid_char_found = true; // mark invalid char
        }
    }

    if (result_idx == 0) {
        return SEQUENCE_FAILED; // nothing valid to execute
    }

    result_array[result_idx] = '\0';

    if (invalid_char_found) {
        return SEQUENCE_FAILED; // error code for invalid chars
    }

    return 0; // success
}
/*
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
*/

int time_parse(char *time) {
	char c=0;
	if(strlen(time) > 6) {
		//error = TIME_ARRAY_ERROR;
		//printk("%dX", error);
		return TIME_ARRAY_ERROR;
	}
	if(strlen(time) < 6) {
		//printk("%dX",TIME_ARRAY_ERROR);
		return TIME_ARRAY_ERROR;
	}
	// how many seconds, default returns error
	int seconds = TIME_LEN_ERROR;

	// TODO: Check that string is not null
	if(strlen(time) == 0) {
		//printk("%dX",TIME_ARRAY_ERROR);
		return TIME_ARRAY_ERROR;
	}
	// Parse values from time string
	// For example: 124033 -> 12hour 40min 33sec
    int test = checkChars(time);
	if(!test) {
		return test;
	}
	
	int values[3];
	values[2] = atoi(time+4); // seconds
	time[4] = 0;
	values[1] = atoi(time+2); // minutes
	time[2] = 0;
	values[0] = atoi(time); // hours
	// Now you have:
	// values[0] hour
	// values[1] minute
	// values[2] second
	if(values[0] > 59 || values[1] > 59 || values[2] > 23) {
		return TIME_VALUE_ERROR;
	}
	
	int hours = values[0] * 60 * 60;
	int minutes = values[1] * 60;
	seconds = values[2];
	seconds = hours + minutes + seconds;
	

	k_timer_init(&timer, timer_handler, NULL);
	k_timer_start(&timer, K_SECONDS(seconds), K_NO_WAIT); // Slingshot.

	// TODO: Add boundary check time values: below zero or above limit not allowed
	// limits are 59 for minutes, 23 for hours, etc

	// TODO: Calculate return value from the parsed minutes and seconds
	// Otherwise error will be returned!
	// seconds = ...

	return seconds;
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
	char rc=0;
	char uart_msg[20];
	memset(uart_msg,0,20);
	int uart_msg_cnt = 0;

	while (true) {
		if (uart_poll_in(uart_dev,&rc) == 0) {
			//printk("Received: %c\n",rc);
			debug_log("Received: %c", rc);

			if(rc == 'X') {	//debug_log("Ender received");
				struct data_t *buf = k_malloc(sizeof(struct data_t)-1);
				if (buf == NULL) {
					//printk("Memory alloc failed\n");
					debug_log("Memory alloc failed");
					continue;
				}
				memset(buf, 0, sizeof(struct data_t));
				strncpy(buf->msg, uart_msg, sizeof(buf->msg));
				//printk("UART: Sending to FIFO: '%s' (len=%d)", buf->msg, strlen(buf->msg));
				k_fifo_put(&dispatcher_fifo, buf);

				uart_msg_cnt = 0;
				memset(uart_msg,0,20);
			}
			else if (rc != '\r') {
    			uart_msg[uart_msg_cnt] = rc;
    			uart_msg_cnt++;
			}		
			/*if (rc != '\r' && rc != 'X') {
				uart_msg[uart_msg_cnt] = rc;
				uart_msg_cnt++;
			}*/ else 
			{
				//printk("UART msg: %s\n", uart_msg);
				debug_log("UART msg: %s", uart_msg);
                
				struct data_t *buf = k_malloc(sizeof(struct data_t));
				if (buf == NULL) {
					//printk("Memory alloc failed\n");
					debug_log("Memory alloc failed");
					continue;
				}
				memset(buf, 0, sizeof(struct data_t));
				strncpy(buf->msg, uart_msg, sizeof(buf->msg) - 1);

				k_fifo_put(&dispatcher_fifo, buf);

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
        	k_msleep(100);
        	continue;
    	}
		struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_MSEC(10));
		if (rec_item == NULL) {
			k_msleep(10);
			continue;
		}
		
		memcpy(sequence, rec_item->msg, 20);
		k_free(rec_item);
		
		// Route based on first character
		if (sequence[0] == 'A' || sequence[0] == 'a') {
			// Traffic sequence
			int rval = sequence_check(sequence);
			if (rval == 0) {
				printk("%sX", result_array);
			} else {
				printk("-6X");
			}
		}
		else {
			int check = time_parse(sequence);
			debug_log("DEinput '%s' len=%d", sequence, strlen(sequence));
			
			printk("%dX", check);
		}
		/*
		else {
			// Time string
			
			int check = time_parse(sequence);
			printk("%dX", check);
		}
		*/
		// Continue with the rest of your partner's code
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

 /*




static void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
	int check = 0;
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

		memcpy(sequence, rec_item->msg, 20);
		k_free(rec_item);

		// Skip if empty
		if (strlen(sequence) == 0) {
			debug_log(">>> Skipping empty sequence");
			continue;
		}

		debug_log(">>> Got: '%s' len=%d", sequence, strlen(sequence));
		// check if sequence

		// Determine routing: traffic sequence vs time string
		int rval = -1;
		check = 0;
		
		// Check if it's a traffic sequence (starts with 'A')
		if (sequence[0] == 'A' || sequence[0] == 'a') {
			rval = sequence_check(sequence);
			if (rval == 0) {
				// Valid traffic sequence
				debug_log("%sX", result_array);
				//printk("8X");

				check = -2;  // Skip time_parse
			}
			else {
				// Invalid sequence format
				//printk("%dX", rval);  // invalid sequence
				debug_log(result_array);
				check = -2;
			}
		}
		else {
			// Not a sequence, try as time string
			check = time_parse(sequence);
			printk("%dX", check); // returnaa??
			debug_log("Time result -> %dX", check);
		}
		
		// näitä oli monta
		/*if (check != -2) {
			printk("%dX", check);
		}

		if(check > 0) {
			debug_log("Time_Parse ok.");
		}

		
		sequence_splitting(sequence);

		
		/*bool sequence_is_valid = true;
		for (int i = 0; i < strlen(sequence); i++) {
    		char c = toupper((unsigned char)sequence[i]);
			if (c != 'R' && c != 'Y' && c != 'G') {
				sequence_is_valid = false;
				break;
			}
		}*/



		/*// Send result over UART
		for (int i = 0; i < strlen(result); i++) {
			uart_poll_out(uart_dev, result[i]);
		}
		uart_poll_out(uart_dev, 'X'); // termination 

		if(sequence[0] != 't' ) {
			strncpy(run_sequence, sequence_split, 20);
		}

		if(Transient == 1) {
			//printk("Running Transient.");
			debug_log("Running Transient.");
		}
		
		//printk("\nvalues: %s || r_delay: %d || y_delay: %d || g_delay: %d", sequence_split, r_delay, y_delay, g_delay);
		//debug_log("values: %s || r_delay: %d || y_delay: %d || g_delay: %d", sequence_split, r_delay, y_delay, g_delay);
		
		//printk("\nTransient: %d\n", Transient);
		//debug_log("Transient: %d", Transient);

		for (int i = 0; i <= strlen(sequence); i++) {
			char c = toupper((unsigned char)sequence[i]);
			//printk("character: %c", c);
			//debug_log("character: %c", c);
			
			if (c == 'R') {
				//printk("Dispatcher: RED signal\n");
				debug_log("Dispatcher: RED signal");
				k_mutex_lock(&red_mutex, K_FOREVER);
				k_condvar_signal(&red_signal);
				k_mutex_unlock(&red_mutex);
                k_condvar_wait(&red_ready_signal, &red_ready_mutex, K_FOREVER);
			}
			else if (c == 'Y') {
				//printk("Dispatcher: YELLOW signal\n");
				debug_log("Dispatcher: YELLOW signal");
				k_mutex_lock(&yellow_mutex, K_FOREVER);
				k_condvar_signal(&yellow_signal);
				k_mutex_unlock(&yellow_mutex);
                k_condvar_wait(&yellow_ready_signal, &yellow_ready_mutex, K_FOREVER);
			}
			else if (c == 'G') {
				//printk("Dispatcher: GREEN signal\n");
				debug_log("Dispatcher: GREEN signal");
				k_mutex_lock(&green_mutex, K_FOREVER);
				k_condvar_signal(&green_signal);
				k_mutex_unlock(&green_mutex);
                k_condvar_wait(&green_ready_signal, &green_ready_mutex, K_FOREVER);
			}
			else if(c == 'J') {
    			//printk("Dispatcher: Button 0 pressed -> %s\n", paused ? "PAUSED" : "RUNNING");
    			debug_log("Dispatcher: Button 0 pressed -> %s", paused ? "PAUSED" : "RUNNING");
			}
			else if(c == 'T') {
				if(Transient == 0) {
					Transient = 1;
				}
			}
			else if(c == 'L') {
				//printk("Dispatcher: button 2 pressed.\n");
				debug_log("Dispatcher: button 2 pressed.");
			}
			else if(c == 'M') {
				//printk("Dispatcher: button 3 pressed.\n");
				debug_log("Dispatcher: button 3 pressed.");
			}
			else if(c == 'N') {
				//printk("Dispatcher: button 4 pressed.\n");
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
			//printk("Running task:\n");
			debug_log("Running task:");
			
			int size = strlen(run_sequence);
			for(int i = 0; i < size ;i++) {
				if(run_sequence[i] == 'R' || run_sequence[i] == 'r') {
					//printk("red");
					debug_log("red");
					k_mutex_lock(&red_mutex, K_FOREVER);
					k_condvar_signal(&red_signal);
					k_mutex_unlock(&red_mutex);
					k_condvar_wait(&red_ready_signal, &red_ready_mutex, K_FOREVER);
				}
				else if(run_sequence[i] == 'y' || run_sequence[i] == 'Y') { 
					//printk("yellow");
					debug_log("yellow");
					k_mutex_lock(&yellow_mutex, K_FOREVER);
					k_condvar_signal(&yellow_signal);
					k_mutex_unlock(&yellow_mutex);
					k_condvar_wait(&yellow_ready_signal, &yellow_ready_mutex, K_FOREVER);
				}
				else if(run_sequence[i] == 'g' || run_sequence[i] == 'G') { 
					//printk("green");
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
*/
K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(debug_thread, STACKSIZE, debug_task, NULL, NULL, NULL, PRIORITY, 0, 0);
