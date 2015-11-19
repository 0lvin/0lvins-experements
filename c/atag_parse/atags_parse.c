/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */
/*
 * Magic values based on available kernel source from htc,
 * atag examples you can get from /proc/atag
 * CONFIG_KEXEC=y
 * CONFIG_ATAGS_PROC=y
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include/setup.h"

char* __pcbid_to_name(signed int id)
{
    switch (id) {
	case PROJECT_PHASE_INVALID: return "INVALID";
	case PROJECT_PHASE_EVM:     return "EVM";
	case PROJECT_PHASE_XA:      return "XA";
	case PROJECT_PHASE_XB:      return "XB";
	case PROJECT_PHASE_XC:      return "XC";
	case PROJECT_PHASE_XD:      return "XD";
	case PROJECT_PHASE_XE:      return "XE";
	case PROJECT_PHASE_XF:      return "XF";
	case PROJECT_PHASE_XG:      return "XG";
	case PROJECT_PHASE_XH:      return "XH";
	default:
	    return "<Latest HW phase>";
    }
}
int parse_atags(struct tag *tag) {
    if (tag->hdr.tag != ATAG_CORE) {
	printf("    No ATAGs?\n");
	return 0;
    }

    for (; tag->hdr.size; tag = tag_next(tag)) {
	printf("  tag:0x%x[0x%x]\n", tag->hdr.tag, tag->hdr.size);
	switch(tag->hdr.tag) {
	     /*case ATAG_ACORN:
	     case ATAG_AKM8976:
	     case ATAG_ALS:
	     case ATAG_BLUETOOTH:
	     case ATAG_BOARDINFO:
	     case ATAG_CAM:
	     case ATAG_CLOCK:*/
	     case ATAG_CMDLINE:
		printf(
			"    command line: %s\n",
			tag->u.cmdline.cmdline
		);
		break;
	     case ATAG_CORE:
		if (tag->hdr.size > 2) {
		    printf(
			"    Mount device %x:%x as %s\n",
			(tag->u.core.rootdev >> 8) & 0xff,
			tag->u.core.rootdev & 0xff,
			(tag->u.core.flags & 1) == 0 ? "rw" : "ro"
		    );
		};
		break;
	    /*case ATAG_CSA:
	    case ATAG_DDR_ID:*/
	    case ATAG_ENGINEERID: {
		    printf("    engineerid = 0x%x\n", tag->u.revision.rev);
		};
		break;
	    /*case ATAG_ETHERNET:
	    case ATAG_GRYO_GSENSOR:
	    case ATAG_GS:*/
	    case ATAG_FRAME_BUFFER_ID:
		printf("    fb addr= 0x%x, size=0x%0x\n", tag->u.mem.start, tag->u.mem.size);
		break;
	    case ATAG_HERO_PANEL_TYPE:
		printf("    paneltype = 0x%x\n", tag->u.revision.rev);
		break;
	    case ATAG_HTC_PCBID: { /* also can be ATAG_VIDEOLFB */
		    int pcbid = 0;
		    unsigned g_htc_pcbid;

		    /* parse ATAG_VIDEOLFB */
		    if (tag->hdr.size >= sizeof(struct tag_videolfb)) {
			printf("    lfb_width=0x%x\n", tag->u.videolfb.lfb_width);
			printf("    lfb_height=0x%x\n", tag->u.videolfb.lfb_height);
			printf("    lfb_depth=0x%x\n", tag->u.videolfb.lfb_depth);
			printf("    lfb_linelength=0x%x\n", tag->u.videolfb.lfb_linelength);
			printf("    lfb_base=0x%x\n", tag->u.videolfb.lfb_base);
			printf("    lfb_size=0x%x\n", tag->u.videolfb.lfb_size);
			printf("    red_size=0x%x\n", tag->u.videolfb.red_size);
			printf("    red_pos=0x%x\n", tag->u.videolfb.red_pos);
			printf("    green_size=0x%x\n", tag->u.videolfb.green_size);
			printf("    green_pos=0x%x\n", tag->u.videolfb.green_pos);
			printf("    blue_size=0x%x\n", tag->u.videolfb.blue_size);
			printf("    blue_pos=0x%x\n", tag->u.videolfb.blue_pos);
			printf("    rsvd_size=0x%x\n", tag->u.videolfb.rsvd_size);
			printf("    rsvd_pos=0x%x\n", tag->u.videolfb.rsvd_pos);
		    } else {
			pcbid = tag->u.revision.rev;
			g_htc_pcbid = (pcbid & (0xFF000000)) >> 24;
			printf("    pcbid = 0x%x (0x%x)\n", pcbid, g_htc_pcbid);
		    }
		};
		break;
	    case ATAG_HWID: {
		    int hwid = 0, find = 0;
		    hwid = tag->u.revision.rev;
		    printf("    hwid = 0x%x\n", hwid);
		};
		break;
	    case ATAG_INITRD:
		printf(
		    "    deprecated initrd found, start from virtual %x [%x]\n",
		    tag->u.initrd.start,
		    tag->u.initrd.size
		);
		break;
	    case ATAG_INITRD2:
		printf(
		    "    initrd started at %x[%x]\n",
		    tag->u.initrd.start,
		    tag->u.initrd.size
		);
		break;
	    /*case ATAG_MEM:
	    case ATAG_MEMCLK:
	    case ATAG_MEMSIZE:
	    case ATAG_MFG_GPIO_TABLE:*/
	    case ATAG_MSM_AWB_CAL: {
		    int i;
		    unsigned char *dptr = (unsigned char *)(&tag->u);
		    printf("    AWB_CAL size = 0x%x\n      ", tag->hdr.size);
		    for (i = 0; i < tag->hdr.size ; i++) {
			printf("0x%02x ", *dptr++);
			if ((i % 16) == 15)
			    printf("\n      ");
		    };
		    printf("\n");
		};
		break;
	    case ATAG_MSM_PARTITION: {
		    struct msm_ptbl_entry *entry = (void *) &tag->u;
		    unsigned count, n;
		    char name[16];
		    count = (tag->hdr.size - 2) /
			    (sizeof(struct msm_ptbl_entry) / sizeof(__u32));
		    printf("    partitions:\n");
		    for (n = 0; n < count; n++) {
			    memcpy(name, entry->name, 15);
			    name[15] = 0;
			    printf("      %16s 0x%2x[0x%06x]\n", name, entry->offset, entry->size * 2);
			    entry++;
		    }
		};
		break;
	    case ATAG_MSM_WIFI: {
		    int i;
		    unsigned char *dptr = (unsigned char *)(&tag->u);
		    printf("    WiFi Data size = 0x%x\n      ", tag->hdr.size);
		    for (i = 0; i < tag->hdr.size ; i++) {
			printf("0x%02x ", *dptr++);
			if ((i % 16) == 15)
			    printf("\n      ");
		    };
		    printf("\n");
		};
		break;
	    /*case ATAG_PS:*/
	    case ATAG_PS_TYPE:
		printf("    PS type = 0x%x\n", tag->u.revision.rev);
		break;
	    case ATAG_PCBID: {
		    unsigned char pcbid = PROJECT_PHASE_INVALID;
		    pcbid = tag->u.revision.rev;
		    printf("    pcbid = %s (0x%x)\n", __pcbid_to_name(pcbid), pcbid);
		};
		break;
	    case ATAG_TP_TYPE:
		printf("    touchpad type = 0x%x\n", tag->u.revision.rev);
		break;
	    /*case ATAG_RAMDISK:
	    case ATAG_RDIMG:*/
	    case ATAG_REVISION:
		printf("    system revision = 0x%x\n", tag->u.revision.rev);
		break;
	    case ATAG_MICROP_VERSION:
		if (tag->hdr.size>=sizeof(struct tag_microp_version)) {
		    printf(
			"    microo version = %c%c%c\n",
			tag->u.microp_version.ver[0],
			tag->u.microp_version.ver[1],
			tag->u.microp_version.ver[2]
		    );
		} else {
		    /* looks as ATAG_MEM_RESERVED */
		    printf("    mem reserved: 0x");
		    int i;
		    unsigned char *dptr = (unsigned char *)(&tag->u);
		    for (i = 0; i < tag->hdr.size ; i++) {
			printf("%02x", *dptr++);
		    };
		    printf("\n");
		}
		break;
	    /*case ATAG_RSVD_MEM:
	    case ATAG_SECURITY:
	    case ATAG_SERIAL:*/
	    case ATAG_SKUID: {
		    printf("    skuid = 0x%x\n", tag->u.revision.rev);
		};
		break;
	    case ATAG_SMI: {
		    printf("    smi size = %d\n", tag->u.mem.size);
		};
		break;
	    /*case ATAG_VIDEOTEXT:
	    case ATAG_WS:*/
	    default:
		printf("    Unknow tag %x[%d]\n", tag->hdr.tag, tag->hdr.size);

	};
    }

    if (tag->hdr.tag != ATAG_NONE) {
	printf("    atags file broken\n");
    }
    return 0;
}

int parse_atag_file(char* name) {
    printf("parse atag file: %s\n", name);
    int fd = open(name, O_RDONLY);
    if (fd < 0) {
	printf("    can't open atag file\n");
	return 0;
    }
    struct stat sb;
    int file_size = 0;
    if (fstat(fd, &sb) != 1) {
	file_size = sb.st_size;
    }
    if (!file_size)
	file_size = 1024;
    char* buffer = malloc(file_size + 1);
    int read_done = 0;
    int res = 0;
    while ((res = read(fd, buffer + read_done, 256)) > 0) {
	read_done += res;
	if ((read_done + 256) > file_size) {
	    buffer = realloc(buffer, file_size + 256);
	    file_size += 256;
	}
    }
    close(fd);
    buffer[read_done + 1] = 0;
    if (res < 0) {
	printf("    can't read\n");
	return 0;
    }
    file_size = read_done;

    return parse_atags((struct tag *)buffer);
}

int main(int argc, char **argv) {
    if (argc != 2) {
	printf("please use such format: %s file.atag\n", argv[0]);
    } else {
	parse_atag_file(argv[1]);
    }
    return 0;
}
