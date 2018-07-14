// gcc astc_4x4.c -o astc_check && ./astc_check
#include <stdio.h>
#include <string.h>

// mesa
#define uint8_t char
#define true 1
#define MIN2( A, B )   ( (A)<(B) ? (A) : (B) )

// input
char in_buf[] = {0x53, 0x88, 0x9, 0xcc, 0xdd, 0x77, 0x79, 0xbf, 0xdf, 0xde, 0xff, 0xde, 0xfd, 0xdf, 0x27, 0x80};
char out_buf[16 * 4] = {
   0x60,0x73,0x70,0xff, 0x5b,0x6d,0x6b,0xff, 0x5c,0x6e,0x7d,0xff, 0x64,0x78,0x88,0xff,
   0x5b,0x6d,0x6b,0xff, 0x5c,0x6e,0x7d,0xff, 0x64,0x78,0x88,0xff, 0x78,0x8f,0x8d,0xff,
   0x5c,0x6e,0x7d,0xff, 0x64,0x78,0x88,0xff, 0x78,0x8f,0x8d,0xff, 0x7a,0x92,0xa5,0xff,
   0x64,0x78,0x88,0xff, 0x78,0x8f,0x8d,0xff, 0x7a,0x92,0xa5,0xff, 0x76,0x8d,0x9f,0xff};
char my_res[16 * 4] = {0};

// copy of bptc
static int
extract_bits(const uint8_t *block,
             int offset,
             int n_bits)
{
   int byte_index = offset / 8;
   int bit_index = offset % 8;
   int n_bits_in_byte = MIN2(n_bits, 8 - bit_index);
   int result = 0;
   int bit = 0;

   while (true) {
      result |= ((block[byte_index] >> bit_index) &
                 ((1 << n_bits_in_byte) - 1)) << bit;

      n_bits -= n_bits_in_byte;

      if (n_bits <= 0)
         return result;

      bit += n_bits_in_byte;
      byte_index++;
      bit_index = 0;
      n_bits_in_byte = MIN2(n_bits, 8);
   }
}

int height = 4;
int width = 4;
int main() {
   int x, y;

   printf("Initial: \n");
   for(x=0; x<16; x++) {
      printf("0x%x ", in_buf[x]&0xFF);
   }

   printf("\nBlock: 0x%x\n", extract_bits(in_buf, 0, 11));
   printf("\nResult:\n");
   for(y=0; y<height; y++) {
      for(x=0; x<(width * 4); x++) {
         printf("0x%x,", ((uint8_t*)my_res)[y*width + x] & 0xFF);
         if (x%4 == 3) {
            printf(" ");
         }
      }
      printf("\n");
   }
   printf("\n");
}
