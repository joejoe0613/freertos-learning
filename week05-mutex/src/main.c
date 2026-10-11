#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


static SemaphoreHandle_t xResourceMutex;


static void vLowTask(void *pvParameters)
{
    volatile unsigned long i;

    (void)pvParameters;

    printf("[LOW] Taking mutex\n");
    fflush(stdout);

    xSemaphoreTake(
        xResourceMutex,
        portMAX_DELAY
    );

    printf("[LOW] Mutex acquired\n");
    fflush(stdout);

    for (i = 0; i < 200000000UL; i++)
    {
        if ((i % 50000000UL) == 0)
        {
            printf("[LOW] Working...\n");
            fflush(stdout);
        }
    }

    printf("[LOW] Releasing mutex\n");
    fflush(stdout);

    xSemaphoreGive(xResourceMutex);

    vTaskSuspend(NULL);
}


static void vMediumTask(void *pvParameters)
{
    volatile unsigned long i;

    (void)pvParameters;

    vTaskDelay(pdMS_TO_TICKS(150)); // Delay to ensure Low task has acquired the mutex before Medium task starts its work

    printf("[MEDIUM] Ready and doing CPU work\n");
    fflush(stdout);

    for (i = 0; i < 100000000UL; i++)
    {
        /* CPU work */
    }

    printf("[MEDIUM] Finished\n");
    fflush(stdout);

    vTaskSuspend(NULL);
}


static void vHighTask(void *pvParameters)
{
    (void)pvParameters;

    vTaskDelay(pdMS_TO_TICKS(100)); // Delay to ensure Low task has acquired the mutex before High task attempts to take it

    printf("[HIGH] Trying to take mutex\n");
    fflush(stdout);

    // This will block until the Low task releases the mutex, demonstrating priority inheritance
    xSemaphoreTake(
        xResourceMutex,
        portMAX_DELAY
    );

    printf("[HIGH] Mutex acquired\n");
    fflush(stdout);

    // Simulate some work while holding the mutex
    xSemaphoreGive(xResourceMutex);

    printf("[HIGH] Finished\n");
    fflush(stdout);

    vTaskSuspend(NULL);
}


void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                   char *pcTaskName)
{
    (void)xTask;

    fprintf(stderr,
            "Stack overflow detected in task: %s\n",
            pcTaskName);

    abort();
}


int main(void)
{
    printf("Week 5 - Priority Inheritance Experiment\n");

    xResourceMutex = xSemaphoreCreateMutex();

    if (xResourceMutex == NULL)
    {
        printf("Mutex creation failed!\n");
        return 1;
    }

    xTaskCreate(
        vLowTask,
        "Low",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        vMediumTask,
        "Medium",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        2,
        NULL
    );

    xTaskCreate(
        vHighTask,
        "High",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        3,
        NULL
    );

    vTaskStartScheduler();

    return 0;
}