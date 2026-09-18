#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

static SemaphoreHandle_t xEventSemaphore = NULL;

static void vEventGeneratorTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        printf("[Generator] Event generated at tick %lu\n", (unsigned long)xTaskGetTickCount());
        
        fflush(stdout);

        xSemaphoreGive(xEventSemaphore);
    }
}

static void vProcessingTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        printf("    [Processor] Waiting for event...\n");
        fflush(stdout);

        if (xSemaphoreTake(xEventSemaphore, portMAX_DELAY) == pdTRUE)
        {
            printf("    [Processor] Event received at tick %lu\n", (unsigned long)xTaskGetTickCount());
            fflush(stdout);
        }
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;

    fprintf(stderr, "Stack overflow detected in task: %s\n", pcTaskName);

    abort();
}

int main(void)
{
   BaseType_t generatorResult;
    BaseType_t processorResult;

    printf("Week 4 - Binary Semaphore Experiment\n");

    xEventSemaphore = xSemaphoreCreateBinary();

    if (xEventSemaphore == NULL)
    {
        printf("Semaphore creation failed!\n");
        return 1;
    }

    generatorResult = xTaskCreate(
        vEventGeneratorTask,
        "Generator",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        1,
        NULL
    );

    processorResult = xTaskCreate(
        vProcessingTask,
        "Processor",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        2,
        NULL
    );

    if ((generatorResult != pdPASS) ||
        (processorResult != pdPASS))
    {
        printf("Task creation failed!\n");
        return 1;
    }

    printf("Semaphore and tasks created successfully.\n");
    printf("Starting scheduler...\n");

    vTaskStartScheduler();

    return 0;
}