#ifndef _CPU_TCKS_H_
#define _CPU_TCKS_H_ 1

#include <cstdint>

typedef enum {
	TCK_USER = 0,
	TCK_NICE,
	TCK_SYSTEM,
	TCK_IDLE,
	TCK_IOWAIT,
	TCK_IRQ,
	TCK_SOFTIRQ,
	TCK_STEAL,
	TCK_GUEST,
	TCK_GUEST_NICE,
	NUM_TCK_TYPES
} tck_type;

typedef struct {
	uint64_t tcks[NUM_TCK_TYPES];
} tck_t;

#endif
