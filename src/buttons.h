#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>


// button pin configurations
// #define BUTTON DT_ALIAS(sw)
#define BUTTON_0 DT_ALIAS(sw0)
#define BUTTON_1 DT_ALIAS(sw1)
#define BUTTON_2 DT_ALIAS(sw2)
#define BUTTON_3 DT_ALIAS(sw3)
#define BUTTON_4 DT_ALIAS(sw4)

//extern struct k_fifo dispatcher_fifo;

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

void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	paused = !paused;  // toggle pause
	char stop_msg[20];

	// printk("Button pressed\n");
	debug_log("Button pressed\n");
	struct data_t *buf = k_malloc(sizeof(struct data_t));
	if (!buf) {
    	// printk("Memory alloc failed\n");
    	debug_log("Memory alloc failed\n");
    	return;
	}
	memset(stop_msg, 0, sizeof(struct data_t));
	strncpy(stop_msg, "j\r", 3);
	strncpy(buf->msg, stop_msg, sizeof(buf->msg) - 1);
	buf->msg[sizeof(buf->msg) - 1] = '\0';

	k_fifo_put(&dispatcher_fifo, buf);
	// printk("Button 0 handler: %s \n", stop_msg);
	debug_log("Button 0 handler: %s \n", stop_msg);
	
}
void button_1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	char stop_msg[20];
	// printk("Button 1 pressed\n");
	debug_log("Button 1 pressed\n");
	struct data_t *buf = k_malloc(sizeof(struct data_t));
	if (!buf) {
    	// printk("Memory alloc failed\n");
    	debug_log("Memory alloc failed\n");
    	return;
	}
	memset(stop_msg, 0, sizeof(struct data_t));
	strncpy(stop_msg, "k\r", 3);
	strncpy(buf->msg, stop_msg, sizeof(buf->msg) - 1);
	buf->msg[sizeof(buf->msg) - 1] = '\0';

	k_fifo_put(&dispatcher_fifo, buf);
}
void button_2_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	char stop_msg[20];
	// printk("Button 2 pressed\n");
	debug_log("Button 2 pressed\n");
	struct data_t *buf = k_malloc(sizeof(struct data_t));
	if (!buf) {
    	// printk("Memory alloc failed\n");
    	debug_log("Memory alloc failed\n");
    	return;
	}
	memset(stop_msg, 0, sizeof(struct data_t));
	strncpy(stop_msg, "l\r", 3);
	strncpy(buf->msg, stop_msg, sizeof(buf->msg) - 1);
	buf->msg[sizeof(buf->msg) - 1] = '\0';

	k_fifo_put(&dispatcher_fifo, buf);
}
void button_3_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	char stop_msg[20];
	// printk("Button 3  pressed\n");
	debug_log("Button 3  pressed\n");
	struct data_t *buf = k_malloc(sizeof(struct data_t));
	if (!buf) {
    	// printk("Memory alloc failed\n");
    	debug_log("Memory alloc failed\n");
    	return;
	}
	memset(stop_msg, 0, sizeof(struct data_t));
	strncpy(stop_msg, "m\r", 3);
	strncpy(buf->msg, stop_msg, sizeof(buf->msg) - 1);
	buf->msg[sizeof(buf->msg) - 1] = '\0';

	k_fifo_put(&dispatcher_fifo, buf);
}
void button_4_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	char stop_msg[20];
	// printk("Button 4  pressed\n");
	debug_log("Button 4  pressed\n");
	struct data_t *buf = k_malloc(sizeof(struct data_t));
	if (!buf) {
    	// printk("Memory alloc failed\n");
    	debug_log("Memory alloc failed\n");
    	return;
	}
	memset(stop_msg, 0, sizeof(struct data_t));
	strncpy(stop_msg, "n\r", 3);
	strncpy(buf->msg, stop_msg, sizeof(buf->msg) - 1);
	buf->msg[sizeof(buf->msg) - 1] = '\0';

	k_fifo_put(&dispatcher_fifo, buf);
}



// Button initialization
int init_button() {
	// button 0
	int ret;
	if (!gpio_is_ready_dt(&button_0)) {
		// printk("Error: button 0 is not ready\n");
		debug_log("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_0, GPIO_INPUT);
	if (ret != 0) {
		// printk("Error: failed to configure pin\n");
		debug_log("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_0, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		// printk("Error: failed to configure interrupt on pin\n");
		debug_log("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	// button_1
	if (!gpio_is_ready_dt(&button_1)) {
		// printk("Error: button 1 is not ready\n");
		debug_log("Error: button 1 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_1, GPIO_INPUT);
	if (ret != 0) {
		// printk("Error: failed to configure pin\n");
		debug_log("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_1, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		// printk("Error: failed to configure interrupt on pin\n");
		debug_log("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	// button_2
	if (!gpio_is_ready_dt(&button_2)) {
		// printk("Error: button 0 is not ready\n");
		debug_log("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_2, GPIO_INPUT);
	if (ret != 0) {
		// printk("Error: failed to configure pin\n");
		debug_log("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_2, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		// printk("Error: failed to configure interrupt on pin\n");
		debug_log("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	// Button_3
	if (!gpio_is_ready_dt(&button_3)) {
		// printk("Error: button 0 is not ready\n");
		debug_log("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_3, GPIO_INPUT);
	if (ret != 0) {
		// printk("Error: failed to configure pin\n");
		debug_log("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_3, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		// printk("Error: failed to configure interrupt on pin\n");
		debug_log("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	//button_4
	if (!gpio_is_ready_dt(&button_4)) {
		// printk("Error: button 0 is not ready\n");
		debug_log("Error: button 0 is not ready\n");
		return -1;
	}

	ret = gpio_pin_configure_dt(&button_4, GPIO_INPUT);
	if (ret != 0) {
		// printk("Error: failed to configure pin\n");
		debug_log("Error: failed to configure pin\n");
		return -1;
	}

	ret = gpio_pin_interrupt_configure_dt(&button_4, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		// printk("Error: failed to configure interrupt on pin\n");
		debug_log("Error: failed to configure interrupt on pin\n");
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