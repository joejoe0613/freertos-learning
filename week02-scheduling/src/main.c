#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"

static void vTaskA(void *pvParameters)
{
    unsigned long counter = 0;

    (void)pvParameters;

    for(;;){
        for (counter = 0;
             counter < 1000000UL;
             counter++)
        {
            /* Simulate CPU work. */
        }

        printf("[Task A] yield at tick = %lu\n",
               (unsigned long)xTaskGetTickCount());

        fflush(stdout);

        taskYIELD();
    }
}

static void vTaskB(void *pvParameters)
{
    unsigned long counter = 0;

    (void)pvParameters;

    for(;;){
        for (counter = 0;
             counter < 1000000UL;
             counter++)
        {
            /* Simulate CPU work. */
        }

        printf("[Task B] yield at tick = %lu\n",
               (unsigned long)xTaskGetTickCount());

        fflush(stdout);

        taskYIELD();
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;

    fprintf(stderr, "StackOverflow detected in task: %s\n", pcTaskName);

    abort();
}

int main(void)
{
    BaseType_t resultA;
    BaseType_t resultB;

    printf("Week 2 - Time Slicing Experement\n");

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

    if ((resultA != pdPASS) ||
        (resultB != pdPASS))
    {
        printf("Task creation failed!\n");
        return 1;
    }

    vTaskStartScheduler();

    return 0;
}
