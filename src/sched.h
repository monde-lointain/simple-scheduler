#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;

typedef volatile uint8_t vu8;
typedef volatile uint16_t vu16;
typedef volatile uint32_t vu32;
typedef volatile uint64_t vu64;
typedef volatile int8_t vs8;
typedef volatile int16_t vs16;
typedef volatile int32_t vs32;
typedef volatile int64_t vs64;
typedef volatile float vf32;
typedef volatile double vf64;

enum TaskID
{
	TASK_IDLE,
	TASK_1,
	TASK_2,
	TASK_3,
	TASK_4,

	TASK_COUNT,
};

typedef enum
{
	TASK_STATUS_READY,
	TASK_STATUS_BLOCKED,

	TASK_STATUS_COUNT,
} TaskStatus;

typedef struct
{
	u32 sp;
	u32 blockCount;
	TaskStatus status;
	void (*taskFunc)(void);
} Task;

#define KILOBYTES(n) ((n) * 1024U)

#define DEFAULT_STACK_SIZE (KILOBYTES(1))
#define SRAM_SIZE (KILOBYTES(128))

#define SRAM_END (SRAM1_BASE + SRAM_SIZE)

#define T1_STACK_BASE (SRAM_END)
#define T2_STACK_BASE (T1_STACK_BASE - DEFAULT_STACK_SIZE)
#define T3_STACK_BASE (T2_STACK_BASE - DEFAULT_STACK_SIZE)
#define T4_STACK_BASE (T3_STACK_BASE - DEFAULT_STACK_SIZE)
#define IDLE_STACK_BASE (T4_STACK_BASE - DEFAULT_STACK_SIZE)
#define SCHED_STACK_BASE (IDLE_STACK_BASE - DEFAULT_STACK_SIZE)

#define HSI_CLOCK (16000000UL)
#define SYSTICK_TIM_CLK HSI_CLOCK

void idle(void);

void task1(void);

void task2(void);

void task3(void);

void task4(void);