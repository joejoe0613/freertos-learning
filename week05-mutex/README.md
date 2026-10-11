# Week 5 - FreeRTOS Mutex and Shared Resource Protection

## Objective

This week focuses on:

- Race Condition
- Mutex
- Critical Section
- Priority Inversion
- Priority Inheritance

The main goal is to understand how FreeRTOS protects shared resources accessed by multiple tasks.

---

# Experiment 5A - Race Condition

Two tasks modify the same shared counter without protection.

```c
static volatile unsigned long sharedCounter = 0;
```

Both tasks perform:

```c
temp = sharedCounter;

taskYIELD();

temp++;

taskYIELD();

sharedCounter = temp;
```

Each task executes 1000 updates.

Expected result:

```text
Expected Counter = 2000
```

However, the actual value may be lower than 2000.

Example:

```text
Expected Counter = 2000
Actual Counter   = 1000
```

## What I Learned

`sharedCounter++` is not necessarily an atomic operation.

The operation can be divided into:

```text
Read
Modify
Write
```

If two tasks access the same variable at the same time, some updates may be lost.

This is a Race Condition.

Also:

```text
volatile != atomic
volatile != thread-safe
```

---

# Experiment 5B - Mutex Protection

A FreeRTOS Mutex was created:

```c
SemaphoreHandle_t xCounterMutex;

xCounterMutex = xSemaphoreCreateMutex();
```

Before accessing the shared counter:

```c
xSemaphoreTake(
    xCounterMutex,
    portMAX_DELAY
);
```

Critical Section:

```c
temp = sharedCounter;

taskYIELD();

temp++;

taskYIELD();

sharedCounter = temp;
```

After the operation:

```c
xSemaphoreGive(xCounterMutex);
```

The complete structure is:

```text
Take Mutex
    |
    v
Read
Modify
Write
    |
    v
Give Mutex
```

Result:

```text
Expected Counter = 2000
Actual Counter   = 2000
```

## What I Learned

A Mutex guarantees Mutual Exclusion.

Only one task can enter the protected Critical Section at a time.

If another task tries to take the same Mutex:

```text
Running
   |
   v
Mutex unavailable
   |
   v
Blocked
```

This prevents multiple tasks from modifying the shared resource simultaneously.

---

# Experiment 5C - Priority Inheritance

Three tasks were created:

```text
HIGH    Priority = 3
MEDIUM  Priority = 2
LOW     Priority = 1
```

The LOW task takes a Mutex first:

```c
xSemaphoreTake(
    xResourceMutex,
    portMAX_DELAY
);
```

HIGH later attempts to take the same Mutex.

Because LOW still owns the Mutex:

```text
HIGH
  |
  v
Blocked
```

Without additional protection, MEDIUM could continue running before LOW and delay the release of the Mutex.

This situation is called:

```text
Priority Inversion
```

FreeRTOS Mutex uses Priority Inheritance.

When HIGH waits for a Mutex held by LOW:

```text
LOW original priority = 1

        ↓

LOW temporary priority = 3
```

LOW can therefore finish its Critical Section sooner and release the Mutex.

After the Mutex is released, LOW returns to its original priority.

## What I Learned

Priority Inversion:

```text
HIGH waits for LOW
        +
MEDIUM prevents LOW from running
```

Priority Inheritance temporarily raises the Mutex owner's priority.

This reduces the delay experienced by the high-priority task.

---

# Mutex vs Binary Semaphore

Both use similar APIs:

```c
xSemaphoreTake()
xSemaphoreGive()
```

But their purposes are different.

```text
Binary Semaphore
→ Event signaling / synchronization

Mutex
→ Shared resource protection
```

A FreeRTOS Mutex also provides Priority Inheritance.

Therefore:

```text
Binary Semaphore != Mutex
```

---

# Critical Section

A very short operation can also be protected using:

```c
taskENTER_CRITICAL();

sharedCounter++;

taskEXIT_CRITICAL();
```

Critical Sections should be kept very short.

They are suitable for short low-level operations, while Mutexes are generally better for protecting shared resources between tasks.

---

# Important APIs

```c
xSemaphoreCreateMutex()

xSemaphoreTake()

xSemaphoreGive()

taskENTER_CRITICAL()

taskEXIT_CRITICAL()
```

---

# Week 5 Key Takeaways

- Shared resources can cause Race Conditions.
- Read-Modify-Write operations are not automatically atomic.
- Mutex provides Mutual Exclusion.
- Tasks waiting for a Mutex can enter the Blocked state.
- Critical Sections should be kept short.
- Priority Inversion can occur when a high-priority task waits for a resource owned by a low-priority task.
- FreeRTOS Mutex provides Priority Inheritance.
- Binary Semaphore and Mutex have different purposes.

---

# Week 5 Result

Week 5 successfully demonstrated:

```text
Race Condition
      ↓
Mutex Protection
      ↓
Critical Section
      ↓
Priority Inversion
      ↓
Priority Inheritance
```

These concepts are important when multiple RTOS tasks share the same resource.