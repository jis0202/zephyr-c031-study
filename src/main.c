/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)

#define WORKER_STACK_SIZE 512
#define WORKER_PRIORITY   5

static const struct gpio_dt_spec led =
	GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/*
 * Initial count = 0
 * Maximum count = 1
 *
 * Worker thread waits until main thread gives this semaphore.
 */
K_SEM_DEFINE(led_sem, 0, 1);

static void led_worker(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	while (1) {
		k_sem_take(&led_sem, K_FOREVER);

		gpio_pin_toggle_dt(&led);

		printf("[worker] LED toggled\n");
	}
}

K_THREAD_DEFINE(
	led_worker_id,
	WORKER_STACK_SIZE,
	led_worker,
	NULL,
	NULL,
	NULL,
	WORKER_PRIORITY,
	0,
	0
);

int main(void)
{
	int ret;
	uint32_t event_count = 0;

	if (!gpio_is_ready_dt(&led)) {
		printf("GPIO device is not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

	if (ret < 0) {
		printf("Failed to configure LED GPIO\n");
		return 0;
	}

	printf("RTOS semaphore test started\n");

	while (1) {
		k_msleep(1000);

		event_count++;

		printf("[main] event %u\n", event_count);

		k_sem_give(&led_sem);
	}

	return 0;
}
