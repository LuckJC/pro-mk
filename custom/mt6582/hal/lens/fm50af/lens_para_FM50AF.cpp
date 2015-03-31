#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

//#include "msdk_nvram_camera_exp.h"
//#include "msdk_lens_exp.h"
#include "camera_custom_nvram.h"
#include "camera_custom_lens.h"

const NVRAM_LENS_PARA_STRUCT FM50AF_LENS_PARA_DEFAULT_VALUE =
{
    //Version
    NVRAM_CAMERA_LENS_FILE_VERSION,

    // Focus Range NVRAM
    {0, 1023},

    // AF NVRAM
    {
        // -------- AF ------------
        {50,  // i4Offset
         19,  // i4NormalNum
         20,  // i4MacroNum
         0,  // i4InfIdxOffset
         0,  // i4MacroIdxOffset
        {
                       0,  20,  40,  80, 130, 170, 230, 300, 340, 420,
                     500, 550, 620, 690, 750, 790, 830, 860, 890, 910,
                       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        },
        35,  // i4THRES_MAIN
        25,  // i4THRES_SUB
        4,  // i4INIT_WAIT
        {500, 500, 500, 500, 500},  // i4FRAME_WAIT
        0,  // i4DONE_WAIT
        0,  // i4FAIL_POS
        33,  // i4FRAME_TIME
        5,  // i4FIRST_FV_WAIT
        16, //12,  // i4FV_CHANGE_THRES
        8000,  // i4FV_CHANGE_OFFSET
        6,  // i4FV_CHANGE_CNT
        0,  // i4GS_CHANGE_THRES
        15,  // i4GS_CHANGE_OFFSET
        12,//12,  // i4GS_CHANGE_CNT
        12,  // i4FV_STABLE_THRES
        8000,  // i4FV_STABLE_OFFSET
        15,  // i4FV_STABLE_NUM
        15,  // i4FV_STABLE_CNT
        15,  // i4FV_1ST_STABLE_THRES
        10000,  // i4FV_1ST_STABLE_OFFSET
        15,  // i4FV_1ST_STABLE_NUM
        15  // i4FV_1ST_STABLE_CNT
        },

        //-------- ZSD AF ------------
        {50,  // i4Offset
         19,  // i4NormalNum
         20,  // i4MacroNum
         0,  // i4InfIdxOffset
         0,  // i4MacroIdxOffset
        {
                       0,  20,  40,  80, 130, 170, 230, 300, 340, 420,
                     500, 550, 620, 690, 750, 790, 830, 860, 890, 910,
                       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        },
        35,  // i4THRES_MAIN
        25,  // i4THRES_SUB
        4,  // i4INIT_WAIT
        {500, 500, 500, 500, 500},  // i4FRAME_WAIT
        0,  // i4DONE_WAIT
        0,  // i4FAIL_POS
        33,  // i4FRAME_TIME
        5,  // i4FIRST_FV_WAIT
        16, //12,  // i4FV_CHANGE_THRES
        8000,  // i4FV_CHANGE_OFFSET
        6,  // i4FV_CHANGE_CNT
        0,  // i4GS_CHANGE_THRES
        15,  // i4GS_CHANGE_OFFSET
        12,//12,  // i4GS_CHANGE_CNT
        12,  // i4FV_STABLE_THRES
        8000,  // i4FV_STABLE_OFFSET
        15,  // i4FV_STABLE_NUM
        15,  // i4FV_STABLE_CNT
        15,  // i4FV_1ST_STABLE_THRES
        10000,  // i4FV_1ST_STABLE_OFFSET
        15,  // i4FV_1ST_STABLE_NUM
        15  // i4FV_1ST_STABLE_CNT
        },


        //-------- VAFC ------------
        {50,  // i4Offset
         19,  // i4NormalNum
         20,  // i4MacroNum
         0,  // i4InfIdxOffset
         0,  // i4MacroIdxOffset
        {
                       0,  20,  40,  80, 130, 170, 230, 300, 340, 420,
                     500, 550, 620, 690, 750, 790, 830, 860, 890, 910,
                       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        },
        35,  // i4THRES_MAIN
        25,  // i4THRES_SUB
        4,  // i4INIT_WAIT
        {500, 500, 500, 500, 500},  // i4FRAME_WAIT
        0,  // i4DONE_WAIT
        0,  // i4FAIL_POS
        33,  // i4FRAME_TIME
        5,  // i4FIRST_FV_WAIT
        16, //12,  // i4FV_CHANGE_THRES
        8000,  // i4FV_CHANGE_OFFSET
        6,  // i4FV_CHANGE_CNT
        0,  // i4GS_CHANGE_THRES
        15,  // i4GS_CHANGE_OFFSET
        12,//12,  // i4GS_CHANGE_CNT
        12,  // i4FV_STABLE_THRES
        8000,  // i4FV_STABLE_OFFSET
        15,  // i4FV_STABLE_NUM
        15,  // i4FV_STABLE_CNT
        15,  // i4FV_1ST_STABLE_THRES
        10000,  // i4FV_1ST_STABLE_OFFSET
        15,  // i4FV_1ST_STABLE_NUM
        15  // i4FV_1ST_STABLE_CNT
        },
        // --- sAF_TH ---
        {
          8,   // i4ISONum;
          {100,150,200,300,400,600,800,1600},       // i4ISO[ISO_MAX_NUM];
                  
          6,   // i4GMeanNum;
          {20,55,105,150,180,205},        // i4GMean[GMEAN_MAX_NUM];

          { 89, 89, 89, 88, 87, 87, 87, 86,
           127,127,127,126,126,126,125,125,
           180,180,180,180,180,180,180,179},         // i4GMR[3][ISO_MAX_NUM];
// ------------------------------------------------------------------------                  
           {10000,12000,14000,16000,20000,24000,30000,40000,
            10000,12000,14000,16000,20000,24000,30000,40000,
            10000,12000,14000,16000,20000,24000,30000,40000,
            10000,12000,14000,16000,20000,24000,30000,40000,
            10000,12000,14000,16000,20000,24000,30000,40000,
            10000,12000,14000,16000,20000,24000,30000,40000},      // i4FV_DC[GMEAN_MAX_NUM][ISO_MAX_NUM];
           
           {7000,7000,7000,7000,7000,7000,7000,7000,
            7000,7000,7000,7000,7000,7000,7000,7000,
            7000,7000,7000,7000,7000,7000,7000,7000,
            7000,7000,7000,7000,7000,7000,7000,7000,
            7000,7000,7000,7000,7000,7000,7000,7000,
            7000,7000,7000,7000,7000,7000,7000,7000},         // i4MIN_TH[GMEAN_MAX_NUM][ISO_MAX_NUM];                  

          {4,4,4,4,5,6,8,10,
           4,4,4,4,5,6,8,10,
           4,4,4,4,5,6,8,10,
           4,4,4,4,5,6,8,10,
           4,4,4,4,5,6,8,10,
           4,4,4,4,5,6,8,10}, // i4HW_TH[GMEAN_MAX_NUM][ISO_MAX_NUM];       
// ------------------------------------------------------------------------
          {0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0},        // i4FV_DC2[GMEAN_MAX_NUM][ISO_MAX_NUM];
           
          {0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0},         // i4MIN_TH2[GMEAN_MAX_NUM][ISO_MAX_NUM];
          
          {4,4,4,4,5,6,8,10,
           4,4,4,4,5,6,8,10,
           4,4,4,4,5,6,8,10,
           4,4,4,4,5,6,8,10,
           4,4,4,4,5,6,8,10,
           4,4,4,4,5,6,8,10}          // i4HW_TH2[GMEAN_MAX_NUM][ISO_MAX_NUM];       
          
         },
// ------------------------------------------------------------------------

         // --- sZSDAF_TH ---
          {
           8,   // i4ISONum;
           {100,150,200,300,400,600,800,1600},       // i4ISO[ISO_MAX_NUM];
                   
           6,   // i4GMeanNum;
           {20,55,105,150,180,205},        // i4GMean[GMEAN_MAX_NUM];

           { 87, 87, 87, 86, 85, 85, 84, 83,
            126,126,126,124,123,123,123,122,
            180,180,180,179,178,178,178,178},        // i4GMR[3][ISO_MAX_NUM];
           
// ------------------------------------------------------------------------                   
           {10000,12000,14000,16000,20000,24000,30000,40000,
            10000,12000,14000,16000,20000,24000,30000,40000,
            10000,12000,14000,16000,20000,24000,30000,40000,
            10000,12000,14000,16000,20000,24000,30000,40000,
            10000,12000,14000,16000,20000,24000,30000,40000,
            10000,12000,14000,16000,20000,24000,30000,40000},        // i4FV_DC[GMEAN_MAX_NUM][ISO_MAX_NUM];
            
           {10000,10000,10000,10000,10000,9000,9000,9000,
            10000,10000,10000,10000,10000,9000,9000,9000,
            10000,10000,10000,10000,10000,9000,9000,9000,
            10000,10000,10000,10000,10000,9000,9000,9000,
            10000,10000,10000,10000,10000,9000,9000,9000,
            10000,10000,10000,10000,10000,9000,9000,9000},         // i4MIN_TH[GMEAN_MAX_NUM][ISO_MAX_NUM];       
   
           {5,6,9,9,11,12,14,16,
            5,6,9,9,11,12,14,16,
            5,6,9,9,11,12,14,16,
            5,6,9,9,11,12,14,16,
            5,6,9,9,11,12,14,16,
            5,6,9,9,11,12,14,16} , // i4HW_TH[GMEAN_MAX_NUM][ISO_MAX_NUM];          
// ------------------------------------------------------------------------
           {0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0},        // i4FV_DC2[GMEAN_MAX_NUM][ISO_MAX_NUM];
            
           {0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0},         // i4MIN_TH2[GMEAN_MAX_NUM][ISO_MAX_NUM];
           
           {5,6,9,9,11,12,14,16,
            5,6,9,9,11,12,14,16,
            5,6,9,9,11,12,14,16,
            5,6,9,9,11,12,14,16,
            5,6,9,9,11,12,14,16,
            5,6,9,9,11,12,14,16}          // i4HW_TH2[GMEAN_MAX_NUM][ISO_MAX_NUM];        
// ------------------------------------------------------------------------           
          },

          1, // i4VAFC_FAIL_CNT;
          0, // i4CHANGE_CNT_DELTA;

          0, // i4LV_THRES;

          18, // i4WIN_PERCENT_W;
          24, // i4WIN_PERCENT_H;                
          100,  // i4InfPos;
          12, //i4AFC_STEP_SIZE;

          {
              {50, 100, 150, 200, 250}, // back to bestpos step
              { 0,   0,   0,   0,   0}  // hysteresis compensate step
          },

          {0, 100, 200, 350, 500}, // back jump
          500,  //i4BackJumpPos


          80, // i4FDWinPercent;
          40, // i4FDSizeDiff;

          7,   //i4StatGain          

          {0,0,0,0,0,0,0,0,0,0,
           0,0,0,0,0,0,0,0,0,0}// i4Coef[20];    [0] left-search enable, [1] left-moving step size, [2] 5 pointer curve fitting enable	 
    },

    {0}
};


UINT32 FM50AF_getDefaultData(VOID *pDataBuf, UINT32 size)
{
    UINT32 dataSize = sizeof(NVRAM_LENS_PARA_STRUCT);

    if ((pDataBuf == NULL) || (size < dataSize))
    {
        return 1;
    }

    // copy from Buff to global struct
    memcpy(pDataBuf, &FM50AF_LENS_PARA_DEFAULT_VALUE, dataSize);

    return 0;
}

PFUNC_GETLENSDEFAULT pFM50AF_getDefaultData = FM50AF_getDefaultData;
