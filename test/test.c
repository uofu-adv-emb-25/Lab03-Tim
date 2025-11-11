#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include <unity.h>
#include "threadLock.h"

#define TEST_RUNNER_PRIORITY ( tskIDLE_PRIORITY + 5UL )
#define LEFT_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define LEFT_TASK_PRIORITY ( TEST_RUNNER_PRIORITY - 1UL )
#define RIGHT_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define RIGHT_TASK_PRIORITY ( TEST_RUNNER_PRIORITY - 1UL )


void setUp(void) {}

void tearDown(void) {}

void test_semaphore_taken(void)
{
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1,1);
    int counter = 0;
    xSemaphoreTake(semaphore, portMAX_DELAY);
    int result = do_loop(semaphore, &counter, "test", 10);
    TEST_ASSERT_EQUAL_INT(pdFALSE, result);
    TEST_ASSERT_EQUAL_INT(0, counter);
}

void test_semaphore_free(void)
{
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1,1);
    int counter = 0;
    int result = do_loop(semaphore, &counter, "test", 10);
    TEST_ASSERT_EQUAL_INT(pdTRUE, result);
    TEST_ASSERT_EQUAL_INT(1, counter);
}

#define LEFT_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define LEFT_TASK_PRIORITY ( TEST_RUNNER_PRIORITY - 1UL )
#define RIGHT_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define RIGHT_TASK_PRIORITY ( TEST_RUNNER_PRIORITY - 1UL )

void test_deadlock(void){
    TaskHandle_t aThread, bThread, supThread;
    SemaphoreHandle_t semaphoreA = xSemaphoreCreateCounting(1,1);
    SemaphoreHandle_t semaphoreB = xSemaphoreCreateCounting(1,1);
    struct DeadlockArgs left_args = {semaphoreA, semaphoreB, 0, 'a'};
    struct DeadlockArgs right_args = {semaphoreB, semaphoreA, 7, 'b'};
    BaseType_t left_status =
        xTaskCreate(deadlock, "Left", LEFT_TASK_STACK_SIZE,
                    (void *)&left_args, LEFT_TASK_PRIORITY, &aThread);
    BaseType_t right_status =
        xTaskCreate(deadlock, "Right", RIGHT_TASK_STACK_SIZE,
                    (void *)&right_args, RIGHT_TASK_PRIORITY, &bThread);
    vTaskDelay(1000);
    TEST_ASSERT_EQUAL_INT(uxSemaphoreGetCount(semaphoreA), 0);
    TEST_ASSERT_EQUAL_INT(uxSemaphoreGetCount(semaphoreB), 0);
    TEST_ASSERT_EQUAL_INT(2, left_args.counter);
    TEST_ASSERT_EQUAL_INT(9, right_args.counter);
    printf("- Killing threads\n");
    vTaskDelete(aThread);
    vTaskDelete(bThread);
    printf("- Killed threads\n");

}

void test_orphan_lock(void){
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1,1);
    int counter = 1;
    int result = orphan_lock(semaphore, &counter, 500);
    TEST_ASSERT_EQUAL_INT(2, counter);
    TEST_ASSERT_EQUAL_INT(pdTRUE, result);
    TEST_ASSERT_EQUAL_INT(1, uxSemaphoreGetCount(semaphore));

    result = orphan_lock(semaphore, &counter, 500);
    TEST_ASSERT_EQUAL_INT(3, counter);
    TEST_ASSERT_EQUAL_INT(pdFALSE, result);
    TEST_ASSERT_EQUAL_INT(0, uxSemaphoreGetCount(semaphore));

}

void test_unorphan_lock(void){
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1,1);
    int counter = 1;
    int result = unorphan_lock(semaphore, 500, &counter);
    TEST_ASSERT_EQUAL_INT(2, counter);
    TEST_ASSERT_EQUAL_INT(pdTRUE, result);
    TEST_ASSERT_EQUAL_INT(1, uxSemaphoreGetCount(semaphore));

    result = unorphan_lock(semaphore, 500, &counter);
    TEST_ASSERT_EQUAL_INT(3, counter);
    TEST_ASSERT_EQUAL_INT(pdTRUE, result);
    TEST_ASSERT_EQUAL_INT(1, uxSemaphoreGetCount(semaphore));

    result = unorphan_lock(semaphore, 500, &counter);
    TEST_ASSERT_EQUAL_INT(4, counter);
    TEST_ASSERT_EQUAL_INT(pdTRUE, result);
    TEST_ASSERT_EQUAL_INT(1, uxSemaphoreGetCount(semaphore));
}

void runner_thread(__unused void *args)
{
    for (;;) {
        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_semaphore_free);
        RUN_TEST(test_semaphore_taken);
        RUN_TEST(test_deadlock);
        RUN_TEST(test_orphan_lock);
        RUN_TEST(test_unorphan_lock);

        UNITY_END();
        sleep_ms(10000);
    }
}

int main (void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    xTaskCreate(runner_thread, "TestRunner",
                configMINIMAL_STACK_SIZE, NULL, TEST_RUNNER_PRIORITY, NULL);
    vTaskStartScheduler();
	return 0;
}