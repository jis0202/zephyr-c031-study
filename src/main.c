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


/*
 * Control Thread로 전달할 명령 데이터
 */
struct control_msg {
	uint32_t event_id;
	bool led_on;
};


/*
 * Device의 현재 상태
 *
 * Main Thread와 Control Thread가 함께 접근하므로
 * Mutex로 보호한다.
 */
struct app_device_state {
	bool led_on;
	uint32_t last_event_id;
};


static struct app_device_state state = {
	.led_on = false,
	.last_event_id = 0,
};


/*
 * 공유 Device State 보호용 Mutex
 */
K_MUTEX_DEFINE(state_mutex);


/*
 * Command -> Control Thread 메시지 전달용 Queue
 */
K_MSGQ_DEFINE(control_msgq,
	      sizeof(struct control_msg),
	      MSGQ_MAX_MESSAGES,
	      4);


/*
 * 실제 Hardware Control 담당 Thread
 */
static void control_worker(void *arg1, void *arg2, void *arg3)
{
	struct control_msg msg;

	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	while (1) {

		/*
		 * 명령이 들어올 때까지 Block
		 */
		k_msgq_get(&control_msgq, &msg, K_FOREVER);

		/*
		 * 실제 Hardware 제어
		 */
		gpio_pin_set_dt(&led, msg.led_on);

		/*
		 * 공유 Device State 갱신
		 */
		k_mutex_lock(&state_mutex, K_FOREVER);

		state.led_on = msg.led_on;
		state.last_event_id = msg.event_id;

		k_mutex_unlock(&state_mutex);

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
	struct app_device_state snapshot;

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
	printf("  STATUS\n");

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

		} else if (strcmp(line, "STATUS") == 0) {

			/*
			 * 공유 상태를 읽는 동안 Mutex 획득
			 */
			k_mutex_lock(&state_mutex, K_FOREVER);

			snapshot = state;

			k_mutex_unlock(&state_mutex);

			/*
			 * printf는 Mutex 밖에서 수행
			 */
			printf("[status]\n");
			printf("  LED: %s\n",
			       snapshot.led_on ? "ON" : "OFF");
			printf("  Last Event: %u\n",
			       snapshot.last_event_id);

			continue;

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
