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

// static void vLowPriorityTask(void* pvParameters)
// {
//     volatile unsigned long counter = 0;

//     (void)pvParameters;

//     for(;;){
//         counter++;

//         if(counter >= 10000000UL){
//             printf("[LOW] running at tick = %lu\n", (unsigned long)xTaskGetTickCount());

//             fflush(stdout);

//             counter = 0;
//         }
//     }
// }

// static void vHighPriorityTask(void* pvParameters)
// {
//     TickType_t xLastWakeTime;

//     (void)pvParameters;

//     xLastWakeTime = xTaskGetTickCount();

//     for(;;)
//     {
//         printf("[HIGH] running at tick = %lu\n", (unsigned long)xTaskGetTickCount());

//         fflush(stdout);

//         vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
//     }
// }

// void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
// {
//     (void)xTask;

//     fprintf(stderr, "StackOverflow detected in task: %s\n", pcTaskName);

//     abort();
// }

// int main(void){
//     BaseType_t resultLow;
//     BaseType_t resultHigh;

//     printf("Week 2 - Preemption Experement\n");

//     resultLow = xTaskCreate(
//         vLowPriorityTask,
//         "LowPriorityTask",
//         configMINIMAL_STACK_SIZE * 2,
//         NULL,
//         1,
//         NULL
//     );

//     resultHigh = xTaskCreate(
//         vHighPriorityTask,
//         "HighPriorityTask",
//         configMINIMAL_STACK_SIZE * 2,
//         NULL,
//         2,
//         NULL
//     );

//     if ((resultLow != pdPASS) ||
//         (resultHigh != pdPASS))
//     {
//         printf("Task creation failed!\n");
//         return 1;
//     }

//     vTaskStartScheduler();

//     return 0;
// }

// static TaskHandle_t xWorkerHandle = NULL;

// static void vWorkerTask(void* pvParameters)
// {
//     volatile unsigned long counter = 0;

//     (void)pvParameters;

//     for(;;){
//         counter++;

//         if(counter >= 10000000UL){
//             printf("[WORKER] running at tick = %lu\n", (unsigned long)xTaskGetTickCount());

//             fflush(stdout);

//             vTaskDelay(pdMS_TO_TICKS(500));
//         }
//     }
// }

// static void vControllerTask(void* pvParameters)
// {
//     (void)pvParameters;

//     for(;;)
//     {
//         vTaskDelay(pdMS_TO_TICKS(2000));

//         printf("\n>>> Suspending Worker Task at tick = %lu\n\n", (unsigned long)xTaskGetTickCount());

//         fflush(stdout);

//         vTaskSuspend(xWorkerHandle);

//         vTaskDelay(pdMS_TO_TICKS(3000));

//         printf("\n>>> Resuming Worker Task at tick = %lu\n\n", (unsigned long)xTaskGetTickCount());

//         fflush(stdout);

//         vTaskResume(xWorkerHandle);
//     }
// }

// void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
// {
//     (void)xTask;

//     fprintf(stderr, "StackOverflow detected in task: %s\n", pcTaskName);

//     abort();
// }

// int main(void){
//     BaseType_t resultWorker;
//     BaseType_t resultController;

//     printf("Week 2 - Suspend / Resume Experiment\n");

//     resultWorker = xTaskCreate(
//         vWorkerTask,
//         "WorkerTask",
//         configMINIMAL_STACK_SIZE * 2,
//         NULL,
//         1,
//         &xWorkerHandle
//     );

//     resultController = xTaskCreate(
//         vControllerTask,
//         "ControllerTask",
//         configMINIMAL_STACK_SIZE * 2,
//         NULL,
//         2,
//         NULL
//     );

//     if ((resultWorker != pdPASS) ||
//         (resultController != pdPASS))
//     {
//         printf("Task creation failed!\n");
//         return 1;
//     }

//     vTaskStartScheduler();

//     return 0;
// }