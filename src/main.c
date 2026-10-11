/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/console/console.h>

#define LED0_NODE DT_ALIAS(led0)

#define CONTROL_STACK_SIZE 512
#define CONTROL_PRIORITY   5

#define MSGQ_MAX_MESSAGES  4

static const struct gpio_dt_spec led =
	GPIO_DT_SPEC_GET(LED0_NODE, gpios);


struct control_msg {
	uint32_t event_id;
	bool led_on;
};


K_MSGQ_DEFINE(control_msgq,
	      sizeof(struct control_msg),
	      MSGQ_MAX_MESSAGES,
	      4);


static void control_worker(void *arg1, void *arg2, void *arg3)
{
	struct control_msg msg;

	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	while (1) {

		k_msgq_get(&control_msgq, &msg, K_FOREVER);

		gpio_pin_set_dt(&led, msg.led_on);

		printf("[control] event %u -> LED %s\n",
		       msg.event_id,
		       msg.led_on ? "ON" : "OFF");
	}
}


K_THREAD_DEFINE(
	control_worker_id,
	CONTROL_STACK_SIZE,
	control_worker,
	NULL,
	NULL,
	NULL,
	CONTROL_PRIORITY,
	0,
	0
);


int main(void)
{
	int ret;
	uint32_t event_count = 0;
	char *line;

	struct control_msg msg;

	if (!gpio_is_ready_dt(&led)) {
		printf("GPIO device is not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

	if (ret < 0) {
		printf("Failed to configure LED GPIO\n");
		return 0;
	}

	console_getline_init();

	printf("\n");
	printf("UART Device Controller started\n");
	printf("Commands:\n");
	printf("  LED ON\n");
	printf("  LED OFF\n");

	while (1) {

		printf("> ");

		line = console_getline();

		if (strcmp(line, "LED ON") == 0) {

			event_count++;

			msg.event_id = event_count;
			msg.led_on = true;

		} else if (strcmp(line, "LED OFF") == 0) {

			event_count++;

			msg.event_id = event_count;
			msg.led_on = false;

		} else {

			printf("[command] invalid command: %s\n", line);
			continue;
		}

		ret = k_msgq_put(&control_msgq, &msg, K_NO_WAIT);

		if (ret != 0) {
			printf("[command] message queue full\n");
			continue;
		}

		printf("[command] event %u queued\n",
		       msg.event_id);
	}

	return 0;
}
