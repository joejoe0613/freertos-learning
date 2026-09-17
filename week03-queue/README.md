# Week 3 - FreeRTOS Queue and Task Communication

## Objective

The goal of Week 3 is to understand how FreeRTOS tasks exchange data safely using queues.

Topics covered:

- Inter-task communication
- FreeRTOS Queue
- Producer-Consumer model
- FIFO behavior
- Queue blocking
- Queue full / Queue empty conditions
- Backpressure
- Structured message transfer

---

## Environment

- Linux Mint
- GCC
- FreeRTOS-Kernel
- FreeRTOS POSIX Port

The FreeRTOS POSIX port is used to execute the FreeRTOS kernel directly on Linux.

Project structure:

```text
week03-queue/
├── README.md
├── Makefile
├── include/
│   └── FreeRTOSConfig.h
├── src/
│   └── main.c
└── output/
    ├── basic-queue.txt
    ├── queue-full-blocking.txt
    └── structured-messages.txt
```

---

# Queue Concept

A FreeRTOS Queue provides a thread-safe way for tasks to exchange data.

The basic model is:

```text
Producer Task
     |
     | xQueueSend()
     v
+-----------+
|   Queue   |
+-----------+
     |
     | xQueueReceive()
     v
Consumer Task
```

A Queue normally follows FIFO behavior:

```text
First In
First Out
```

Example:

```text
Producer sends:

10
20
30

Consumer receives:

10
20
30
```

---

# Why Use a Queue?

A shared global variable could be used to exchange data between tasks:

```c
int sharedValue;
```

However, direct shared-memory communication can cause problems such as:

```text
Race conditions
Data overwrite
Synchronization problems
Timing dependencies
```

A FreeRTOS Queue provides:

```text
Data storage
FIFO ordering
Synchronization
Blocking
Safe task-to-task communication
```

This allows tasks to exchange data without continuously polling shared variables.

---

# Important FreeRTOS Queue APIs

## xQueueCreate()

Creates a new queue.

Example:

```c
xQueueCreate(
    5,
    sizeof(int)
);
```

This creates a queue that stores:

```text
5 items
```

where each item has the size:

```text
sizeof(int)
```

Conceptually:

```text
[ int ][ int ][ int ][ int ][ int ]
```

---

## xQueueSend()

Sends data into the queue.

Example:

```c
int value = 10;

xQueueSend(
    xDataQueue,
    &value,
    portMAX_DELAY
);
```

The queue copies the value into its internal storage.

The queue does not simply store the address of the local variable.

---

## xQueueReceive()

Receives and removes the oldest item from the queue.

Example:

```c
int receivedValue;

xQueueReceive(
    xDataQueue,
    &receivedValue,
    portMAX_DELAY
);
```

If the queue contains:

```text
[10][20][30]
```

after one receive:

```text
receivedValue = 10
```

and the queue becomes:

```text
[20][30]
```

---

# Experiment 3A - Basic Producer / Consumer Queue

Two tasks were created:

```text
Producer Task
Consumer Task
```

The Producer generates one integer every 500 ms.

The Consumer waits for data from the queue.

Configuration:

```text
Queue length = 5
Item type    = int
```

Basic architecture:

```text
Producer
   |
   | integer
   v
[ Queue ]
   |
   v
Consumer
```

The Producer used:

```c
xQueueSend(
    xDataQueue,
    &value,
    portMAX_DELAY
);
```

The Consumer used:

```c
xQueueReceive(
    xDataQueue,
    &receivedValue,
    portMAX_DELAY
);
```

---

## Expected Behavior

Example output:

```text
[Producer] sending 1
[Consumer] received 1

[Producer] sending 2
[Consumer] received 2

[Producer] sending 3
[Consumer] received 3
```

The Consumer receives the same data in the same order in which the Producer sends it.

---

## Queue Empty Behavior

When the Consumer calls:

```c
xQueueReceive(
    xDataQueue,
    &receivedValue,
    portMAX_DELAY
);
```

and the queue is empty:

```text
Consumer
Running
   |
   | xQueueReceive()
   v
Queue Empty
   |
   v
Blocked
```

The Consumer does not continuously poll the queue.

When the Producer sends new data:

```text
Producer
   |
   | xQueueSend()
   v
Queue receives data
   |
   v
Consumer
Blocked -> Ready
```

This avoids unnecessary CPU usage.

---

# Experiment 3B - Queue Full and Backpressure

The Producer was configured to generate data faster than the Consumer could process it.

Configuration example:

```text
Queue length = 3

Producer period = 200 ms
Consumer processing time = 1000 ms
```

The Queue therefore gradually fills.

Example:

```text
Producer sends 2
Producer sends 3
Producer sends 4

Queue:

[2][3][4]
```

The Queue is now full.

---

## Producer Blocking

If the Producer calls:

```c
xQueueSend(
    xDataQueue,
    &value,
    portMAX_DELAY
);
```

while the queue is full, the Producer enters the Blocked state.

```text
Producer
Running
   |
   | Queue Full
   v
Blocked
```

The Producer does not overwrite the existing queue data.

It waits until the Consumer removes an item.

---

## Consumer Removes an Item

Suppose the queue contains:

```text
[3][4][5]
```

The Consumer receives one item:

```text
received = 3
```

The queue becomes:

```text
[4][5][ ]
```

A free slot is now available.

A Producer that was blocked waiting to send data can return to the Ready state.

```text
Producer
Blocked
   |
   | Queue has free space
   v
Ready
```

---

# Backpressure

This behavior demonstrates backpressure.

Backpressure occurs when:

```text
Producer
is faster than
Consumer
```

The queue temporarily stores pending data.

When the queue becomes full, the Producer is automatically blocked.

This prevents the Producer from continuously producing data that the Consumer cannot handle.

Conceptually:

```text
Fast Producer
     |
     v
[ Queue fills ]
     |
     v
Queue Full
     |
     v
Producer Blocked
```

---

# Queue Depth

The current number of messages in a queue can be checked using:

```c
uxQueueMessagesWaiting(xDataQueue)
```

Example:

```text
Queue items = 1
Queue items = 2
Queue items = 3
```

This makes it possible to observe the queue filling during the experiment.

---

# Experiment 3C - Structured Messages

The previous experiments transferred only integers.

Real embedded systems often need to transfer multiple pieces of information together.

A structured message was therefore created.

Example:

```c
typedef struct
{
    int sensorId;
    int value;
    TickType_t timestamp;

} SensorMessage_t;
```

Each message contains:

```text
sensorId
value
timestamp
```

---

# Structured Message Queue

The queue was created using:

```c
xDataQueue = xQueueCreate(
    5,
    sizeof(SensorMessage_t)
);
```

The queue therefore stores complete:

```text
SensorMessage_t
```

objects instead of individual integers.

Architecture:

```text
Sensor Task
     |
     | SensorMessage_t
     v
+--------------------+
|       Queue        |
+--------------------+
     |
     v
Logger Task
```

---

# Sensor Task

The Sensor Task creates messages such as:

```text
Sensor ID = 1
Value     = 20
Timestamp = 1
```

The experiment simulated several sensor IDs.

Example output:

```text
[Sensor] ID=1 Value=20 Timestamp=1
[Logger] ID=1 Value=20 Timestamp=1

[Sensor] ID=2 Value=51 Timestamp=501
[Logger] ID=2 Value=51 Timestamp=501

[Sensor] ID=3 Value=102 Timestamp=1001
[Logger] ID=3 Value=102 Timestamp=1001
```

This makes it clear that the queue transfers the complete message structure.

---

# Queue Data Copy

When the Producer sends:

```c
xQueueSend(
    xDataQueue,
    &message,
    portMAX_DELAY
);
```

FreeRTOS copies the contents of:

```text
message
```

into the queue.

For example:

```text
Producer:

ID        = 2
Value     = 51
Timestamp = 501
```

is copied into the queue.

The Logger later receives:

```text
Consumer:

ID        = 2
Value     = 51
Timestamp = 501
```

The original local message variable can then be modified without changing the copy already stored in the queue.

---

# Queue vs Shared Variable

Using a shared variable:

```text
Producer
   |
shared variable
   |
Consumer
```

can cause timing and synchronization problems.

Using a Queue:

```text
Producer
   |
   v
[ Queue ]
   |
   v
Consumer
```

provides controlled data transfer.

The queue allows the Producer and Consumer to operate at different speeds while preserving pending data.

---

# Queue Full vs Queue Empty

## Queue Empty

```text
Consumer attempts Receive
        |
        v
Queue Empty
        |
        v
Consumer Blocked
```

When data arrives:

```text
Producer Send
        |
        v
Consumer Ready
```

---

## Queue Full

```text
Producer attempts Send
        |
        v
Queue Full
        |
        v
Producer Blocked
```

When the Consumer removes data:

```text
Consumer Receive
        |
        v
Free Queue Slot
        |
        v
Producer Ready
```

---

# Task State Relationship

Queue operations are closely related to the task states studied in Week 1 and Week 2.

For example:

```text
Consumer
Running
   |
   | xQueueReceive()
   v
Blocked
   |
   | Message arrives
   v
Ready
   |
   | Scheduler selects task
   v
Running
```

The same concept applies to a Producer waiting for free queue space.

---

# Important Week 3 APIs

```c
xQueueCreate()
xQueueSend()
xQueueReceive()
uxQueueMessagesWaiting()
```

Queue type:

```c
QueueHandle_t
```

Example:

```c
static QueueHandle_t xDataQueue = NULL;
```

---

# Week 3 Key Takeaways

1. FreeRTOS Queue provides safe task-to-task data communication.

2. Queue items are normally processed in FIFO order.

3. `xQueueSend()` copies data into Queue storage.

4. `xQueueReceive()` copies data out of the Queue and removes that item.

5. A Consumer can block when the Queue is empty.

6. A Producer can block when the Queue is full.

7. Blocking prevents tasks from wasting CPU time through busy polling.

8. A Queue can absorb temporary differences between Producer and Consumer speeds.

9. Queue Full behavior provides backpressure to a fast Producer.

10. Queue item types can be simple values or complete structures.

11. Structured messages are useful for transferring information such as sensor ID, value, and timestamp.

12. Queue synchronization behavior is directly related to the FreeRTOS Ready, Running, and Blocked task states.

---

# Week 3 Result

Week 3 successfully demonstrated:

- FreeRTOS inter-task communication
- Queue creation
- Producer-Consumer architecture
- FIFO data transfer
- Queue Empty blocking
- Queue Full blocking
- Backpressure
- Queue depth observation
- Structured message transfer
- Task synchronization through Queue operations

The Week 3 experiments demonstrate how FreeRTOS tasks can safely exchange data without relying on unsafe shared variables.

These Queue concepts provide the foundation for more advanced RTOS communication and synchronization mechanisms.
