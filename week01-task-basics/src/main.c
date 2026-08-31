#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"


static void vTaskA(void *pvParameters)
{
    TickType_t xLastWakeTime;

    (void) pvParameters;

    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        printf("[Task A] tick = %lu\n",
               (unsigned long) xTaskGetTickCount());

        fflush(stdout);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}


static void vTaskB(void *pvParameters)
{
    TickType_t xLastWakeTime;

    (void) pvParameters;

    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        printf("[Task B] tick = %lu\n",
               (unsigned long) xTaskGetTickCount());

        fflush(stdout);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
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
    BaseType_t resultA;
    BaseType_t resultB;

    printf("FreeRTOS Week 1 - Task Basics\n");

    resultA = xTaskCreate(
        vTaskA,
        "TaskA",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        1,
        NULL
    );

    resultB = xTaskCreate(
        vTaskB,
        "TaskB",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        1,
        NULL
    );

    if ((resultA != pdPASS) || (resultB != pdPASS))
    {
        printf("Task creation failed!\n");
        return 1;
    }

    printf("Tasks created successfully.\n");
    printf("Starting scheduler...\n");

    vTaskStartScheduler();

    printf("Scheduler failed to start.\n");

    return 0;
}