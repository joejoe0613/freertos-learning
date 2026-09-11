#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

static QueueHandle_t xDataQueue = NULL;

static void vProducerTask(void *pvParameters)
{
    int value = 1;

    (void)pvParameters;

    for(;;)
    {
        printf("[Producer] Sending %d at tick %lu\n", value, xTaskGetTickCount());

        fflush(stdout);

        if(xQueueSend(xDataQueue, &value, portMAX_DELAY) == pdPASS)
        {
            printf("[Producer] send success\n");
            fflush(stdout);
        }

        value++;

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void vConsumerTask(void *pvParameters)
{
    int receivedValue = 0;

    (void)pvParameters;

    for(;;)
    {
        if(xQueueReceive(xDataQueue, &receivedValue, portMAX_DELAY) == pdPASS)
        {
            printf("[Consumer] Received %d at tick %lu\n", receivedValue, xTaskGetTickCount());
            fflush(stdout);
        }
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;

    printf("Stack overflow detected in task %s\n", pcTaskName);
    abort();
}

int main(void)
{
    BaseType_t producerResult, consumerResult;

    printf("Week 3 - Basic Queue Experiment\n");
    
    xDataQueue = xQueueCreate(5, sizeof(int));

    if (xDataQueue == NULL)
    {
        printf("Queue creation failed!\n");
        return 1;
    }

    producerResult = xTaskCreate(
        vProducerTask,
        "Producer",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        1,
        NULL
    );

    consumerResult = xTaskCreate(
        vConsumerTask,
        "Consumer",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        1,
        NULL
    );

    if((producerResult != pdPASS) || (consumerResult != pdPASS))
    {
        printf("Task creation failed!\n");
        return 1;
    }

    printf("Queue and tasks created successfully.\n");
    printf("Starting scheduler...\n");

    vTaskStartScheduler();

    return 0;
}