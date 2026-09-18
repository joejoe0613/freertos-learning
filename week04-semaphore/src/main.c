#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

static SemaphoreHandle_t xEventSemaphore = NULL;

static void vEventGeneratorTask(void *pvParameters)
{
    int generated = 0;
    (void)pvParameters;

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(200));

        generated++;

        printf("[Generator] Event %d generated at tick %lu\n", generated, (unsigned long)xTaskGetTickCount());
        printf("[Generator] Give event, pending count = %u\n", (unsigned int)uxSemaphoreGetCount(xEventSemaphore));
        

        if(xSemaphoreGive(xEventSemaphore) != pdTRUE){
            printf("[Generator] Semaphore FULL - event not recorded\n");
        }
        else{
            printf("[Generator] Give success\n");
        }
        fflush(stdout);

        xSemaphoreGive(xEventSemaphore);
    }
}

static void vProcessingTask(void *pvParameters)
{
    int processed = 0;
    
    (void)pvParameters;

    for (;;)
    {
        if (xSemaphoreTake(
                xEventSemaphore,
                portMAX_DELAY) == pdTRUE)
        {
            processed++;

            printf("    [Processor] Processing event #%d at tick %lu\n", processed, (unsigned long)xTaskGetTickCount());
            printf("    [Processor] Take event, remaining count = %u\n", (unsigned int)uxSemaphoreGetCount(xEventSemaphore));

            fflush(stdout);

            /*
             * Simulate slow processing.
             */
            vTaskDelay(pdMS_TO_TICKS(1000));
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

    xEventSemaphore = xSemaphoreCreateCounting(5, 0);

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