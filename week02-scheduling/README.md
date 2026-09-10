# Week 2 - FreeRTOS Scheduling and Task Control

## Objective

The goal of Week 2 is to understand how the FreeRTOS scheduler handles:

- Equal-priority tasks
- Time slicing
- Voluntary yielding
- Priority-based preemption
- Task suspension
- Task resumption

Week 2 extends the task scheduling concepts introduced in Week 1.

---

## Environment

- Linux Mint
- GCC
- FreeRTOS-Kernel
- FreeRTOS POSIX Port

The FreeRTOS POSIX port is used to execute the kernel directly on Linux.

---

## FreeRTOS Configuration

The main scheduling-related configuration used in Week 2 is:

```c
#define configUSE_PREEMPTION       1
#define configUSE_TIME_SLICING     1
#define configTICK_RATE_HZ         1000
#define configMAX_PRIORITIES       5
#define INCLUDE_vTaskSuspend       1
```

Some experiments temporarily modified these settings to observe different scheduler behaviors.

---

# Experiment 2A - Equal-Priority Scheduling

Two tasks were created with the same priority.

```text
Task A
Priority = 1

Task B
Priority = 1
```

Both tasks remained continuously Ready and did not block.

The goal was to observe how FreeRTOS schedules multiple Ready tasks with the same priority.

---

## Experiment 2A-1 - Time Slicing Enabled

Configuration:

```c
#define configUSE_TIME_SLICING 1
```

Both Task A and Task B remained Ready.

The FreeRTOS tick allows equal-priority tasks to share CPU time.

Example behavior:

```text
Task A
Task B
Task A
Task B
...
```

The exact output order is not necessarily strictly alternating when using the Linux POSIX simulator.

### Observation

Both equal-priority tasks were able to execute even though neither task entered the Blocked state.

### Key Concept

Time slicing allows multiple Ready tasks at the same priority level to share CPU execution time.

---

## Experiment 2A-2 - Time Slicing Disabled

Configuration:

```c
#define configUSE_TIME_SLICING 0
```

Both tasks remained at the same priority and continued running without blocking or yielding.

One task continued executing while the other task remained Ready.

Example behavior:

```text
Task B
Task B
Task B
Task B
...
```

### Observation

When time slicing was disabled, the scheduler no longer automatically switched between equal-priority Ready tasks because of the system tick.

### Key Concept

Disabling time slicing removes tick-based round-robin scheduling between equal-priority tasks.

---

## Experiment 2A-3 - Explicit taskYIELD()

Time slicing remained disabled:

```c
#define configUSE_TIME_SLICING 0
```

Each task explicitly called:

```c
taskYIELD();
```

Example:

```c
for (;;)
{
    /* Perform CPU work */

    printf("Task running\n");

    taskYIELD();
}
```

### Observation

Both Task A and Task B were able to execute again.

### Key Concept

`taskYIELD()` voluntarily returns control to the scheduler.

The task does not enter the Blocked state.

Instead:

```text
Running
   |
   | taskYIELD()
   v
Ready
```

The task remains eligible to run immediately again.

---

# Time Slicing vs taskYIELD()

Time slicing:

```text
Scheduler-controlled switching
```

`taskYIELD()`:

```text
Task-controlled voluntary switching
```

Both can allow equal-priority tasks to share the CPU, but they are different mechanisms.

---

# Experiment 2B - Priority Preemption

Two tasks with different priorities were created.

```text
High Priority Task
Priority = 2

Low Priority Task
Priority = 1
```

The Low Priority Task continuously performed CPU work.

The High Priority Task periodically entered the Blocked state and became Ready every 1000 ms.

---

## Experiment 2B-1 - Preemption Enabled

Configuration:

```c
#define configUSE_PREEMPTION 1
```

The High Priority Task periodically called:

```c
vTaskDelayUntil()
```

While the High Priority Task was Blocked, the Low Priority Task executed.

When the High Priority Task became Ready again, it immediately preempted the Low Priority Task.

Example behavior:

```text
HIGH runs
   |
   v
HIGH blocks

LOW runs continuously

   |
   | HIGH becomes Ready
   v

LOW is preempted

HIGH runs

   |
   v
HIGH blocks again

LOW resumes
```

### Key Concept

With preemption enabled, a newly Ready task can immediately preempt the currently Running task if its priority is higher.

---

## Experiment 2B-2 - Preemption Disabled

Configuration:

```c
#define configUSE_PREEMPTION 0
```

The Low Priority Task continuously executed without blocking or yielding.

When the High Priority Task became Ready, it could not automatically preempt the Low Priority Task.

### Observation

The High Priority Task remained Ready but could not obtain CPU time until the Running task voluntarily returned control to the scheduler.

### Key Concept

Without preemption, task switching depends on the currently Running task voluntarily blocking or yielding.

---

# Preemption vs Time Slicing

These two concepts are different.

## Time Slicing

Used mainly when:

```text
Task A Priority 1
Task B Priority 1
```

It controls CPU sharing between tasks at the same priority.

## Preemption

Used when:

```text
High Priority Task = 2
Low Priority Task  = 1
```

It controls whether a higher-priority Ready task can immediately replace a lower-priority Running task.

---

# Experiment 2C - Task Suspend and Resume

Two tasks were created:

```text
Worker Task
Priority = 1

Controller Task
Priority = 2
```

The Worker Task periodically executed every 500 ms.

The Controller Task:

```text
Allows Worker to run
        |
        v
Waits 2 seconds
        |
        v
Suspends Worker
        |
        v
Waits 3 seconds
        |
        v
Resumes Worker
```

---

## Task Handle

A task handle was stored when the Worker Task was created.

```c
static TaskHandle_t xWorkerHandle = NULL;
```

Task creation:

```c
xTaskCreate(
    vWorkerTask,
    "Worker",
    configMINIMAL_STACK_SIZE * 2,
    NULL,
    1,
    &xWorkerHandle
);
```

The task handle identifies the Worker Task so that another task can control it.

---

## Suspending a Task

The Controller Task uses:

```c
vTaskSuspend(xWorkerHandle);
```

The Worker Task enters the Suspended state.

While Suspended, the Worker does not execute.

---

## Resuming a Task

The Controller later calls:

```c
vTaskResume(xWorkerHandle);
```

The Worker returns to the Ready state and can be selected by the scheduler again.

---

# Blocked vs Suspended

## Blocked

A task enters Blocked when waiting for something.

Examples:

```text
Delay
Queue
Semaphore
Notification
Event
```

The task automatically returns to Ready when the waiting condition is satisfied.

Example:

```text
Running
   |
   | vTaskDelay()
   v
Blocked
   |
   | timeout expires
   v
Ready
```

---

## Suspended

A Suspended task does not automatically return to Ready.

Example:

```text
Running / Ready / Blocked
          |
          | vTaskSuspend()
          v
      Suspended
          |
          | vTaskResume()
          v
        Ready
```

The task must explicitly be resumed.

---

# Important FreeRTOS APIs

## taskYIELD()

Voluntarily gives control back to the scheduler.

```c
taskYIELD();
```

State behavior:

```text
Running -> Ready
```

---

## vTaskSuspend()

Suspends a specified task.

```c
vTaskSuspend(xTaskHandle);
```

---

## vTaskResume()

Resumes a previously suspended task.

```c
vTaskResume(xTaskHandle);
```

---

## TaskHandle_t

Used to identify and control a specific FreeRTOS task.

```c
TaskHandle_t xTaskHandle;
```

---

# Scheduler Concepts

## Priority

The scheduler selects the highest-priority task from the Ready state.

```text
Ready Tasks:

Task A Priority 2
Task B Priority 1

Scheduler selects Task A.
```

---

## Time Slicing

Allows Ready tasks at the same priority to share CPU time.

---

## Preemption

Allows a higher-priority Ready task to interrupt a lower-priority Running task.

---

## Yield

Allows the currently Running task to voluntarily request another scheduling decision.

---

## Blocking

Temporarily removes a task from CPU scheduling until a condition is satisfied.

---

## Suspension

Explicitly removes a task from normal scheduling until another task resumes it.

---

# Week 2 Key Takeaways

1. FreeRTOS scheduling depends on both task priority and task state.

2. Time slicing is used to share CPU time between equal-priority Ready tasks.

3. `taskYIELD()` voluntarily returns control to the scheduler.

4. `taskYIELD()` does not place the task in the Blocked state.

5. A task that calls `taskYIELD()` remains Ready.

6. Preemption allows a higher-priority Ready task to interrupt a lower-priority Running task.

7. When preemption is disabled, a Running task must voluntarily block or yield before another task can execute.

8. Time slicing and preemption are different scheduling mechanisms.

9. `vTaskSuspend()` places a task in the Suspended state.

10. A Suspended task must be explicitly resumed using `vTaskResume()`.

11. Blocked tasks automatically return to Ready when their waiting condition is satisfied.

12. Task handles allow one task to identify and control another task.

---

# Week 2 Result

Week 2 successfully demonstrated:

- Equal-priority task scheduling
- Time slicing
- Scheduling without time slicing
- Voluntary context switching using `taskYIELD()`
- Priority-based preemption
- Non-preemptive scheduling
- Task handles
- Task suspension
- Task resumption
- Differences between Ready, Blocked, and Suspended states

The Week 2 experiments provide a deeper understanding of how the FreeRTOS scheduler selects and controls tasks.
