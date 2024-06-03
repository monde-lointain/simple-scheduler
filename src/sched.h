#pragma once

#include "types.h"

typedef enum
{
	TASK_IDLE,
	TASK_1,
	TASK_2,
	TASK_3,
	TASK_4,

	TASK_COUNT,
} TaskID;

typedef enum
{
	TASK_STATUS_READY,
	TASK_STATUS_BLOCKED,

	TASK_STATUS_COUNT,
} TaskStatus;

typedef struct
{
	u32 *sp;
	u32 blockCount;
	TaskStatus status;
	void (*taskFunc)(void);
} Task;

#define KILOBYTES(n) ((n) * 1024U)

#define DEFAULT_STACK_SIZE (KILOBYTES(1U))
#define SRAM_SIZE (KILOBYTES(128U))

#define SRAM_END (SRAM1_BASE + SRAM_SIZE)

#define T1_STACK_BASE (SRAM_END)
#define T2_STACK_BASE (T1_STACK_BASE - DEFAULT_STACK_SIZE)
#define T3_STACK_BASE (T2_STACK_BASE - DEFAULT_STACK_SIZE)
#define T4_STACK_BASE (T3_STACK_BASE - DEFAULT_STACK_SIZE)
#define IDLE_STACK_BASE (T4_STACK_BASE - DEFAULT_STACK_SIZE)
#define SCHED_STACK_BASE (IDLE_STACK_BASE - DEFAULT_STACK_SIZE)

#define HSI_CLOCK (16000000U)
#define SYSTICK_TIM_CLK HSI_CLOCK

void idle(void);

void task1(void);

void task2(void);

void task3(void);

void task4(void);