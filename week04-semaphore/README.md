# Week 4 - FreeRTOS Semaphore and Task Synchronization

## Objective

The goal of Week 4 is to understand how FreeRTOS semaphores are used for task synchronization and event signaling.

Topics covered:

- Binary Semaphore
- Counting Semaphore
- Task synchronization
- Event signaling
- Blocking on semaphore
- Event coalescing
- Semaphore count
- Difference between Queue and Semaphore

---

## Environment

- Linux Mint
- GCC
- FreeRTOS-Kernel
- FreeRTOS POSIX Port

The FreeRTOS POSIX port is used to execute the FreeRTOS kernel directly on Linux.

Project structure:

```text
week04-semaphore/
├── README.md
├── Makefile
├── include/
│   └── FreeRTOSConfig.h
├── src/
│   └── main.c
└── output/
    ├── binary-semaphore.txt
    ├── binary-event-coalescing.txt
    └── counting-semaphore.txt
```

---

# Semaphore Concept

A semaphore is mainly used for synchronization rather than transferring application data.

Conceptually:

```text
Task A
  |
  | Give
  v
[ Semaphore ]
  |
  | Take
  v
Task B
```

Unlike a Queue, a semaphore does not normally contain application data such as:

```text
Sensor ID
Value
Timestamp
```

Instead, it represents:

```text
Event available
Resource available
Synchronization condition
```

---

# Queue vs Semaphore

A Queue is primarily used to transfer data.

Example:

```text
Producer
   |
   | SensorMessage_t
   v
[ Queue ]
   |
   v
Consumer
```

The Consumer receives actual data.

A Semaphore is primarily used to indicate that something happened.

Example:

```text
Generator
   |
   | xSemaphoreGive()
   v
[ Semaphore ]
   |
   | xSemaphoreTake()
   v
Processor
```

The Processor knows that an event occurred, but the semaphore itself does not contain the event data.

---

# Important FreeRTOS Semaphore APIs

## xSemaphoreCreateBinary()

Creates a binary semaphore.

```c
xEventSemaphore = xSemaphoreCreateBinary();
```

A binary semaphore has two logical states:

```text
0 = unavailable
1 = available
```

---

## xSemaphoreTake()

Attempts to obtain a semaphore.

Example:

```c
xSemaphoreTake(
    xEventSemaphore,
    portMAX_DELAY
);
```

If the semaphore is unavailable, the task enters the Blocked state.

---

## xSemaphoreGive()

Releases or signals a semaphore.

Example:

```c
xSemaphoreGive(xEventSemaphore);
```

This can cause a task waiting on the semaphore to transition from:

```text
Blocked
   |
   v
Ready
```

---

## xSemaphoreCreateCounting()

Creates a counting semaphore.

Example:

```c
xSemaphoreCreateCounting(
    5,
    0
);
```

Parameters:

```text
Maximum count = 5
Initial count = 0
```

---

## uxSemaphoreGetCount()

Returns the current semaphore count.

Example:

```c
uxSemaphoreGetCount(xEventSemaphore);
```

This is useful for observing the number of pending semaphore tokens.

---

# Experiment 4A - Binary Semaphore Synchronization

Two tasks were created:

```text
Event Generator Task
Processor Task
```

The Event Generator periodically generates an event.

The Processor waits for the event.

Architecture:

```text
Event Generator
      |
      | xSemaphoreGive()
      v
+------------------+
| Binary Semaphore |
+------------------+
      |
      | xSemaphoreTake()
      v
Processor
```

---

## Event Generator

The Generator periodically calls:

```c
xSemaphoreGive(xEventSemaphore);
```

Example behavior:

```text
Generator
   |
   | Event generated
   v
Semaphore = 1
```

---

## Processor

The Processor calls:

```c
xSemaphoreTake(
    xEventSemaphore,
    portMAX_DELAY
);
```

If the semaphore is not available:

```text
Processor
Running
   |
   | xSemaphoreTake()
   v
Semaphore unavailable
   |
   v
Blocked
```

When the Generator gives the semaphore:

```text
Generator
   |
   | xSemaphoreGive()
   v
Semaphore available
   |
   v
Processor
Blocked -> Ready
```

---

## Expected Behavior

Example output:

```text
[Processor] Waiting for event...

[Generator] Event generated at tick 1000
[Processor] Event received at tick 1000

[Processor] Waiting for event...

[Generator] Event generated at tick 2000
[Processor] Event received at tick 2000
```

---

# Task Synchronization

The Processor does not continuously poll for events.

Instead, it waits using:

```c
xSemaphoreTake(
    xEventSemaphore,
    portMAX_DELAY
);
```

This causes the task to enter the Blocked state when no event is available.

The CPU can then execute other Ready tasks.

This avoids busy waiting.

---

# Priority and Preemption

The Processor can have a higher priority than the Generator.

Example:

```text
Processor Priority = 2
Generator Priority = 1
```

Suppose:

```text
Processor = Blocked
Generator = Running
```

When the Generator gives the semaphore:

```text
Processor
Blocked -> Ready
```

Because the Processor has a higher priority, it may immediately preempt the Generator.

This behavior connects the semaphore experiment with the preemption concepts studied in Week 2.

---

# Experiment 4B - Binary Semaphore Event Coalescing

The Event Generator was modified to generate events faster than the Processor could handle them.

Example configuration:

```text
Generator period = 200 ms
Processor processing time = 1000 ms
```

The Generator therefore produces multiple events while the Processor is busy.

---

## Binary Semaphore Limitation

A binary semaphore only represents:

```text
0
or
1
```

Suppose the Processor takes the semaphore:

```text
Semaphore:

1 -> 0
```

The Processor then performs work for 1000 ms.

During that time, the Generator may generate several events:

```text
Event #2
Event #3
Event #4
Event #5
```

However, the binary semaphore cannot count all of these events.

The first Give changes:

```text
0 -> 1
```

Additional Gives while the semaphore is already available do not increase the value beyond 1.

Conceptually:

```text
Give
Give
Give
Give

  |
  v

Binary Semaphore

[1]
```

---

# Event Coalescing

This behavior is known as event coalescing.

Multiple events can be represented by a single available semaphore token.

Example:

```text
Generated events:

1
2
3
4
5

Semaphore state:

1
```

The Processor only knows:

```text
At least one event occurred
```

It does not know exactly how many events occurred.

---

## Observation

The number of generated events can become much larger than the number of processed events.

Example:

```text
Generator:
Event #1
Event #2
Event #3
Event #4
Event #5

Processor:
Process #1
Process #2
```

This is expected behavior for a binary semaphore.

It is not necessarily a software bug.

---

# When Binary Semaphore Is Appropriate

Binary Semaphore is useful when the application only needs to know:

```text
Something happened
```

Examples:

```text
Data ready
Peripheral event occurred
Start processing
Wake up a task
```

It is not suitable when the exact number of events must be preserved.

---

# Experiment 4C - Counting Semaphore

To preserve multiple pending events, a counting semaphore was used.

Example:

```c
xEventSemaphore = xSemaphoreCreateCounting(
    5,
    0
);
```

This creates:

```text
Maximum Count = 5
Initial Count = 0
```

Unlike a binary semaphore:

```text
Binary Semaphore:
0 or 1
```

a counting semaphore can represent:

```text
0
1
2
3
4
5
```

---

# Counting Pending Events

Suppose events are generated faster than they are processed.

Each Give can increase the count:

```text
Event 1
Count = 1

Event 2
Count = 2

Event 3
Count = 3

Event 4
Count = 4
```

The semaphore therefore remembers how many pending event tokens exist.

---

# Taking a Counting Semaphore

The Processor calls:

```c
xSemaphoreTake(
    xEventSemaphore,
    portMAX_DELAY
);
```

Each successful Take reduces the count by one.

Example:

```text
Count = 4
   |
   | Take
   v
Count = 3
```

Another Take:

```text
Count = 3
   |
   | Take
   v
Count = 2
```

---

# Observing Semaphore Count

The current count can be checked using:

```c
uxSemaphoreGetCount(xEventSemaphore);
```

Example output:

```text
[Generator] pending count = 1
[Generator] pending count = 2
[Generator] pending count = 3
[Generator] pending count = 4
[Generator] pending count = 5
```

The Processor may then reduce the count:

```text
[Processor] remaining count = 4
[Processor] remaining count = 3
```

---

# Maximum Count

A counting semaphore has a fixed maximum value.

For example:

```c
xSemaphoreCreateCounting(
    5,
    0
);
```

means:

```text
Maximum Count = 5
```

If the count is already:

```text
5
```

another call to:

```c
xSemaphoreGive()
```

cannot increase the count to:

```text
6
```

The Give operation fails.

Example:

```c
if (xSemaphoreGive(xEventSemaphore) == pdTRUE)
{
    printf("Give success\n");
}
else
{
    printf("Semaphore FULL - event not recorded\n");
}
```

This demonstrates that a counting semaphore is not an unlimited counter.

---

# Binary Semaphore vs Counting Semaphore

## Binary Semaphore

Possible values:

```text
0
1
```

Useful for:

```text
Event notification
Task synchronization
Wake-up signal
```

---

## Counting Semaphore

Possible values:

```text
0 ... Maximum Count
```

Useful for:

```text
Counting pending events
Tracking available resources
Controlling limited resources
```

---

# Counting Semaphore as Resource Counter

A counting semaphore can also represent the number of available resources.

For example:

```c
xSemaphoreCreateCounting(
    3,
    3
);
```

means:

```text
Maximum resources = 3
Available resources = 3
```

When a task obtains a resource:

```text
3 -> 2
```

Another task:

```text
2 -> 1
```

Another task:

```text
1 -> 0
```

A fourth task attempting to Take the semaphore must wait:

```text
No resource available
        |
        v
Task Blocked
```

When one resource is released:

```text
0 -> 1
```

a waiting task can become Ready.

---

# Queue vs Binary Semaphore vs Counting Semaphore

## Queue

Used when the application needs to transfer actual data.

Example:

```text
SensorMessage_t
ID
Value
Timestamp
```

Conceptually:

```text
Producer
   |
   v
[ Data Queue ]
   |
   v
Consumer
```

---

## Binary Semaphore

Used when the application only needs to know whether an event occurred.

Conceptually:

```text
Event
  |
  v
[ 0 / 1 ]
  |
  v
Task
```

---

## Counting Semaphore

Used when the application needs to track how many events or resources are available.

Conceptually:

```text
Events
  |
  v
[ 0 ... N ]
  |
  v
Task
```

---

# Task State Relationship

Semaphore operations are closely related to FreeRTOS task states.

Example:

```text
Processor
Running
   |
   | xSemaphoreTake()
   v
Semaphore unavailable
   |
   v
Blocked
```

When another task gives the semaphore:

```text
Generator
   |
   | xSemaphoreGive()
   v
Processor
Blocked -> Ready
```

If the Processor has a higher priority than the currently Running task:

```text
Ready
   |
   | Preemption
   v
Running
```

This connects semaphore synchronization with the scheduling concepts studied in Week 1 and Week 2.

---

# Binary Semaphore vs Queue Blocking

The blocking behavior resembles the Queue experiments from Week 3.

Queue Empty:

```text
Consumer
xQueueReceive()
      |
      v
Blocked
```

---

# Week 4 Key Takeaways

1. Semaphores are primarily used for synchronization and event signaling.
Semaphore unavailable:

```text
Task
xSemaphoreTake()
      |
      v
Blocked
```

Both mechanisms allow tasks to wait without busy polling.

The main difference is what is being exchanged:

```text
Queue
-> Data

Semaphore
-> Synchronization / Event token
```

---

# Important Week 4 APIs

Semaphore handle:

```c
SemaphoreHandle_t
```

Binary Semaphore:

```c
xSemaphoreCreateBinary()
```

Counting Semaphore:

```c
xSemaphoreCreateCounting()
```

Take:

```c
xSemaphoreTake()
```

Give:

```c
xSemaphoreGive()
```

Read current count:

```c
uxSemaphoreGetCount()
```

---

# Week 4 Key Takeaways

1. Semaphores are primarily used for synchronization and event signaling.

2. A Binary Semaphore has two logical states: available and unavailable.

3. A task can block while waiting for a semaphore.

4. `xSemaphoreGive()` can wake a task waiting on a semaphore.

5. A higher-priority task awakened by a semaphore may immediately preempt a lower-priority task.

6. Binary Semaphores cannot preserve the exact number of repeated events.

7. Multiple Give operations may be coalesced into a single available Binary Semaphore token.

8. Event coalescing is expected behavior for a Binary Semaphore.

9. Counting Semaphores can preserve multiple pending event tokens.

10. A Counting Semaphore has a configurable maximum count.

11. Counting Semaphores can represent pending events or available resources.

12. Queue and Semaphore have different purposes.

13. Queue is primarily used for transferring data.

14. Semaphore is primarily used for synchronization.

15. Blocking on a Semaphore avoids CPU-wasting busy polling.

---

# Week 4 Result

Week 4 successfully demonstrated:

- Binary Semaphore creation
- Task synchronization using Semaphore
- Blocking on Semaphore Take
- Wake-up using Semaphore Give
- Priority interaction and preemption
- Binary Semaphore event coalescing
- Counting Semaphore
- Pending event counting
- Maximum semaphore count
- Resource counting concept
- Difference between Queue and Semaphore

The Week 4 experiments demonstrate how FreeRTOS tasks can synchronize efficiently without continuously polling for events.

These Semaphore concepts provide the foundation for studying Mutex and shared-resource protection in Week 5.