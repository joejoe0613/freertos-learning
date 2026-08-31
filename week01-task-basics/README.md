# Week 1 - FreeRTOS Task Basics

## Objective

The goal of Week 1 is to understand the fundamental task scheduling model of FreeRTOS.

Topics covered:

- Task creation
- FreeRTOS scheduler
- Task states
- Task priority
- System tick
- `vTaskDelay()`
- `vTaskDelayUntil()`
- Task starvation

---

## Environment

- Linux Mint
- GCC
- FreeRTOS-Kernel
- FreeRTOS POSIX Port

The FreeRTOS POSIX port is used to run the FreeRTOS kernel directly on Linux.

Project structure:

```text
freertos-learning/
└── week01-task-basics/
    ├── README.md
    ├── Makefile
    ├── src/
    │   └── main.c
    ├── include/
    │   └── FreeRTOSConfig.h
    └── output/
        ├── basic-vTaskDelay.txt
        ├── periodic-vTaskDelayUntil.txt
        ├── different-priority.txt
        └── starvation.txt
```

---

## FreeRTOS Configuration

The following FreeRTOS settings were used in Week 1:

```c
#define configTICK_RATE_HZ                  1000
#define configUSE_PREEMPTION               1
#define configMAX_PRIORITIES               5
#define configMINIMAL_STACK_SIZE           128
#define configUSE_TIMERS                   0
#define configSUPPORT_DYNAMIC_ALLOCATION   1
#define configTOTAL_HEAP_SIZE              (16 * 1024)
```

Since `configTICK_RATE_HZ` is set to 1000:

```text
1000 ticks = approximately 1 second
1 tick     = approximately 1 ms
```

---

# Experiment 1A - Basic Periodic Tasks

Two FreeRTOS tasks were created.

| Task | Delay | Priority |
|------|------:|---------:|
| Task A | 500 ms | 1 |
| Task B | 1000 ms | 1 |

Each task prints the current FreeRTOS tick count and then calls:

```c
vTaskDelay()
```

Example:

```c
for (;;)
{
    printf("[Task A] tick = %lu\n",
           (unsigned long)xTaskGetTickCount());

    vTaskDelay(pdMS_TO_TICKS(500));
}
```

### Observation

Task A executes approximately every 500 ms.

Task B executes approximately every 1000 ms.

When a task calls `vTaskDelay()`, the task changes from the Running state to the Blocked state.

While the task is Blocked, it does not consume CPU time.

After the delay expires, the task returns to the Ready state and waits for the scheduler to execute it.

### Task State Transition

```text
Running
   |
   | vTaskDelay()
   v
Blocked
   |
   | Delay expires
   v
Ready
   |
   | Scheduler selects task
   v
Running
```

---

# Experiment 1B - Fixed Periodic Scheduling

The two tasks were modified to use:

```c
vTaskDelayUntil()
```

Example:

```c
TickType_t xLastWakeTime;

xLastWakeTime = xTaskGetTickCount();

for (;;)
{
    printf("[Task A] tick = %lu\n",
           (unsigned long)xTaskGetTickCount());

    vTaskDelayUntil(
        &xLastWakeTime,
        pdMS_TO_TICKS(500)
    );
}
```

### Difference Between vTaskDelay and vTaskDelayUntil

`vTaskDelay()` delays the task relative to the current execution time.

For example:

```text
20 ms task execution
+
500 ms delay
=
approximately 520 ms period
```

Therefore, execution time may accumulate and cause timing drift.

`vTaskDelayUntil()` uses a fixed reference wake-up time.

For example:

```text
0 ms
500 ms
1000 ms
1500 ms
2000 ms
```

This makes `vTaskDelayUntil()` more suitable for periodic real-time tasks.

### Key Observation

For periodic tasks:

```text
vTaskDelayUntil()
```

provides more stable periodic scheduling than:

```text
vTaskDelay()
```

---

# Experiment 2 - Task Priority

The task priorities were changed to:

| Task | Period | Priority |
|------|------:|---------:|
| Task A | 1000 ms | 2 |
| Task B | 1000 ms | 1 |

Both tasks periodically become Ready at approximately the same time.

The FreeRTOS scheduler selects Task A first because Task A has the higher priority.

Example output:

```text
[Task A] tick = 1000
[Task B] tick = 1001

[Task A] tick = 2000
[Task B] tick = 2001

[Task A] tick = 3000
[Task B] tick = 3001
```

### Scheduler Rule

FreeRTOS does not simply execute the task with the highest priority at all times.

Instead, the scheduler:

```text
Find all Ready tasks
        |
        v
Find the highest-priority Ready task
        |
        v
Execute that task
```

For example:

```text
Task A
Priority = 2
Blocked

Task B
Priority = 1
Ready
```

Task B can execute because Task A is not currently Ready.

Therefore:

> Task priority only determines scheduling order between tasks that are currently in the Ready state.

---

# Experiment 3 - Task Starvation

To demonstrate task starvation, the high-priority Task A was modified so that it never entered the Blocked state.

Task A:

```c
static void vTaskA(void *pvParameters)
{
    volatile unsigned long counter = 0;

    (void)pvParameters;

    printf("[Task A] High-priority task started.\n");

    for (;;)
    {
        counter++;
    }
}
```

Priority configuration:

```text
Task A Priority = 2
Task B Priority = 1
```

Task A remains continuously Running or Ready.

Task B may also be Ready, but the scheduler always finds Task A at the higher priority.

### Scheduling Behavior

```text
Ready Tasks:

Task A   Priority 2
Task B   Priority 1

        |
        v

Scheduler selects Task A

        |
        v

Task A continues running

        |
        v

Task A remains Ready

        |
        v

Scheduler selects Task A again
```

Task B cannot obtain CPU time.

This condition is called:

```text
Task Starvation
```

### Key Lesson

A high-priority task should normally enter the Blocked state when it has no useful work to perform.

Busy waiting should generally be avoided in an RTOS application.

---

# FreeRTOS Task States

The main task states introduced in Week 1 are:

## Running

The task is currently executing on the CPU.

In a single-core system, only one task can be Running at a time.

## Ready

The task is able to execute but is currently waiting for CPU time.

## Blocked

The task is waiting for an event or a timeout.

Examples include:

```text
Delay
Queue
Semaphore
Task Notification
Event
```

A Blocked task does not compete for CPU execution.

## Suspended

The task has been explicitly suspended.

Suspended tasks are not selected by the scheduler until they are resumed.

Suspended tasks were not experimentally investigated in Week 1.

---

# Task State Model

```text
              Ready
                |
                | Scheduler selects task
                v
             Running
                |
                | Delay / Wait
                v
             Blocked
                |
                | Event or timeout completes
                v
              Ready
```

---

# Important FreeRTOS APIs

## xTaskCreate()

Creates a new FreeRTOS task.

```c
xTaskCreate(
    TaskFunction,
    "TaskName",
    StackSize,
    Parameters,
    Priority,
    TaskHandle
);
```

---

## vTaskStartScheduler()

Starts the FreeRTOS scheduler.

```c
vTaskStartScheduler();
```

After the scheduler starts, FreeRTOS determines which Ready task should execute.

---

## xTaskGetTickCount()

Returns the current FreeRTOS tick count.

```c
TickType_t tick;

tick = xTaskGetTickCount();
```

---

## pdMS_TO_TICKS()

Converts milliseconds into FreeRTOS ticks.

```c
pdMS_TO_TICKS(500)
```

---

## vTaskDelay()

Blocks a task for a specified amount of time relative to the current time.

```c
vTaskDelay(pdMS_TO_TICKS(500));
```

---

## vTaskDelayUntil()

Blocks a task until the next periodic wake-up time.

```c
vTaskDelayUntil(
    &xLastWakeTime,
    pdMS_TO_TICKS(500)
);
```

This API is useful for periodic real-time tasks.

---

# Stack Overflow Hook

Stack overflow checking was enabled in the FreeRTOS configuration.

The application therefore implements:

```c
void vApplicationStackOverflowHook(
    TaskHandle_t xTask,
    char *pcTaskName
)
{
    (void)xTask;

    fprintf(
        stderr,
        "Stack overflow detected in task: %s\n",
        pcTaskName
    );

    abort();
}
```

If FreeRTOS detects a task stack overflow, this hook is called.

This allows the application to determine how the error should be handled.

---

# Week 1 Key Takeaways

1. FreeRTOS applications are composed of multiple tasks managed by the scheduler.

2. The scheduler selects the highest-priority task among all tasks currently in the Ready state.

3. A Running task can enter the Blocked state when it waits for a delay or an event.

4. Blocked tasks do not consume CPU execution time.

5. `vTaskDelay()` delays relative to the current execution time.

6. `vTaskDelayUntil()` is more suitable for periodic tasks because it uses a fixed periodic reference.

7. Higher-priority tasks can prevent lower-priority tasks from executing if they never enter the Blocked state.

8. This behavior can result in task starvation.

9. Busy waiting should generally be avoided in RTOS applications.

10. Tasks should normally block while waiting for work instead of continuously polling.

---

# Week 1 Result

Week 1 successfully demonstrated:

- FreeRTOS environment setup on Linux
- FreeRTOS POSIX port
- Task creation
- Scheduler startup
- Periodic tasks
- FreeRTOS system ticks
- `vTaskDelay()`
- `vTaskDelayUntil()`
- Task priorities
- Preemptive scheduling
- Task state transitions
- Task starvation

The Week 1 experiments provide the foundation for studying more advanced FreeRTOS scheduling and task management concepts.
