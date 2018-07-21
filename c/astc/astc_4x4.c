// gcc astc_4x4.c -o astc_check && ./astc_check
#include <stdio.h>
#include <string.h>

// mesa
#define uint8_t unsigned char
#define bool char
#define uint32_t int
#define uint16_t short
#define false 0
#define true 1
#define MIN2( A, B )   ( (A)<(B) ? (A) : (B) )

// input
uint8_t in_buf[] = {0x53, 0x88, 0x9, 0xcc, 0xdd, 0x77, 0x79, 0xbf, 0xdf, 0xde, 0xff, 0xde, 0xfd, 0xdf, 0x27, 0x80};
uint8_t out_buf[16 * 4] = {
   0x60,0x73,0x70,0xff, 0x5b,0x6d,0x6b,0xff, 0x5c,0x6e,0x7d,0xff, 0x64,0x78,0x88,0xff,
   0x5b,0x6d,0x6b,0xff, 0x5c,0x6e,0x7d,0xff, 0x64,0x78,0x88,0xff, 0x78,0x8f,0x8d,0xff,
   0x5c,0x6e,0x7d,0xff, 0x64,0x78,0x88,0xff, 0x78,0x8f,0x8d,0xff, 0x7a,0x92,0xa5,0xff,
   0x64,0x78,0x88,0xff, 0x78,0x8f,0x8d,0xff, 0x7a,0x92,0xa5,0xff, 0x76,0x8d,0x9f,0xff};
uint8_t my_res[16 * 4] = {0};

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

// code from format description
static uint32_t
hash52(uint32_t p)
{
   p ^= p >> 15;
   p -= p << 17;
   p += p << 7;
   p += p << 4;
   p ^= p >> 5;
   p += p << 16;
   p ^= p >> 7;
   p ^= p >> 3;
   p ^= p << 6;
   p ^= p >> 17;
   return p;
}

static int
select_partition(int seed, int x, int y, int z,
                 int partitioncount, int small_block)
{
   if (small_block) {
      x <<= 1;
      y <<= 1;
      z <<= 1;
   }

   seed += (partitioncount-1) * 1024;
   uint32_t rnum = hash52(seed);
   uint8_t seed1 = rnum & 0xF;
   uint8_t seed2 = (rnum >>  4) & 0xF;
   uint8_t seed3 = (rnum >>  8) & 0xF;
   uint8_t seed4 = (rnum >> 12) & 0xF;
   uint8_t seed5 = (rnum >> 16) & 0xF;
   uint8_t seed6 = (rnum >> 20) & 0xF;
   uint8_t seed7 = (rnum >> 24) & 0xF;
   uint8_t seed8 = (rnum >> 28) & 0xF;
   uint8_t seed9 = (rnum >> 18) & 0xF;
   uint8_t seed10 = (rnum >> 22) & 0xF;
   uint8_t seed11 = (rnum >> 26) & 0xF;
   uint8_t seed12 = ((rnum >> 30) | (rnum << 2)) & 0xF;

   seed1 *= seed1;
   seed2 *= seed2;
   seed3 *= seed3;
   seed4 *= seed4;
   seed5 *= seed5;
   seed6 *= seed6;
   seed7 *= seed7;
   seed8 *= seed8;
   seed9 *= seed9;
   seed10 *= seed10;
   seed11 *= seed11;
   seed12 *= seed12;

   int sh1, sh2, sh3;
   if (seed & 1) {
      sh1 = (seed&2 ? 4:5);
      sh2 = (partitioncount==3 ? 6:5);
   } else {
      sh1 = (partitioncount==3 ? 6:5);
      sh2 = (seed&2 ? 4:5);
   }
   sh3 = (seed & 0x10) ? sh1 : sh2;

   seed1 >>= sh1;
   seed2 >>= sh2;
   seed3 >>= sh1;
   seed4 >>= sh2;
   seed5 >>= sh1;
   seed6 >>= sh2;
   seed7 >>= sh1;
   seed8 >>= sh2;
   seed9 >>= sh3;
   seed10 >>= sh3;
   seed11 >>= sh3;
   seed12 >>= sh3;

   int a = seed1*x + seed2*y + seed11*z + (rnum >> 14);
   int b = seed3*x + seed4*y + seed12*z + (rnum >> 10);
   int c = seed5*x + seed6*y + seed9 *z + (rnum >>  6);
   int d = seed7*x + seed8*y + seed10*z + (rnum >>  2);

   a &= 0x3F; b &= 0x3F; c &= 0x3F; d &= 0x3F;

   if (partitioncount < 4)
      d = 0;
   if (partitioncount < 3)
      c = 0;

   if (a >= b && a >= c && a >= d) {
      return 0;
   } else if( b >= c && b >= d ) {
      return 1;
   } else if( c >= d ) {
      return 2;
   } else {
      return 3;
   }
}

static void
bit_transfer_signed(uint16_t* a, uint16_t* b)
{
    *b >>= 1;
    *b |= *a & 0x80;
    *a >>= 1;
    *a &= 0x3F;
    if( (*a&0x20)!=0 ) *a-=0x40;
}

/*
color blue_contract(int r, int g, int b, int a)
{
    color c;
    c.r = (r+b) >> 1;
    c.g = (g+b) >> 1;
    c.b = b;
    c.a = a;
    return c;
}

void clamp_unorm8(color c)
{
    if(c.r < 0) {c.r=0;} else if(c.r > 255) {c.r=255;}
    if(c.g < 0) {c.g=0;} else if(c.g > 255) {c.g=255;}
    if(c.b < 0) {c.b=0;} else if(c.b > 255) {c.b=255;}
    if(c.a < 0) {c.a=0;} else if(c.a > 255) {c.a=255;}
}
*/

// from TC
struct TexelWeightParams {
   uint32_t width;
   uint32_t height;
   bool dual_plane;
   uint32_t max_weight;
   bool void_extentLDR;
   bool void_extentHDR;
};

uint32_t get_num_weight_values(struct TexelWeightParams* params) {
   uint32_t ret = params->width * params->height;
   if(params->dual_plane) {
      ret *= 2;
   }
   return ret;
}

enum EIntegerEncoding {
    IntegerEncoding_Bits,
    IntegerEncoding_Quint,
    IntegerEncoding_Trit
};

struct IntegerEncodedValue {
    enum EIntegerEncoding m_Encoding;
    uint32_t m_NumBits;
    uint32_t m_BitValue;
    uint32_t m_QuintValue;
    uint32_t m_TritValue;
};

// Count the number of bits set in a number.
static uint32_t Popcnt(uint32_t n) {
   uint32_t c;
   for(c = 0; n; c++) {
      n &= n-1;
   }
   return c;
}

static void CreateEncoding(uint32_t maxVal, struct IntegerEncodedValue* value) {
    while(maxVal > 0) {
      uint32_t check = maxVal + 1;

      // Is maxVal a power of two?
      if(!(check & (check - 1))) {
	  value->m_Encoding = IntegerEncoding_Bits;
	  value->m_NumBits = Popcnt(maxVal);
	  return;
      }

      // Is maxVal of the type 3*2^n - 1?
      if((check % 3 == 0) && !((check/3) & ((check/3) - 1))) {
	  value->m_Encoding = IntegerEncoding_Trit;
	  value->m_NumBits = Popcnt(check/3 - 1);
	  return;
      }

      // Is maxVal of the type 5*2^n - 1?
      if((check % 5 == 0) && !((check/5) & ((check/5) - 1))) {
	  value->m_Encoding = IntegerEncoding_Quint;
	  value->m_NumBits = Popcnt(check/5 - 1);
	  return;
      }

      // Apparently it can't be represented with a bounded integer sequence...
      // just iterate.
      maxVal--;
    }
    value->m_Encoding = IntegerEncoding_Bits;
    value->m_NumBits = 0;
}

uint32_t GetBitLength(uint32_t nVals, struct IntegerEncodedValue* value) {
    uint32_t totalBits = value->m_NumBits * nVals;
    if(value->m_Encoding == IntegerEncoding_Trit) {
      totalBits += (nVals * 8 + 4) / 5;
    } else if(value->m_Encoding == IntegerEncoding_Quint) {
      totalBits += (nVals * 7 + 2) / 3;
    }
    return totalBits;
}

uint32_t GetPackedBitSize(struct TexelWeightParams* params) {
  uint32_t check = 0;

  // How many indices do we have?
  uint32_t nIdxs = get_num_weight_values(params);

  struct IntegerEncodedValue value;
  CreateEncoding(params->max_weight, &value);

  check = GetBitLength(nIdxs, &value);
  printf("\nMaxWeight %d nIdxs %d check %d\n", params->max_weight, nIdxs, check);
  return check;
}

static bool
DecodeBlockInfo(uint8_t* block, struct TexelWeightParams* params) {
    // Read the entire block mode all at once
    uint32_t modeBits = extract_bits(block, 0, 11);

    printf("\nMode: 0x%x\n", modeBits & 0xffff);

    // Does this match the void extent block mode?
    if((modeBits & 0x01FF) == 0x1FC) {
      if (modeBits & 0x200) {
        params->void_extentLDR = false;
        params->void_extentHDR = true;
      } else {
        params->void_extentLDR = true;
        params->void_extentHDR = false;
      }

      // Next two bits must be one.
      if (!(modeBits & 0x400) || !extract_bits(block, 11, 1)) {
          return true;
      }

      return false;
    }

    // First check if the last four bits are zero
    if((modeBits & 0xF) == 0) {
        return true;
    }

    // If the last two bits are zero, then if bits
    // [6-8] are all ones, this is also reserved.
    if((modeBits & 0x3) == 0 &&
       (modeBits & 0x1C0) == 0x1C0) {
           return true;
    }

    // Otherwise, there is no error... Figure out the layout
    // of the block mode. Layout is determined by a number
    // between 0 and 9 corresponding to table C.2.8 of the
    // ASTC spec.
    uint32_t layout = 0;

    if((modeBits & 0x1) || (modeBits & 0x2)) {
      // layout is in [0-4]
      if(modeBits & 0x8) {
        // layout is in [2-4]
        if(modeBits & 0x4) {
          // layout is in [3-4]
          if(modeBits & 0x100) {
            layout = 4;
          } else {
            layout = 3;
          }
        } else {
          layout = 2;
        }
      } else {
        // layout is in [0-1]
        if(modeBits & 0x4) {
          layout = 1;
        } else {
          layout = 0;
        }
      }
    } else {
      // layout is in [5-9]
      if(modeBits & 0x100) {
        // layout is in [7-9]
        if(modeBits & 0x80) {
          // layout is in [7-8]
          if(modeBits & 0x20) {
            layout = 8;
          } else {
            layout = 7;
          }
        } else {
          layout = 9;
        }
      } else {
        // layout is in [5-6]
        if(modeBits & 0x80) {
          layout = 6;
        } else {
          layout = 5;
        }
      }
    }

    // Determine R
    uint32_t R = !!(modeBits & 0x10);
    if(layout < 5) {
      R |= (modeBits & 0x3) << 1;
    } else {
      R |= (modeBits & 0xC) >> 1;
    }

    // Determine width & height
    switch(layout) {
      case 0: {
        uint32_t A = (modeBits >> 5) & 0x3;
        uint32_t B = (modeBits >> 7) & 0x3;
        params->width = B + 4;
        params->height = A + 2;
        break;
      }

      case 1: {
        uint32_t A = (modeBits >> 5) & 0x3;
        uint32_t B = (modeBits >> 7) & 0x3;
        params->width = B + 8;
        params->height = A + 2;
        break;
      }

      case 2: {
        uint32_t A = (modeBits >> 5) & 0x3;
        uint32_t B = (modeBits >> 7) & 0x3;
        params->width = A + 2;
        params->height = B + 8;
        break;
      }

      case 3: {
        uint32_t A = (modeBits >> 5) & 0x3;
        uint32_t B = (modeBits >> 7) & 0x1;
        params->width = A + 2;
        params->height = B + 6;
        break;
      }

      case 4: {
        uint32_t A = (modeBits >> 5) & 0x3;
        uint32_t B = (modeBits >> 7) & 0x1;
        params->width = B + 2;
        params->height = A + 2;
        break;
      }

      case 5: {
        uint32_t A = (modeBits >> 5) & 0x3;
        params->width = 12;
        params->height = A + 2;
        break;
      }

      case 6: {
        uint32_t A = (modeBits >> 5) & 0x3;
        params->width = A + 2;
        params->height = 12;
        break;
      }

      case 7: {
        params->width = 6;
        params->height = 10;
        break;
      }

      case 8: {
        params->width = 10;
        params->height = 6;
        break;
      }

      case 9: {
        uint32_t A = (modeBits >> 5) & 0x3;
        uint32_t B = (modeBits >> 9) & 0x3;
        params->width = A + 6;
        params->height = B + 6;
        break;
      }

      default:
        return true;
    }

    // Determine whether or not we're using dual planes
    // and/or high precision layouts.
    bool D = (layout != 9) && (modeBits & 0x400);
    bool H = (layout != 9) && (modeBits & 0x200);

    if(H) {
      const uint32_t maxWeights[6] = { 9, 11, 15, 19, 23, 31 };
      params->max_weight = maxWeights[R-2];
    } else {
      const uint32_t maxWeights[6] = { 1, 2, 3, 4, 5, 7 };
      params->max_weight = maxWeights[R-2];
    }

    params->dual_plane = D;

    return false;
}

static void
FillError(uint8_t* my_res, int height, int width)
{

    for(int j = 0; j < height; j++) {
        for(int i = 0; i < width; i++) {
                my_res[(j * width + i) * 4 + 0] = 0xFF;
                my_res[(j * width + i) * 4 + 1] = 0xFF;
                my_res[(j * width + i) * 4 + 2] = 0x00;
                my_res[(j * width + i) * 4 + 3] = 0xFF;
        }
    }
}

static void
FillVoidExtentLDR(uint8_t* block, uint8_t* my_res, int height, int width){

    // Decode the RGBA components and renormalize them to the range [0, 255]
    uint32_t r = extract_bits(block, 64, 16);
    uint32_t g = extract_bits(block, 80, 16);
    uint32_t b = extract_bits(block, 96, 16);
    uint32_t a = extract_bits(block, 112, 16);

    uint32_t rgba = (r >> 8) | (g & 0xFF00) | (b & 0xFF00) << 8
      | (a & 0xFF00) << 16;

    for (uint32_t j = 0; j < height; j++)
    for (uint32_t i = 0; i < width; i++) {
      my_res[(j * width + i) * 4 + 0] = extract_bits((uint8_t*)&rgba, 0, 8);
      my_res[(j * width + i) * 4 + 1] = extract_bits((uint8_t*)&rgba, 8, 8);
      my_res[(j * width + i) * 4 + 2] = extract_bits((uint8_t*)&rgba, 16, 8);
      my_res[(j * width + i) * 4 + 3] = extract_bits((uint8_t*)&rgba, 24, 8);
    }
}

static void
dump(uint8_t* data, uint32_t size) {
       uint32_t x;
       printf("\n");
       for(x=0; x<size; x++) {
	  printf("0x%x,", data[x]&0xFF);
	if (x%4 == 3) {
	    printf(" ");
	}
	if (x%16 == 15) {
	    printf("\n");
	}
       }
       printf("\n");
}

void DecodeColorValues(uint32_t *out, uint8_t *data, uint32_t *modes,
                         const uint32_t nPartitions, const uint32_t nBitsForColorData) {
    // First figure out how many color values we have
    uint32_t nValues = 0;
    for(uint32_t i = 0; i < nPartitions; i++) {
      nValues += ((modes[i]>>2) + 1) << 1;
    }
    printf("\nnValues=%d\n", nValues);
}

static void
DecompressBlock(uint8_t *block, uint8_t* my_res, int height, int width) {
   struct TexelWeightParams weight = {0};

   if (DecodeBlockInfo(block, &weight)) {
       FillError(my_res, height, width);
       return;
   }

   printf("width %d, height %d, dual_plane %d, max_weight %d, void_extentLDR %d, void_extentHDR %d\n",
          weight.width, weight.height, weight.dual_plane,
          weight.max_weight, weight.void_extentLDR, weight.void_extentHDR);


   if (weight.void_extentLDR) {
     FillVoidExtentLDR(block, my_res, width, height);
     return;
   }

   if (weight.void_extentHDR) {
     printf("HDR void extent blocks are unsupported!");
     FillError(my_res, height, width);
     return;
   }

    if(weight.width > width) {
      printf("Texel weight grid width should be smaller than block width");
      FillError(my_res, width, height);
      return;
    }

    if(weight.height > height) {
      printf("Texel weight grid height should be smaller than block height");
      FillError(my_res, height, width);
      return;
    }

    // Read num partitions
    uint32_t nPartitions = extract_bits(block, 11, 2) + 1;

    if(nPartitions == 4 && weight.dual_plane) {
      printf("Dual plane mode is incompatible with four partition blocks");
      FillError(my_res, height, width);
      return;
    }

    // Based on the number of partitions, read the color endpoint mode for
    // each partition.

    // Determine partitions, partition index, and color endpoint modes
    int planeIdx = -1;
    uint32_t partitionIndex;
    uint32_t colorEndpointMode[4] = {0, 0, 0, 0};

    // Define color data.
    uint8_t colorEndpointData[16];
    memset(colorEndpointData, 0, sizeof(colorEndpointData));

    // Read extra config data...
    uint32_t baseCEM = 0;
    uint32_t remainingBits = 128;
    uint32_t readBits = 0;
    if(nPartitions == 1) {
      colorEndpointMode[0] = extract_bits(block, 13, 4);
      partitionIndex = 0;
      readBits = 17;
    } else {
      partitionIndex = extract_bits(block, 13, 10);
      baseCEM = extract_bits(block, 23, 6);
      readBits = 29;
    }

    uint32_t baseMode = (baseCEM & 3);
    remainingBits -= readBits;

    printf("nPartitions=0x%x, baseMode=0x%x, baseCEM=0x%x, partitionIndex=0x%x, colorEndpointMode=0x%x \n ",
           nPartitions, baseMode, baseCEM, partitionIndex, colorEndpointMode[0]);

    // Remaining bits are color endpoint data...
    uint32_t nWeightBits = GetPackedBitSize(&weight);
    remainingBits -= nWeightBits;
    printf("\nremainingBits=%d, nWeightBits=%d\n", remainingBits, nWeightBits);

    // Consider extra bits prior to texel data...
    uint32_t extraCEMbits = 0;
    if(baseMode) {
      switch(nPartitions) {
      case 2: extraCEMbits += 2; break;
      case 3: extraCEMbits += 5; break;
      case 4: extraCEMbits += 8; break;
      default: /*wrong partitions bits*/ break;
      }
    }
    remainingBits -= extraCEMbits;

    // Do we have a dual plane situation?
    uint32_t planeSelectorBits = 0;
    if(weight.dual_plane) {
      planeSelectorBits = 2;
    }
    remainingBits -= planeSelectorBits;

    // Read color data...
    uint32_t colorDataBits = remainingBits;
    uint32_t out_pos = 0;
    while(remainingBits > 0) {
      uint32_t nb = MIN2(remainingBits, 8);
      uint32_t b = extract_bits(block, readBits, nb);
      colorEndpointData[out_pos] = b & 0xFF;
      out_pos++;
      readBits += nb;
      remainingBits -= nb;
    }

    dump(colorEndpointData, 16);
    // Read the plane selection bits
    planeIdx = extract_bits(block, readBits, planeSelectorBits);
    readBits += planeSelectorBits;

    // Read the rest of the CEM
    if(baseMode) {
      uint32_t extraCEM = extract_bits(block, readBits, extraCEMbits);
      readBits += extraCEMbits;

      uint32_t CEM = (extraCEM << 6) | baseCEM;
      CEM >>= 2;

      bool C[4] = { 0 };
      for(uint32_t i = 0; i < nPartitions; i++) {
        C[i] = CEM & 1;
        CEM >>= 1;
      }

      uint8_t M[4] = { 0 };
      for(uint32_t i = 0; i < nPartitions; i++) {
        M[i] = CEM & 3;
        CEM >>= 2;
	// assert M[i] <= 3;
      }

      for(uint32_t i = 0; i < nPartitions; i++) {
        colorEndpointMode[i] = baseMode;
        if(!(C[i])) colorEndpointMode[i] -= 1;
        colorEndpointMode[i] <<= 2;
        colorEndpointMode[i] |= M[i];
      }
    } else if(nPartitions > 1) {
      uint32_t CEM = baseCEM >> 2;
      for(uint32_t i = 0; i < nPartitions; i++) {
        colorEndpointMode[i] = CEM;
      }
    }

    dump((uint8_t*) colorEndpointMode, 16);
    // Make sure everything up till here is sane.
    // Decode both color data and texel weight data
    uint32_t colorValues[32] = {}; // Four values, two endpoints, four maximum paritions
    DecodeColorValues(colorValues, colorEndpointData, colorEndpointMode,
                      nPartitions, colorDataBits);
    printf("\ncolorValues\n");
    dump((uint8_t*) colorValues, 32 * 4);

    FillError(my_res, height, width);
}

//checks
int height = 4;
int width = 4;
int main() {
   int x, y;

   printf("Initial:");
   dump(in_buf, 16);

   DecompressBlock(in_buf, my_res, height, width);

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
