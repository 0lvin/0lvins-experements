#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

#define handle_error(msg) \
   do { perror(msg); exit(-1); } while (0)

/*
http://en.wikipedia.org/wiki/Graphics_Interchange_Format
byte#  hexadecimal  text or
(hex)               value      Meaning
0:     47 49 46
       38 39 61     GIF89a     Header
                               Logical Screen Descriptor
6:     90 01        400         - width pixels
8:     90 01        400         - height pixels
A:     F7                       - GCT follows for 256 colors with resolution 3 x 8bits/primary
B:     00           0           - background color #0
C:     00                       - default aspect ratio
D:                             Global Color Table
:
30D:   21 FF 0B                Application Extension
310:   4E 45 54
       53 43 41
       50 45 32
       2E 30        NETSCAPE2.0
31B:   03 01                    - data follows
31D:   FF FF                    - loop animation
31F:   00                       - end
320:   21 F9 04                Graphic Control Extension frame #1
323:   08                       - no transparency
324:   09 00                    - 0.09 sec duration
325:   00                       - no transparent color
327:   00                       - end
328:   2C                      Image Descriptor
329:   00 00 00 00  (0,0)       - scan pixels from left top...
32D:   90 01 90 01  (400,400)   - ...to right bottom
331:   00                       - end
332:   08           8          LZW min code size
333:   FF           255        255 bytes LZW encoded image data follow
334:                data
433:   FF           255        255 bytes LZW encoded image data follow
                    data
                     :
92BA:  00                      end
92BB:  21 F9 04                Graphic Control Extension frame #2
 :                                                              :
153B7B:21 F9 04                Graphic Control Extension frame #44
 :
15CF35:3B           1 429 301  File terminates
 */
int main(int argc, char *argv[]) {
	if(argc != 2)
		handle_error("Wrong count params\n");
	int fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		handle_error("Error in open\n");
	unsigned char *addr;
	lseek(fd, SEEK_SET, 0);
	size_t length = lseek(fd, 0, SEEK_END);
	lseek(fd, SEEK_SET, 0);
	
	if (length < 13) //header size
		handle_error("sorry too short");
	addr = mmap(NULL, length, PROT_READ,
			   MAP_PRIVATE, fd, 0);
	if (addr == MAP_FAILED)
	   handle_error("mmap");

	if (memcmp("GIF", addr, 3))
		handle_error("Not gif\n");

	if (!(memcmp("89a", addr+3, 3) == 0 || memcmp("87a", addr+3, 3) == 0))
		handle_error("wrong format");
	
	int width = 0;
	memcpy(&width, addr + 6, 2); // 6 
	printf("width: %d\n",width);
	
	int height = 0;
	memcpy(&height, addr + 8, 2); // 8
	printf("height: %d\n",height);
	
	int gct = 0; // A: GCT follows for 256 colors with resolution 3 x 8bits/primary
	memcpy(&gct, addr + 10, 1);
	int global_bit_pixel = 2 << (gct & 0x07); ////Size of Global Color Table    3 Bits
	printf("Size of Global Color Table: %d\n", global_bit_pixel);
	printf("Sort Flag: %d\n",(gct&0x8)); //Sort Flag                     1 Bit
	int global_color_resolution = (((gct & 0x70) >> 3) + 1); //Color Resolution              3 Bits
	printf("Color Resolution: %d\n", global_color_resolution);
	int has_global_cmap = (gct & 0x80) != 0;//Global Color Table Flag       1 Bit
	printf("Color table exist: %d\n", has_global_cmap); 
	int background_color = 0; //B: background color index
	memcpy(&background_color, addr + 11, 1);
	printf("background_color: %d\n",background_color);

	int aspect = 0; //C: default aspect ratio
	memcpy(&aspect, addr + 12, 1);
	printf("aspect: %d\n",aspect);

	char * color_table = NULL;
	int last_decoded = 13;
	if (has_global_cmap) {
		if ((13 + global_bit_pixel * 3) > length)
			handle_error("sorry too short");
		last_decoded = 13 + global_bit_pixel * 3;
		color_table = malloc(global_bit_pixel * 3);
		memcpy(color_table,  addr + 13, global_bit_pixel * 3);
		int i;
		printf("Global color table:\n");
		for(i = 0; i <= global_bit_pixel; i ++)
			printf("%2x-%2x-%2x\n",*(color_table + i*3) & 255, *(color_table + 1 + i*3) & 255, *(color_table + 2 + i*3) & 255);
	}  
	//gif_get_next_step 
	for(;;) {
		printf("Get %x\n",*(addr + last_decoded));
		switch ((*(addr + last_decoded))&255) {
			case ';': printf("Image decoding done.\n"); return 0;
			case ',': {
				//gif_get_frame_info
				last_decoded ++;
				int x = 0, y=0,frame_width=0, frame_height=0;
				memcpy(&x, addr + last_decoded, 2);
				memcpy(&y, addr + last_decoded + 2, 2);
				memcpy(&frame_width, addr + last_decoded+4, 2);
				memcpy(&frame_height, addr + last_decoded+6, 2);
				printf("frame position %d*%d on %x*%x\n",frame_width,frame_height,x,y);
				printf("Local color table: %x\n",*(addr + last_decoded + 8));
				//not done
				//context->frame_interlace = BitSet (buf[8], INTERLACE);
				last_decoded = last_decoded + 9;
				size_t lzw_bits = *(addr + last_decoded);
				//realsize more to one bit!
				lzw_bits ++;
				printf("Bits in lzw data: %ld\n", lzw_bits);
				last_decoded ++;
				int lzw_blocksize = *(addr + last_decoded);
				last_decoded ++;
				char* lzw = NULL;
				size_t lzw_size = 0;
				size_t minimal_allign = sizeof(size_t)/sizeof(char);
				while(lzw_blocksize != 0) {
					printf("lzw code size: %d\n",lzw_blocksize);
					//from last_decoded - to last_decoded + lzw_blocksize must be lzw
					//not alligned by sizeof(size_t) but allways alloc more than minimal_allign*n 
					size_t new_size = lzw_size + lzw_blocksize + minimal_allign;
					
					lzw = realloc(lzw, new_size);
					memcpy(lzw + lzw_size, addr + last_decoded, lzw_blocksize);
					lzw_size += lzw_blocksize;
					last_decoded = last_decoded + lzw_blocksize;
					lzw_blocksize = *(addr + last_decoded);
					last_decoded ++;
				}
				printf("Absolute lzw_size %ld by byte = %ld \n", lzw_size, lzw_bits);

				size_t current_pos_lzw = 0;				
				for(current_pos_lzw = 0; current_pos_lzw < lzw_size; current_pos_lzw ++ ) {
					printf("0x%2x\n",*(lzw  + current_pos_lzw) & 255);
				}

				struct _lwz_table_transalate {
					size_t pos;
					size_t size;
				};

				typedef struct _lwz_table_transalate lwz_table_transalate;
				lwz_table_transalate * transalate_table = NULL;
				size_t transalate_table_size = (1 << lzw_bits);
				transalate_table = calloc(sizeof(lwz_table_transalate), transalate_table_size);
				
				for(current_pos_lzw = 0; current_pos_lzw < (1 << (lzw_bits -1)); current_pos_lzw ++ ) {
					transalate_table[current_pos_lzw].pos = current_pos_lzw;
					transalate_table[current_pos_lzw].size = 1;
				}
				
				size_t clear = (1 << (lzw_bits -1));
				size_t end = (1 << (lzw_bits -1)) + 1;
				size_t last_pos_index = end + 1;
				printf("clear: %ld end: %ld\n", clear, end);

				size_t current_bit = 0;
				size_t count_bits = lzw_bits;

				char stack[100];
				size_t stack_pos = 0;
				
				size_t last_index = clear;
				
				size_t code = 0;
				/*
       0: {0}
(010)  2: {2}     +6: {0,2}
(001)  1: {1}     +7: {2,1}
(0110) 6: {0,2}   +8: {1,0}
(1000) 8: {1,0}   +9: {0,2,1}
(0001) 1: {1}     +10:{1,0,1}
(1010) 10:{1,0,1} +11:{1,1}
(0010) 2: {2}     +12:{1,0,1,2}
(0000) 0: {0}     +13:{2,0}
(0001) 1: {1}     +14:{0,1}
(1101) 13:{2,0}   +15:{1,2}
				 */
				for(current_bit = 0 ; current_bit/8 < lzw_size && code != end; current_bit += count_bits) {	
					memcpy(&code, lzw + (current_bit/8), sizeof(size_t));
					code = code >> (current_bit%8);
					code = (code & ((1 << count_bits) - 1));
					printf(" %ld => %ld\n",code, transalate_table[code].size);
					if (code == end) {
						printf("It's all;");
					} else if (code == clear) {
						printf("clear bitsize\n");
						count_bits = lzw_bits;
					} else {
						char result = 0;
						if (transalate_table[code].size == 1) {
							result = transalate_table[code].pos;
							stack[stack_pos] = result;
							stack_pos ++;
						} else {
							int some_index = 0;
							for(;some_index < transalate_table[code].size - 1; some_index ++) {
								stack[stack_pos] = stack[transalate_table[code].pos + some_index];
								stack_pos ++;	
							}
						}
						int some_index = 0;
						printf("Stack :");
						for(;some_index < stack_pos; some_index ++)
							printf("%d, ",stack[some_index] & 255);
						printf("\n");
							
						if (last_index != clear) { // only first time < 0 - than 1 and more chars length
							transalate_table[last_pos_index].pos = stack_pos - transalate_table[code].size - 1;
							printf("stack_p %ld last_index %ld\n", stack_pos, last_index);
							transalate_table[last_pos_index].size = transalate_table[code].size + 1;
								
							printf("table: ");
							printf("%ld pos: %ld size: %ld =>", last_pos_index, transalate_table[last_pos_index].pos, transalate_table[last_pos_index].size);
							int some_index = 0;
							for(;some_index < transalate_table[last_pos_index].size; some_index ++) {
								if (transalate_table[last_pos_index].size == 1)
									printf("%ld, ",transalate_table[last_pos_index].pos & 255);
								else 
									printf("%d, ",stack[transalate_table[last_pos_index].pos + some_index] & 255);
							}
							if (last_pos_index == 8)
								count_bits = 4;
							if (last_pos_index == 16)
								count_bits = 5;
							printf("!%ld\n", count_bits);
							last_pos_index ++;
							last_index = code;

						} else { 
							last_index = code;
						}
					}
				}				
				//start 
				break;
			};
			case '!': {
				printf("I must start decode extension.\n");
				//gif_get_extension
				last_decoded ++;
				switch(*(addr + last_decoded)) {
					case 0xf9: {			/* Graphic Control Extension */
						printf("Transaprent inforamation\n");
						last_decoded ++;
						printf("count bytes in this block %x\n",*(addr + last_decoded));
						printf("disposal: %d\n", (*(addr + last_decoded + 1) >> 2) & 0x7);
						printf("input_flag: %d\n", (*(addr + last_decoded + 1) >> 1) & 0x1);
						int delay_time;
						memcpy(&delay_time, (addr + last_decoded + 2), 2);
						printf("delay: %d\n", delay_time);
						int transparent;				
						if ((*(addr + last_decoded) & 0x1) != 0) {
							transparent = *(addr + last_decoded + 4);
						} else {
							transparent = -1;
						}
						printf("transparent: %d\n",transparent);
						last_decoded = last_decoded + *(addr + last_decoded) + 1; //size block + 1 for byte size block
						if(*(addr + last_decoded) != 0)
							handle_error("block must be done.\n");
						else 
							last_decoded ++;
						break;
					}
					case 0xff: /* application extension */ {
						handle_error("we not not can decode this extension.\n"); 
					}
					default:
						handle_error("stoped.\n");
				}
				break;
			};
			default: {
				handle_error("wrong data.");
			};
		}
	}
	return 0;
};
