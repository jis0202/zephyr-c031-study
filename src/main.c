/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)

#define CONTROL_STACK_SIZE 512
#define CONTROL_PRIORITY   5

#define MSGQ_MAX_MESSAGES  4

static const struct gpio_dt_spec led =
	GPIO_DT_SPEC_GET(LED0_NODE, gpios);


/*
 * Thread 사이에서 전달할 실제 데이터
 */
struct control_msg {
	uint32_t event_id;
	bool led_on;
};


/*
 * control_msg 크기의 메시지를 최대 4개 저장할 수 있는 Queue
 */
K_MSGQ_DEFINE(control_msgq,
	      sizeof(struct control_msg),
	      MSGQ_MAX_MESSAGES,
	      4);


/*
 * Message Queue의 데이터를 받아 실제 HW를 제어하는 Thread
 */
static void control_worker(void *arg1, void *arg2, void *arg3)
{
	struct control_msg msg;

	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	while (1) {

		/*
		 * 메시지가 들어올 때까지 Block
		 */
		k_msgq_get(&control_msgq, &msg, K_FOREVER);

		/*
		 * Queue에서 받은 값으로 LED 제어
		 */
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
	bool next_led_state = true;

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

	printf("RTOS message queue test started\n");

	while (1) {

		k_msleep(1000);

		event_count++;

		/*
		 * 전달할 메시지 생성
		 */
		msg.event_id = event_count;
		msg.led_on = next_led_state;

		printf("[main] send event %u -> LED %s\n",
		       msg.event_id,
		       msg.led_on ? "ON" : "OFF");

		/*
		 * Queue에 메시지 전달
		 */
		ret = k_msgq_put(&control_msgq, &msg, K_NO_WAIT);

		if (ret != 0) {
			printf("[main] message queue full\n");
		} else {
			next_led_state = !next_led_state;
		}
	}

	return 0;
}
