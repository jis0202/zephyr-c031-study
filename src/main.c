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

#define MSGQ_MAX_MESSAGES      4
#define AUTO_STATUS_PERIOD_SEC 5

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
 * Application의 공유 Device State
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
 * 공유 Device State 보호
 */
K_MUTEX_DEFINE(state_mutex);


/*
 * Main Thread -> Control Thread 명령 전달
 */
K_MSGQ_DEFINE(control_msgq,
	      sizeof(struct control_msg),
	      MSGQ_MAX_MESSAGES,
	      4);


/*
 * Device State를 안전하게 snapshot으로 복사
 */
static void get_state_snapshot(struct app_device_state *snapshot)
{
	k_mutex_lock(&state_mutex, K_FOREVER);

	*snapshot = state;

	k_mutex_unlock(&state_mutex);
}


/*
 * Device State 출력
 *
 * Mutex 안에서는 state 복사만 하고,
 * printf는 Mutex 해제 후 수행한다.
 */
static void print_device_status(const char *source)
{
	struct app_device_state snapshot;

	get_state_snapshot(&snapshot);

	printf("[%s]\n", source);
	printf("  LED: %s\n",
	       snapshot.led_on ? "ON" : "OFF");
	printf("  Last Event: %u\n",
	       snapshot.last_event_id);
}


/*
 * System Workqueue에서 실행되는 Work Handler
 *
 * Thread context이므로 Mutex와 printf 사용 가능
 */
static void status_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);

	print_device_status("auto status");
}


/*
 * Work Item 정의
 */
K_WORK_DEFINE(status_work, status_work_handler);


/*
 * Timer expiry callback
 *
 * Interrupt context이므로 실제 STATUS 처리를 하지 않고
 * Work Item만 System Workqueue에 제출한다.
 */
static void status_timer_handler(struct k_timer *timer)
{
	ARG_UNUSED(timer);

	(void)k_work_submit(&status_work);
}


/*
 * Periodic Timer 정의
 */
K_TIMER_DEFINE(status_timer, status_timer_handler, NULL);


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

		k_msgq_get(&control_msgq, &msg, K_FOREVER);

		gpio_pin_set_dt(&led, msg.led_on);

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
	printf("Auto status period: %d sec\n",
	       AUTO_STATUS_PERIOD_SEC);

	/*
	 * 첫 5초 후 실행,
	 * 이후 5초마다 반복
	 */
	k_timer_start(&status_timer,
		      K_SECONDS(AUTO_STATUS_PERIOD_SEC),
		      K_SECONDS(AUTO_STATUS_PERIOD_SEC));

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

			print_device_status("status");
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
