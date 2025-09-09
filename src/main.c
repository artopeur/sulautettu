/* Arto Peurasaari ja Atte Kankkunen */
/* Viikon 2 tehtävä kuvauksessa osa 1 tehty*/
/* Viikon 2 tehtävän kuvauksen keskeytys +1p tehty*/
/* viikon 2 tehtävän lisäpiste2 vielä tekemättä.*/

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>


// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

// button pin configurations
// #define BUTTON DT_ALIAS(sw)
#define BUTTON_0 DT_ALIAS(sw0)
#define BUTTON_1 DT_ALIAS(sw1)
#define BUTTON_2 DT_ALIAS(sw2)
#define BUTTON_3 DT_ALIAS(sw3)
#define BUTTON_4 DT_ALIAS(sw4)


static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET_OR(BUTTON_0, gpios, {0});
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET_OR(BUTTON_1, gpios, {1});
static const struct gpio_dt_spec button_2 = GPIO_DT_SPEC_GET_OR(BUTTON_2, gpios, {2});
static const struct gpio_dt_spec button_3 = GPIO_DT_SPEC_GET_OR(BUTTON_3, gpios, {3});
static const struct gpio_dt_spec button_4 = GPIO_DT_SPEC_GET_OR(BUTTON_4, gpios, {4});

static struct gpio_callback button_0_data;
static struct gpio_callback button_1_data;
static struct gpio_callback button_2_data;
static struct gpio_callback button_3_data;
static struct gpio_callback button_4_data;

// Red led thread initialization
#define STACKSIZE 500
#define PRIORITY 5

int led_state = 0;
int prev = 0;

void red_led_task(void *, void *, void*);
void blue_led_task(void *, void *, void *);
void green_led_task(void *, void *, void *);

K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(blue_thread,STACKSIZE,blue_led_task,NULL,NULL,NULL,PRIORITY,0,0);

// Main program
int main(void)
{
	init_led();
	int ret = init_button();
	if(ret < 0) {
		return 0;
	}
	while(1) {
		k_msleep(10);
	}

	return 0;
}

void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button pressed\n");
	if(led_state == 4) {
		led_state = prev;
	}
	else {
		prev = led_state;
		led_state = 4;
	}
}
void button_1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button 1 pressed\n");
	if(led_state == 4) {
		led_state = prev;
	}
	else {
		prev = led_state;
		led_state = 4;
	}
}
void button_2_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button 2 pressed\n");
	if(led_state == 4) {
		led_state = prev;
	}
	else {
		prev = led_state;
		led_state = 4;
	}
}
void button_3_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button 3  pressed\n");
	if(led_state == 4) {
		led_state = prev;
	}
	else {
		prev = led_state;
		led_state = 4;
	}
}
void button_4_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button 4  pressed\n");
	if(led_state == 4) {
		led_state = prev;
	}
	else {
		prev = led_state;
		led_state = 4;
	}
}

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
	
	return 0;
}

// Button initialization
int init_button() {
	// button 0
	int ret;
	if (!gpio_is_ready_dt(&button_0)) {
		printk("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_0, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_0, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	// button_1
	if (!gpio_is_ready_dt(&button_1)) {
		printk("Error: button 1 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_1, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	// button_2
	if (!gpio_is_ready_dt(&button_2)) {
		printk("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_2, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_2, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	// Button_3
	if (!gpio_is_ready_dt(&button_3)) {
		printk("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_3, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_3, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	//button_4
	if (!gpio_is_ready_dt(&button_4)) {
		printk("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_4, GPIO_INPUT);
	if (ret != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_4, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}



	gpio_init_callback(&button_0_data, button_0_handler, BIT(button_0.pin));
	gpio_add_callback(button_0.port, &button_0_data);

	gpio_init_callback(&button_1_data, button_1_handler, BIT(button_1.pin));
	gpio_add_callback(button_1.port, &button_1_data);

	gpio_init_callback(&button_2_data, button_2_handler, BIT(button_2.pin));
	gpio_add_callback(button_2.port, &button_2_data);

	gpio_init_callback(&button_3_data, button_3_handler, BIT(button_3.pin));
	gpio_add_callback(button_3.port, &button_3_data);

	gpio_init_callback(&button_4_data, button_4_handler, BIT(button_4.pin));
	gpio_add_callback(button_4.port, &button_4_data);
	printk("Set up buttons ok\n");
	
	return 0;
}


// Task to handle red led
void red_led_task(void *, void *, void*) {
	printk("Red led thread started\n");
	while(1) {
		if(led_state == 0) {
			// 1. set led on 
			gpio_pin_set_dt(&red,1);
			printk("Red on\n");
			// 2. sleep for 2 seconds
			k_sleep(K_SECONDS(1));
			// 3. set led off
			gpio_pin_set_dt(&red,0);
			printk("Red off\n");
			// 4. sleep for 2 seconds
			k_sleep(K_SECONDS(1));
			if(led_state == 4) {
				k_yield();
			}
			else {
				led_state=1;
				prev = 0;
			}
				
		}
	k_yield();	
	}
	
}

// Task to handle yellow, with blue and green leds
void blue_led_task(void *, void *, void*) {
	
	printk("Yellow led thread started\n");
	while (true) {
		if(led_state == 1) {
			
			// 1. set led on 
			gpio_pin_set_dt(&green,1);
			gpio_pin_set_dt(&red,1);
			printk("yellow on\n");
			// 2. sleep for 2 seconds
			k_sleep(K_SECONDS(1));
			// 3. set led off
			gpio_pin_set_dt(&green,0);
			gpio_pin_set_dt(&red,0);
			printk("yellow off\n");
			// 4. sleep for 2 seconds
			k_sleep(K_SECONDS(1));
			if(led_state == 4) {
				k_yield();
			}
			else {
				if(prev ==0) {
					led_state = 2;
				}
				else {
					led_state = 0;
				}
			}
		}
		k_yield();
	}
}

// Task to handle green led
void green_led_task(void *, void *, void*) {
	
	printk("Red led thread started\n");
	while (true) {
		if(led_state == 2) {

		
			// 1. set led on 
			gpio_pin_set_dt(&green,1);
			printk("green on\n");
			// 2. sleep for 2 seconds
			k_sleep(K_SECONDS(1));
			// 3. set led off
			gpio_pin_set_dt(&green,0);
			printk("green off\n");
			// 4. sleep for 2 seconds
			k_sleep(K_SECONDS(1));
			if(led_state == 4) {
				k_yield();
			}
			else {
				led_state=1;
				prev = 2;
			}
			
		}

		k_yield();
	}
}

