#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vcop.h>
#include <math.h>

int main(void);

#include "vcop_pd_hog_vector_kernel.h"

#define FRM_WIDTH   (256/4)
#define FRM_HEIGHT  (488/4)
#define BLK_WIDTH   (32)
#define BLK_HEIGHT  (32)



#define WIN_SLIDE_STEP_SIZE               (4) // This information is not required for this kernels. this was used in the prevoius stage
#define HOG_CELL_SIZE_IN_WIN_SLIDE_STEPS  (2) // 2*4 -> 8
#define HOG_BLK_SIZE_IN_NUM_CELLS         (1) // 1*8 -> 8
#define HOG_BLK_STEPS_IN_WIN_SLIDE_STEPS  (2) // 2*4 -> 8
#define NUM_HOR_HOG_BLKS                  (8)
#define NUM_VER_HOG_BLKS                 (16)

void memCopy2D
(
unsigned char * srcAddr,
unsigned char * dstAddr,
unsigned short width,
unsigned short height,
short srcLineOffset,
short dstLineOffset
)
{
	int i;
	for(i=0;i<height;i++)
		memcpy(dstAddr + i*dstLineOffset, srcAddr + i*srcLineOffset, width);
}

int hog_computation(unsigned short * inPtr,unsigned short * refPtr,unsigned short pitch)
{

  int i,j,k,l,m,n;
  unsigned short * ptr;
  unsigned short sum;
  int NUM_HOG_IN_BLK                    = (HOG_BLK_SIZE_IN_NUM_CELLS*HOG_BLK_SIZE_IN_NUM_CELLS);

  for(i = 0; i < NUM_VER_HOG_BLKS; i++)
  {
    for(j = 0; j < NUM_HOR_HOG_BLKS; j++)
    {
      for(k = 0; k < HOG_BLK_SIZE_IN_NUM_CELLS; k++)
      {
        for(l = 0; l < HOG_BLK_SIZE_IN_NUM_CELLS; l++)
        {
          sum = 0;
          for(m = 0; m < HOG_CELL_SIZE_IN_WIN_SLIDE_STEPS; m++)
          {
            for(n = 0; n < HOG_CELL_SIZE_IN_WIN_SLIDE_STEPS; n++)
            {
              ptr = inPtr + i*HOG_BLK_STEPS_IN_WIN_SLIDE_STEPS*pitch+j*HOG_BLK_STEPS_IN_WIN_SLIDE_STEPS + k*HOG_CELL_SIZE_IN_WIN_SLIDE_STEPS*pitch+l*HOG_CELL_SIZE_IN_WIN_SLIDE_STEPS + m*pitch+n;
              sum += *ptr;
            }
          }
          refPtr[((i*NUM_HOR_HOG_BLKS + j)*NUM_HOG_IN_BLK) + k*HOG_BLK_SIZE_IN_NUM_CELLS + l] = sum;
        }
      }
    }
  }
  return 0;
}

void sum_2x2(unsigned short * inPtr,unsigned short * outPtr,unsigned short width, unsigned short height ,unsigned short inPitch)
{
  int i,j;
  for(i = 0; i < height; i++)
  {
    for(j = 0; j < width; j++)
    {
      outPtr[i*width+ j] = inPtr[i*inPitch+ j] + inPtr[i*inPitch+ j+1]  + inPtr[(i+1)*inPitch+ j]  + inPtr[(i+1)*inPitch+ j+1];  
    }
  }
}

int main()
{
  int fail,i,j,k,l,m;

  unsigned short frmWidth   =  FRM_WIDTH;
  unsigned short frmheight  =  FRM_HEIGHT;
  int NUM_HOG_IN_WIN                    = (NUM_VER_HOG_BLKS*HOG_BLK_SIZE_IN_NUM_CELLS * NUM_HOR_HOG_BLKS*HOG_BLK_SIZE_IN_NUM_CELLS);
  int NUM_HOG_IN_BLK                    = (HOG_BLK_SIZE_IN_NUM_CELLS*HOG_BLK_SIZE_IN_NUM_CELLS);
  int WIN_SIZE_HOR_IN_WIN_SLIDE_STEPS   = ((((NUM_HOR_HOG_BLKS-1)*HOG_BLK_STEPS_IN_WIN_SLIDE_STEPS)) + (HOG_BLK_SIZE_IN_NUM_CELLS*HOG_CELL_SIZE_IN_WIN_SLIDE_STEPS));
  int WIN_SIZE_VER_IN_WIN_SLIDE_STEPS   = ((((NUM_VER_HOG_BLKS-1)*HOG_BLK_STEPS_IN_WIN_SLIDE_STEPS)) + (HOG_BLK_SIZE_IN_NUM_CELLS*HOG_CELL_SIZE_IN_WIN_SLIDE_STEPS));

  unsigned short * inPtr;
  unsigned short * outPtr;
  unsigned short * refPtr;
  unsigned short numHorWins   =  (frmWidth - WIN_SIZE_HOR_IN_WIN_SLIDE_STEPS)+1;
  unsigned short numVerWins   =  (frmheight - WIN_SIZE_VER_IN_WIN_SLIDE_STEPS)+1;

  unsigned short * buff_a;
  unsigned short * buff_b;
  unsigned short * buff_c;

  inPtr   = (unsigned short *)malloc(frmWidth*frmheight*2);
  outPtr  = (unsigned short *)malloc(numHorWins*numVerWins*NUM_HOG_IN_WIN*2);
  refPtr  = (unsigned short *)malloc(numHorWins*numVerWins*NUM_HOG_IN_WIN*2);

#if VCOP_HOST_EMULATION
  buff_a  = (unsigned short *)malloc(1024*16);
  buff_b  = (unsigned short *)malloc(1024*16);
  buff_c  = (unsigned short *)_aligned_malloc(1024*32, 1024*4);
#else
  buff_a  = (unsigned short *)(0x40050000);
  buff_b  = (unsigned short *)(0x40054000);
  buff_c  = (unsigned short *)(0x40040000);
#endif

  srand(4);
  for(i = 0; i < frmWidth*frmheight; i++)
  {
    inPtr[i] =(((unsigned short)rand()) & 0xFFF);
  }


  for(i = 0; i < (numHorWins/16) ; i++)
  {
    for(j = 0; j < (numVerWins/2)  ; j++)
    {
      if(j ==0)
      {
        memCopy2D((unsigned char *)(inPtr+16*i),(unsigned char *)buff_a,33*2,33,frmWidth*2,34*2);
#if 0
        sum_2x2(buff_a,buff_c,32,32,33);
#else
        vcop_ti_pd_2x2_sum(buff_a,buff_c,32,32,34);
#endif

      }
      memCopy2D((unsigned char *)(inPtr+16*i + 32*frmWidth + j*2*frmWidth),(unsigned char *)buff_a,33*2,3,frmWidth*2,34*2);

#if 0
      sum_2x2(buff_a,(buff_c + ((32*32*2 + j*32*2*2)%(1024*4))/2),32,2,33);
#else
      vcop_ti_pd_2x2_sum(buff_a,(buff_c + ((32*32*2 + j*32*2*2)%(1024*4))/2),32,2,34);
#endif     
      vcop_ti_pd_hog_computation(buff_c, ((j*32*2*2)%(1024*4)), (((j+1)*32*2*2)%(1024*4)),(unsigned short *)buff_b,(unsigned int *)buff_b,(unsigned int *)buff_a, 32, 16,2);
      
      for(k = 0; k < 2; k++)
      {
        for(l = 0; l < (16/8); l++)
        {
          for(m = 0; m < 8; m++)
          {
            memCopy2D((((unsigned char *)buff_a) + k*(16/8)*(NUM_HOG_IN_WIN/2)*32 + l*(NUM_HOG_IN_WIN/2)*32 + m*4),(((unsigned char *)outPtr)+ 2*(NUM_HOG_IN_WIN*(j*2*numHorWins+i*16)) + k*NUM_HOG_IN_WIN*numHorWins*2 + l*8*NUM_HOG_IN_WIN*2 + m*NUM_HOG_IN_WIN*2),4,NUM_HOG_IN_WIN/2,32,4);
          }
        }
      }
    }
  }

  for(i = 0; i < numVerWins; i++)
  {
    for(j = 0; j < numHorWins; j++)
    {
      hog_computation((inPtr+frmWidth*i + j),(refPtr+ (NUM_HOG_IN_WIN*(i*numHorWins+j))),frmWidth);
    }
  }

  for(i = 0; i < numVerWins-1; i++)
  {
    for(j = 0; j < numHorWins-1; j++)
    {
      if(memcmp((unsigned char *)(refPtr+ (NUM_HOG_IN_WIN*(i*numHorWins+j))),(unsigned char *)(outPtr+ (NUM_HOG_IN_WIN*(i*numHorWins+j))),128*2))
      {
        printf("Failing at %4d %4d \n",i,j);
      }
    }
  }
  printf("Test completed \n");

  return 0;
}

