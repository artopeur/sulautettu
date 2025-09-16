#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/timing/timing.h>


void red_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void *);
void green_led_task(void *, void *, void *);

K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);

// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

//FUNCTIONS DECLARATION
int init_led();

// Initialize leds
int  init_led() {

	// Led pin initialization
	int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: Led configure failed\n");		
		return ret;
	}
	int ret2 = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_ACTIVE);
	if (ret2 < 0) {
		printk("Error: Led configure failed\n");		
		return ret2;
	}
	int ret3 = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
	if (ret3 < 0) {
		printk("Error: Led configure failed\n");		
		return ret3;
	}
	// set led off
	gpio_pin_set_dt(&red,0);
	// set led off
	gpio_pin_set_dt(&blue,0);
	// set led off
	gpio_pin_set_dt(&green,0);

	printk("Leds initialized ok\n");
	
    timing_init();
    printk("timing initialized.\n");
	return 0;
}


// Task to handle red led
void red_led_task(void *, void *, void*) {
	printk("Red led thread started\n");

    while(1) {
        k_condvar_wait(&red_signal, &red_mutex, K_FOREVER);
        timing_start();
		timing_t red_start_time = timing_counter_get(); 

        // 1. set led on 
        gpio_pin_set_dt(&red,1);
        //printk("Red on\n");
        // 2. sleep for 2 seconds
        k_sleep(K_SECONDS(1));
        // 3. set led off
        gpio_pin_set_dt(&red,0);
        //printk("Red off\n");

        k_msleep(100); //comment this for debugging
            
        k_condvar_broadcast(&red_ready_signal);
        timing_t red_end_time = timing_counter_get();
		timing_stop();
    	uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&red_start_time, &red_end_time));
		printk("Red task: %lld\n", timing_ns /1000); 
    }

}

// Task to handle yellow, with blue and green leds
void yellow_led_task(void *, void *, void*) {
    printk("Yellow led thread started\n");
    
    while(1) {
        
        k_condvar_wait(&yellow_signal, &yellow_mutex, K_FOREVER);
        timing_start();
		timing_t yellow_start_time = timing_counter_get();
        // 1. set led on 
        gpio_pin_set_dt(&green,1);
        gpio_pin_set_dt(&red,1);
        //printk("yellow on\n");
        // 2. sleep for 2 seconds
        k_sleep(K_SECONDS(1));
        // 3. set led off
        gpio_pin_set_dt(&green,0);
        gpio_pin_set_dt(&red,0);
        //printk("yellow off\n");

        k_msleep(100); //comment this for debugging
            
        k_condvar_broadcast(&yellow_ready_signal);	
        timing_t yellow_end_time = timing_counter_get();
		timing_stop();
    	uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&yellow_start_time, &yellow_end_time));
		printk("yellow task: %lld\n", timing_ns / 1000);	
    }
}

// Task to handle green led
void green_led_task(void *, void *, void*) {
	printk("Green led thread started\n");
    while(1) {
        
        k_condvar_wait(&green_signal, &green_mutex, K_FOREVER);
        timing_start();
		timing_t green_start_time = timing_counter_get();

        // 1. set led on 
        gpio_pin_set_dt(&green,1);
        //printk("green on\n");
        // 2. sleep for 2 seconds
        k_sleep(K_SECONDS(1));
        // 3. set led off
        gpio_pin_set_dt(&green,0);
        //printk("green off\n");    
        k_msleep(100);  //comment this for debugging
        
        k_condvar_broadcast(&green_ready_signal);
        timing_t green_end_time = timing_counter_get();
		timing_stop();
    	uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&green_start_time, &green_end_time));
		printk("green task: %lld\n", timing_ns /1000);	
    }
}