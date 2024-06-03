#include "led.h"
#include "sched.h"

#include "stm32f4xx.h"

#define WRAP_INC(n, max) ((n) = (++(n) == (max)) ? 0U : (n))

// clang-format off
static Task tasks[TASK_COUNT] =
{
  //      sp         block count      status         proc
    {(u32 *)IDLE_STACK_BASE,    0U,     TASK_STATUS_READY, &idle},
    {(u32 *)T1_STACK_BASE,      0U,     TASK_STATUS_READY, &task1},
    {(u32 *)T2_STACK_BASE,      0U,     TASK_STATUS_READY, &task2},
    {(u32 *)T3_STACK_BASE,      0U,     TASK_STATUS_READY, &task3},
    {(u32 *)T4_STACK_BASE,      0U,     TASK_STATUS_READY, &task4},
};
static u32 currentTask = TASK_1;
// clang-format on

static u32 gTickCount = 0;

static void enableProcessorFaults(void)
{
    SET_BIT(SCB->SHCSR, SCB_SHCSR_MEMFAULTENA_Msk);
    SET_BIT(SCB->SHCSR, SCB_SHCSR_BUSFAULTENA_Msk);
    SET_BIT(SCB->SHCSR, SCB_SHCSR_USGFAULTENA_Msk);
}

u32 *getPSPValue(void);
u32 *getPSPValue(void)
{
    return tasks[currentTask].sp;
}

void storePSPValue(u32 *currStackBase);
void storePSPValue(u32 *currStackBase)
{
    tasks[currentTask].sp = currStackBase;
}

void updateNextTask(void);
void updateNextTask(void)
{
    // Search for a task that is ready
    for (u32 i = 0; i < (u32)TASK_COUNT; i++)
    {
        currentTask = (currentTask + 1) % (u32)TASK_COUNT;
        if ((tasks[currentTask].status == TASK_STATUS_READY) && (currentTask != (u32)TASK_IDLE))
        {
            return;
        }
    }

    // If no tasks found
    currentTask = (u32)TASK_IDLE;
}

static void initSchedulerStack(u32 stackBase)
{
    __set_MSP(stackBase);
}

static void schedule(void)
{
    SET_BIT(SCB->ICSR, SCB_ICSR_PENDSVSET_Msk);
    __ISB();
}

static void taskDelay(u32 tickCount)
{
    // Enter critical section
    u32 primask = __get_PRIMASK();
    __set_PRIMASK(1);

    if (currentTask != (u32)TASK_IDLE)
    {
        tasks[currentTask].blockCount = gTickCount + tickCount;
        tasks[currentTask].status = TASK_STATUS_BLOCKED;
        schedule();
    }

    __set_PRIMASK(primask);
}

static void unblockTasks(void)
{
    for (u32 i = TASK_1; i < (u32)TASK_COUNT; i++)
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
    for (register u32 i = 0; i < (u32)TASK_COUNT; i++)
    {
        u32 *taskSP = tasks[i].sp;

        *--taskSP = 0x1000000U;             // xpsr
        *--taskSP = (u32)tasks[i].taskFunc; // lr
        *--taskSP = EXC_RETURN_THREAD_PSP;  // pc

        // r0 - r12
        for (u32 j = 0; j < 13U; j++)
        {
            *--taskSP = 0U;
        }

        tasks[i].sp = taskSP;
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

static __attribute__((naked)) void switchSPToPSP(void)
{
    __ASM volatile("push {lr}       \n\t"
                   "bl getPSPValue  \n\t" // Get current PSP address and store in r0
                   "msr psp, r0     \n\t" // Move into PSP
                   "pop {lr}        \n\t"
                   "mov r0, #0x02   \n\t" // Set SP to use PSP
                   "msr control, r0 \n\t"
                   "bx lr");
}

void task1(void)
{
    while (true)
    {
        ledOn(LED_GREEN);
        taskDelay(1000);
        ledOff(LED_GREEN);
        taskDelay(1000);
    }
}

void task2(void)
{
    while (true)
    {
        ledOn(LED_ORANGE);
        taskDelay(500);
        ledOff(LED_ORANGE);
        taskDelay(500);
    }
}

void task3(void)
{
    while (true)
    {
        ledOn(LED_BLUE);
        taskDelay(250);
        ledOff(LED_BLUE);
        taskDelay(250);
    }
}

void task4(void)
{
    while (true)
    {
        ledOn(LED_RED);
        taskDelay(125);
        ledOff(LED_RED);
        taskDelay(125);
    }
}

int main(void);
int main(void)
{
    enableProcessorFaults();

    initSchedulerStack(SCHED_STACK_BASE);

    initTasks();

    ledInitAll();

    initTimer(1000);

    switchSPToPSP();

    task1();

    while (true)
    {
    }
}

void SysTick_Handler(void);
void SysTick_Handler(void)
{
    gTickCount++;
    unblockTasks();
    schedule();
}

__attribute__((naked)) void PendSV_Handler(void);
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
