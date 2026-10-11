#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

static volatile unsigned long sharedCounter = 0;

static SemaphoreHandle_t xCounterMutex = NULL;

static void vTaskA(void *pvParameters)
{
    unsigned long temp;
    int i;

    (void) pvParameters;

    for (i = 0; i < 1000; ++i)
    {
        xSemaphoreTake(xCounterMutex, portMAX_DELAY); /* Take the mutex before accessing sharedCounter */   
        
        temp = sharedCounter;

        taskYIELD(); /* Yield to allow other tasks to run */

        temp++;
        
        taskYIELD();

        sharedCounter = temp;

        xSemaphoreGive(xCounterMutex); /* Release the mutex after accessing sharedCounter */
    }

    printf("[Task A] Finished\n");
    fflush(stdout);

    vTaskSuspend(NULL); /* Suspend itself */
}

static void vTaskB(void *pvParameters)
{
    unsigned long temp;
    int i;

    (void) pvParameters;

    for (i = 0; i < 1000; ++i)
    {
        xSemaphoreTake(xCounterMutex, portMAX_DELAY); /* Take the mutex before accessing sharedCounter */

        temp = sharedCounter;

        taskYIELD(); /* Yield to allow other tasks to run */

        temp++;
        
        taskYIELD();

        sharedCounter = temp;

        xSemaphoreGive(xCounterMutex); /* Release the mutex after accessing sharedCounter */    
    }

    printf("[Task B] Finished\n");
    fflush(stdout);

    vTaskSuspend(NULL); /* Suspend itself */
}

static void vMonitorTask(void *pvParameters)
{
    (void) pvParameters;

    vTaskDelay(pdMS_TO_TICKS(2000)); /* Initial delay to allow other tasks to start */

    printf("\nExpected Counter = 2000\n");
    printf("Actual Counter = %lu\n", sharedCounter);
    fflush(stdout);

    vTaskSuspend(NULL); /* Suspend itself */
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void) xTask;
    (void) pcTaskName;

    fprintf(stderr, "Stack overflow detected in task: %s\n", pcTaskName); // Optionally, you can halt the system or reset it here
    
    abort(); // Terminate the program to prevent further damage
}

int main(void)
{
    xCounterMutex = xSemaphoreCreateMutex();

    if(xCounterMutex == NULL)
    {
        printf("Mutex creation failed\n");
        return 1;
    }

    printf("Week 5 - Race Condition Experiment\n");

    xTaskCreate(
        vTaskA,
        "TaskA",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        vTaskB,
        "TaskB",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        vMonitorTask,
        "Monitor",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        2,
        NULL
    );

    vTaskStartScheduler();

    return 0;
}