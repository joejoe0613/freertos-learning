#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef struct 
{
    /* data */
    int sensorID;
    int value;
    TickType_t timestamp;
} SensorMessage_t;


static QueueHandle_t xDataQueue = NULL;

static void vProducerTask(void *pvParameters)
{
    SensorMessage_t message;

    int sequence = 0;

    (void)pvParameters;

    for (;;)
    {
        /*
         * Simulate three different sensors.
         */
        message.sensorID = (sequence % 3) + 1;

        /*
         * Give each sensor a different value range
         * so the message structure is easier to observe.
         */
        switch (message.sensorID)
        {
            case 1:
                message.value = 20 + sequence;
                break;

            case 2:
                message.value = 50 + sequence;
                break;

            case 3:
                message.value = 100 + sequence;
                break;

            default:
                message.value = 0;
                break;
        }

        message.timestamp = xTaskGetTickCount();

        if (xQueueSend(
                xDataQueue,
                &message,
                portMAX_DELAY) == pdPASS)
        {
            printf(
                "[Producer] ID=%d Value=%d Timestamp=%lu\n",
                message.sensorID,
                message.value,
                (unsigned long)message.timestamp
            );

            fflush(stdout);
        }

        sequence++;

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void vConsumerTask(void *pvParameters)
{
    SensorMessage_t received;

    (void)pvParameters;

    for(;;)
    {
        if (xQueueReceive(xDataQueue, &received, portMAX_DELAY) == pdPASS){
            printf("[Consumer] ID=%d Value=%d Timestamp=%lu\n", received.sensorID, received.value, (unsigned long)received.timestamp);
        }

        fflush(stdout);
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
    
    xDataQueue = xQueueCreate(5, sizeof(SensorMessage_t));

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