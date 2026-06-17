/* ARM ELF relocation types missing from musl's elf.h for non-ARM targets.
 * Values from ARM ELF ABI IHI0044 and glibc elf.h. */
#ifndef R_ARM_CALL
#define R_ARM_CALL              28
#endif
#ifndef R_ARM_JUMP24
#define R_ARM_JUMP24            29
#endif
#ifndef R_ARM_THM_JUMP24
#define R_ARM_THM_JUMP24        30
#endif
#ifndef R_ARM_MOVW_ABS_NC
#define R_ARM_MOVW_ABS_NC       43
#endif
#ifndef R_ARM_MOVT_ABS
#define R_ARM_MOVT_ABS          44
#endif
#ifndef R_ARM_THM_MOVW_ABS_NC
#define R_ARM_THM_MOVW_ABS_NC   47
#endif
#ifndef R_ARM_THM_MOVT_ABS
#define R_ARM_THM_MOVT_ABS      48
#endif
#ifndef R_ARM_THM_JUMP19
#define R_ARM_THM_JUMP19        51
#endif
