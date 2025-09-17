#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

// Task declarations
void red_led_task(void *, void *, void *);
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
int init_led() {
    int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        //printk("Error: Led configure failed\n");
        debug_log("Error: Led configure failed");
        return ret;
    }

    int ret2 = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_ACTIVE);
    if (ret2 < 0) {
        //printk("Error: Led configure failed\n");
        debug_log("Error: Led configure failed");
        return ret2;
    }

    int ret3 = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
    if (ret3 < 0) {
        //printk("Error: Led configure failed\n");
        debug_log("Error: Led configure failed");
        return ret3;
    }

    gpio_pin_set_dt(&red,0);
    gpio_pin_set_dt(&blue,0);
    gpio_pin_set_dt(&green,0);

    //printk("Leds initialized ok\n");
    debug_log("Leds initialized ok");

    return 0;
}

// Task to handle red led
void red_led_task(void *, void *, void*) {
    //printk("Red led thread started\n");
    debug_log("Red led thread started");

    while(1) {
        k_condvar_wait(&red_signal, &red_mutex, K_FOREVER);

        if(manual == 1) {
            gpio_pin_set_dt(&red, 1);
        } else {
            gpio_pin_set_dt(&red,1);
            //printk("Red on\n");
            debug_log("Red on");

            k_sleep(K_SECONDS(1));

            gpio_pin_set_dt(&red,0);
            //printk("Red off\n");
            debug_log("Red off");
        }

        k_condvar_broadcast(&red_ready_signal);
    }
}

// Task to handle yellow led
void yellow_led_task(void *, void *, void*) {
    //printk("Yellow led thread started\n");
    debug_log("Yellow led thread started");

    while(1) {
        k_condvar_wait(&yellow_signal, &yellow_mutex, K_FOREVER);

        if(manual == 2) {
            gpio_pin_set_dt(&green,1);
            gpio_pin_set_dt(&red,1);
        }
        else if(manual == 4) {
            gpio_pin_set_dt(&green,1);
            gpio_pin_set_dt(&red,1);
            k_sleep(K_SECONDS(1));
            gpio_pin_set_dt(&green,0);
            gpio_pin_set_dt(&red,0);
            k_sleep(K_SECONDS(1));
        }
        else {
            gpio_pin_set_dt(&green,1);
            gpio_pin_set_dt(&red,1);
            //printk("yellow on\n");
            debug_log("Yellow on");

            k_sleep(K_SECONDS(1));

            gpio_pin_set_dt(&green,0);
            gpio_pin_set_dt(&red,0);
            //printk("yellow off\n");
            debug_log("Yellow off");
        }

        k_condvar_broadcast(&yellow_ready_signal);        
    }
}

// Task to handle green led
void green_led_task(void *, void *, void*) {
    //printk("Green led thread started\n");
    debug_log("Green led thread started");

    while(1) {
        k_condvar_wait(&green_signal, &green_mutex, K_FOREVER);

        if(manual == 3) {
            gpio_pin_set_dt(&green, 1);
        } else {
            gpio_pin_set_dt(&green,1);
            //printk("green on\n");
            debug_log("Green on");

            k_sleep(K_SECONDS(1));

            gpio_pin_set_dt(&green,0);
            //printk("green off\n");
            debug_log("Green off");
        }

        k_condvar_broadcast(&green_ready_signal);
    }
}
