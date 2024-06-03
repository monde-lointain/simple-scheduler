#include "stm32f4xx.h"

#include "led.h"
#include "sched.h"

#include "led.c"

#define WRAP_INC(n, max) ((n) = (++(n) == (max)) ? 0 : (n))

// clang-format off
Task tasks[TASK_COUNT] =
{
  //      sp         block count      status         proc
    {IDLE_STACK_BASE,    0,      TASK_STATUS_READY, &idle},
    {T1_STACK_BASE,      0,      TASK_STATUS_READY, &task1},
    {T2_STACK_BASE,      0,      TASK_STATUS_READY, &task2},
    {T3_STACK_BASE,      0,      TASK_STATUS_READY, &task3},
    {T4_STACK_BASE,      0,      TASK_STATUS_READY, &task4},
};
u32 currentTask = TASK_1;
// clang-format on

u32 gTickCount = 0;

static void enableProcessorFaults(void)
{
    SET_BIT(SCB->SHCSR, SCB_SHCSR_MEMFAULTENA_Msk);
    SET_BIT(SCB->SHCSR, SCB_SHCSR_BUSFAULTENA_Msk);
    SET_BIT(SCB->SHCSR, SCB_SHCSR_USGFAULTENA_Msk);
}

u32 getPSPValue(void)
{
    return tasks[currentTask].sp;
}

void storePSPValue(u32 currStackBase)
{
    tasks[currentTask].sp = currStackBase;
}

void updateNextTask(void)
{
    // Search for a task that is ready
    for (u32 i = 0; i < TASK_COUNT; i++)
    {
        WRAP_INC(currentTask, TASK_COUNT);
        if ((tasks[currentTask].status == TASK_STATUS_READY) && (currentTask != TASK_IDLE))
        {
            return;
        }
    }

    // If no tasks found
    currentTask = TASK_IDLE;
}

static void initSchedulerStack(u32 stackBase)
{
    __set_MSP(stackBase);
}

static void schedule(void)
{
    SET_BIT(SCB->ICSR, SCB_ICSR_PENDSVSET_Msk);
}

static void taskDelay(u32 tickCount)
{
    // Enter critical section
    u32 primask = __get_PRIMASK();
    __set_PRIMASK(1);

    if (currentTask != TASK_IDLE)
    {
        tasks[currentTask].blockCount = gTickCount + tickCount;
        tasks[currentTask].status = TASK_STATUS_BLOCKED;
        schedule();
    }

    __set_PRIMASK(primask);
}

static void unblockTasks(void)
{
    for (u32 i = TASK_1; i < TASK_COUNT; i++)
    {
        Task *task = &tasks[i];
        if ((task->blockCount == gTickCount) && (task->status != TASK_STATUS_READY))
        {
            task->status = TASK_STATUS_READY;
        }
    }
}

static void initTasks(void)
{
    // Set up stack frame for each task
    for (u32 i = 0; i < TASK_COUNT; i++)
    {
        u32 *sp = (u32 *)tasks[i].sp;

        *--sp = 0x1000000U;             // xpsr
        *--sp = (u32)tasks[i].taskFunc; // lr
        *--sp = EXC_RETURN_THREAD_PSP;  // pc

        // r0 - r12
        for (u32 j = 0; j < 13; j++)
        {
            *--sp = 0;
        }

        tasks[i].sp = (u32)sp;
    }
}

static void initTimer(u32 tickHz)
{
    u32 countValue = SYSTICK_TIM_CLK / tickHz;

    // Configure systick
    CLEAR_BIT(SysTick->LOAD, SysTick_LOAD_RELOAD_Msk);
    SET_BIT(SysTick->LOAD, countValue);

    SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
    SET_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk);

    // Enable
    SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
}

__attribute__((naked)) void switchSPToPSP(void)
{
    __ASM volatile("push {lr}       \n\t"
                   "bl getPSPValue  \n\t" // Get current PSP address and store in r0
                   "msr psp, r0     \n\t" // Move into PSP
                   "pop {lr}        \n\t"
                   "mov r0, #0x02   \n\t" // Set SP to use PSP
                   "msr control, r0 \n\t"
                   "bx lr");
}

static void ledMain(u32 pin, u32 toggleFreq)
{
    while (true)
    {
        ledOn(pin);
        taskDelay(toggleFreq);
        ledOff(pin);
        taskDelay(toggleFreq);
    }
}

void task1(void)
{
    ledMain(LED_GREEN, 1000);
}

void task2(void)
{
    ledMain(LED_ORANGE, 500);
}

void task3(void)
{
    ledMain(LED_BLUE, 250);
}

void task4(void)
{
    ledMain(LED_RED, 125);
}

int main(void)
{
    enableProcessorFaults();

    initSchedulerStack(SCHED_STACK_BASE);

    initTasks();

    ledInitAll();

    initTimer(1000);

    switchSPToPSP();

    task1();
}

void SysTick_Handler(void)
{
    gTickCount++;
    unblockTasks();
    schedule();
}

__attribute__((naked)) void PendSV_Handler(void)
{
    __ASM volatile("mrs r0, psp         \n\t" // Retrieve current PSP and
                   "stmdb r0!, {r4-r11} \n\t" // push SF2 registers to it

                   "push {lr}           \n\t"
                   "bl storePSPValue    \n\t" // Save current value of PSP

                   "bl updateNextTask   \n\t" // Update next task to run

                   "bl getPSPValue      \n\t" // Get PSP of next task

                   "ldmia r0!, {r4-r11} \n\t" // Restore registers and
                   "msr psp, r0         \n\t" // stack frame

                   "pop {lr}            \n\t"
                   "bx lr");
}

void idle(void)
{
    while (true)
    {
    }
}