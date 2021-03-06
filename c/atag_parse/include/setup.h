/*
 *  linux/include/asm/setup.h
 *
 *  Copyright (C) 1997-1999 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Structure passed to kernel to tell it about the
 *  hardware it's running on.  See Documentation/arm/Setup
 *  for more info.
 */
#ifndef __ASMARM_SETUP_H
#define __ASMARM_SETUP_H

#include <linux/types.h>

#define COMMAND_LINE_SIZE 1024

/* The list ends with an ATAG_NONE node. */
#define ATAG_NONE	0x00000000

struct tag_header {
	__u32 size;
	__u32 tag;
};

/* The list must start with an ATAG_CORE node */
#define ATAG_CORE	0x54410001

struct tag_core {
	__u32 flags;		/* bit 0 = read-only */
	__u32 pagesize;
	__u32 rootdev;
};

/* it is allowed to have multiple ATAG_MEM nodes */
#define ATAG_MEM	0x54410002

struct tag_mem32 {
	__u32	size;
	__u32	start;	/* physical start address */
};

/* VGA text type displays */
#define ATAG_VIDEOTEXT	0x54410003

struct tag_videotext {
	__u8		x;
	__u8		y;
	__u16		video_page;
	__u8		video_mode;
	__u8		video_cols;
	__u16		video_ega_bx;
	__u8		video_lines;
	__u8		video_isvga;
	__u16		video_points;
};

/* describes how the ramdisk will be used in kernel */
#define ATAG_RAMDISK	0x54410004

struct tag_ramdisk {
	__u32 flags;	/* bit 0 = load, bit 1 = prompt */
	__u32 size;	/* decompressed ramdisk size in _kilo_ bytes */
	__u32 start;	/* starting block of floppy-based RAM disk image */
};

/* describes where the compressed ramdisk image lives (virtual address) */
/*
 * this one accidentally used virtual addresses - as such,
 * it's deprecated.
 */
#define ATAG_INITRD	0x54410005

/* describes where the compressed ramdisk image lives (physical address) */
#define ATAG_INITRD2	0x54420005

struct tag_initrd {
	__u32 start;	/* physical start address */
	__u32 size;	/* size of compressed ramdisk image in bytes */
};

/* board serial number. "64 bits should be enough for everybody" */
#define ATAG_SERIAL	0x54410006

struct tag_serialnr {
	__u32 low;
	__u32 high;
};

/* board revision */
#define ATAG_REVISION	0x54410007

struct tag_revision {
	__u32 rev;
	__u32 rev2;
};

/* initial values for vesafb-type framebuffers. see struct screen_info
 * in include/linux/tty.h
 */
#define ATAG_VIDEOLFB	0x54410008

struct tag_videolfb {
	__u16		lfb_width;
	__u16		lfb_height;
	__u16		lfb_depth;
	__u16		lfb_linelength;
	__u32		lfb_base;
	__u32		lfb_size;
	__u8		red_size;
	__u8		red_pos;
	__u8		green_size;
	__u8		green_pos;
	__u8		blue_size;
	__u8		blue_pos;
	__u8		rsvd_size;
	__u8		rsvd_pos;
};

/* command line: \0 terminated string */
#define ATAG_CMDLINE	0x54410009

struct tag_cmdline {
	char	cmdline[1];	/* this is the minimum size */
};

/* acorn RiscPC specific information */
#define ATAG_ACORN	0x41000101

struct tag_acorn {
	__u32 memc_control_reg;
	__u32 vram_pages;
	__u8 sounddefault;
	__u8 adfsdrives;
};

/* footbridge memory clock, see arch/arm/mach-footbridge/arch.c */
#define ATAG_MEMCLK	0x41000402

struct tag_memclk {
	__u32 fmemclk;
};

/* Light sensor calibration value */
#define ATAG_ALS	0x5441001b

struct tag_als_kadc {
	__u32 kadc;
};

/*
 * configuration tags specific to msm
 * (0x4d534D == MSM)
 */
#define ATAG_SMI		0x4d534D71
#define ATAG_HWID		0x4d534D72
#define ATAG_SKUID		0x4d534D73
#define ATAG_HERO_PANEL_TYPE	0x4d534D74
#define ATAG_ENGINEERID		0x4d534D75
#define ATAG_HTC_PCBID		0x54410008
#define ATAG_PCBID		0x4d534D76
#define ATAG_MSM_PARTITION	0x4d534D70
#define ATAG_PS_TYPE		0x4d534D77
/*get framebuffer address and size from atag, passed by bootloader*/
#define ATAG_FRAME_BUFFER_ID	0x4d534D79
 /* Touch Controller ID values */
#define ATAG_TP_TYPE		0x4d534D78
/* Microp version */
#define ATAG_MICROP_VERSION	0x5441000a
/* it is allowed to have multiple ATAG_MEM_RESERVED nodes */
/* these indicate places where hotpluggable memory is present */
/* which are not active during boot */
#define ATAG_MEM_RESERVED	0x5441000A
/* MSM CAMERA AWB Calibration */
#define ATAG_MSM_AWB_CAL	0x59504550
/* MSM WiFi */
#define ATAG_MSM_WIFI		0x57494649
#define ATAG_MFG_GPIO_TABLE 	0x59504551
#define ATAG_BLUETOOTH		0x43294329
#define ATAG_PS			0x5441001c
#define ATAG_GS			0x5441001d
#define ATAG_MEMSIZE		0x5441001e
#define ATAG_TP			0x41387898
#define ATAG_WS			0x54410023
#define ATAG_BLDR_LOG 		0x54410024
#define ATAG_LAST_BLDR_LOG	0x54410025
#define ATAG_BATT_DATA		0x54410027
#define ATAG_CAM		0x54410021
#define ATAG_GRYO_GSENSOR 	0x54410020
#define ATAG_CSA 		0x5441001f
/* radio security */
#define ATAG_SECURITY		0x54410022

struct tag_batt_data {
	__s32 magic_num;
	__s32 soc;
	__s32 ocv;
	__s32 cc;
	__u32 currtime;
};
struct tag_bldr_log {
	__u32 addr;
	__u32 size;
};

struct tag_last_bldr_log {
	__u32 addr;
	__u32 size;
};


struct tag_microp_version {
	char ver[4];
};

struct msm_ptbl_entry
{
	char name[16];
	__u32 offset;
	__u32 size;
	__u32 flags;
};

enum {
    PROJECT_PHASE_INVALID = -2,
    PROJECT_PHASE_EVM = -1,
    PROJECT_PHASE_XA  =  0,
    PROJECT_PHASE_XB  =  1,
    PROJECT_PHASE_XC  =  2,
    PROJECT_PHASE_XD  =  3,
    PROJECT_PHASE_XE  =  4,
    PROJECT_PHASE_XF  =  5,
    PROJECT_PHASE_XG  =  6,
    PROJECT_PHASE_XH  =  7,
    PROJECT_PHASE_A   =  0x80,
    PROJECT_PHASE_B   =  0x81,
    PROJECT_PHASE_C   =  0x82,
    PROJECT_PHASE_D   =  0x83,
    PROJECT_PHASE_E   =  0x84,
    PROJECT_PHASE_F   =  0x85,
    PROJECT_PHASE_G   =  0x86,
    PROJECT_PHASE_H   =  0x87,
};

/* end magic tags */

struct tag {
	struct tag_header hdr;
	union {
		struct tag_core		core;
		struct tag_mem32	mem;
		struct tag_videotext	videotext;
		struct tag_ramdisk	ramdisk;
		struct tag_initrd	initrd;
		struct tag_serialnr	serialnr;
		struct tag_revision	revision;
		struct tag_microp_version	microp_version;
		struct tag_videolfb	videolfb;
		struct tag_cmdline	cmdline;
		struct tag_als_kadc als_kadc;
		/*
		 * Acorn specific
		 */
		struct tag_acorn	acorn;

		/*
		 * DC21285 specific
		 */
		struct tag_memclk	memclk;

		/*
		 * msm only
		 */
		struct tag_bldr_log	bldr_log;
		struct tag_last_bldr_log last_bldr_log;
		struct tag_batt_data	batt_data;
	} u;
};

struct tagtable {
	__u32 tag;
	int (*parse)(const struct tag *);
};

#define tag_member_present(tag,member)				\
	((unsigned long)(&((struct tag *)0L)->member + 1)	\
		<= (tag)->hdr.size * 4)

#define tag_next(t)	((struct tag *)((__u32 *)(t) + (t)->hdr.size))
#define tag_size(type)	((sizeof(struct tag_header) + sizeof(struct type)) >> 2)

#define for_each_tag(t,base)		\
	for (t = base; t->hdr.size; t = tag_next(t))

#ifdef __KERNEL__

#define __tag __used __attribute__((__section__(".taglist.init")))
#define __tagtable(tag, fn) \
static const struct tagtable __tagtable_##fn __tag = { tag, fn }

/*
 * Memory map description
 */
#define NR_BANKS	CONFIG_ARM_NR_BANKS

struct membank {
	phys_addr_t start;
	unsigned long size;
	unsigned int highmem;
};

struct meminfo {
	int nr_banks;
	struct membank bank[NR_BANKS];
};

extern struct meminfo meminfo;

#define for_each_bank(iter,mi)				\
	for (iter = 0; iter < (mi)->nr_banks; iter++)

#define bank_pfn_start(bank)	__phys_to_pfn((bank)->start)
#define bank_pfn_end(bank)	(__phys_to_pfn((bank)->start) + \
						__phys_to_pfn((bank)->size))
#define bank_pfn_size(bank)	((bank)->size >> PAGE_SHIFT)
#define bank_phys_start(bank)	(bank)->start
#define bank_phys_end(bank)	((bank)->start + (bank)->size)
#define bank_phys_size(bank)	(bank)->size

extern int arm_add_memory(phys_addr_t start, unsigned long size);
extern void early_print(const char *str, ...);
extern void dump_machine_table(void);

/*
 * Early command line parameters.
 */
struct early_params {
	const char *arg;
	void (*fn)(char **p);
};

#define __early_param(name,fn)					\
static struct early_params __early_##fn __used			\
__attribute__((__section__(".early_param.init"))) = { name, fn }

#endif  /*  __KERNEL__  */

#endif
