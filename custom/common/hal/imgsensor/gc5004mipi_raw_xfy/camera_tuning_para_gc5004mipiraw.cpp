#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
 
#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_gc5004mipiraw.h"
#include "camera_info_gc5004mipiraw.h"
#include "camera_custom_AEPlinetable.h"
const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    Version: NVRAM_CAMERA_PARA_FILE_VERSION,
    //SensorId
    SensorId: SENSOR_ID,
    ISPComm:{
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    },
    ISPPca:{
        #include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    ISPRegs:{
        #include INCLUDE_FILENAME_ISP_REGS_PARAM
        },
    ISPMfbMixer:{{
        {//00: MFB mixer for ISO 100
            0x00000000, 0x00000000
        },
        {//01: MFB mixer for ISO 200
            0x00000000, 0x00000000
        },
        {//02: MFB mixer for ISO 400
            0x00000000, 0x00000000
        },
        {//03: MFB mixer for ISO 800
            0x00000000, 0x00000000
        },
        {//04: MFB mixer for ISO 1600
            0x00000000, 0x00000000
        },
        {//05: MFB mixer for ISO 2400
            0x00000000, 0x00000000
        },
        {//06: MFB mixer for ISO 3200
            0x00000000, 0x00000000
        }
    }},
    ISPCcmPoly22:{
        61125,    // i4R_AVG
        13230,    // i4R_STD
        82475,    // i4B_AVG
        21155,    // i4B_STD
        {  // i4P00[9]
            6507500, -3375000, -570000, -1177500, 4017500, -280000, -495000, -1707500, 4757500
        },
        {  // i4P10[9]
            95873, -406194, 308698, 196995, -244506, 47512, 113544, -129217, 22160
        },
        {  // i4P01[9]
            600182, -720515, 114547, 390432, -352014, -38418, 192856, -212761, 29706
        },
        {  // i4P20[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P11[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P02[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        // rDevicesInfo
        {
            1024,    // u4MinGain, 1024 base = 1x
            7186,    // u4MaxGain, 16x
            52,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            25,    // u4PreExpUnit 
            25,    // u4PreMaxFrameRate
            25,    // u4VideoExpUnit  
            25,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            50,    // u4CapExpUnit 
            10,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            28,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
        },
        // rHistConfig
        {
            2,    // u4HistHighThres
            40,    // u4HistLowThres
            2,    // u4MostBrightRatio
            1,    // u4MostDarkRatio
            160,    // u4CentralHighBound
            20,    // u4CentralLowBound
            {240, 230, 220, 210, 200},    // u4OverExpThres[AE_CCT_STRENGTH_NUM] 
            {86, 108, 128, 148, 170},    // u4HistStretchThres[AE_CCT_STRENGTH_NUM] 
            {18, 22, 26, 30, 34}    // u4BlackLightThres[AE_CCT_STRENGTH_NUM] 
        },
        // rCCTConfig
        {
            TRUE,    // bEnableBlackLight
            TRUE,    // bEnableHistStretch
            FALSE,    // bEnableAntiOverExposure
            TRUE,    // bEnableTimeLPF
            FALSE,    // bEnableCaptureThres
            FALSE,    // bEnableVideoThres
            FALSE,    // bEnableStrobeThres
            60,    // u4AETarget
            52,    // u4StrobeAETarget
            40,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            4,    // u4BlackLightStrengthIndex
            2,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            90,    // u4InDoorEV = 9.0, 10 base 
            8,    // i4BVOffset delta BV = value/10 
            64,    // u4PreviewFlareOffset
            64,    // u4CaptureFlareOffset
            5,    // u4CaptureFlareThres
            64,    // u4VideoFlareOffset
            5,    // u4VideoFlareThres
            64,    // u4StrobeFlareOffset
            5,    // u4StrobeFlareThres
            50,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            50,    // u4VideoMaxFlareThres
            0,    // u4VideoMinFlareThres
            18,    // u4FlatnessThres    // 10 base for flatness condition.
            75    // u4FlatnessStrength
        }
    },
    // AWB NVRAM
    {
        // AWB calibration data
        {
            // rUnitGain (unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rGoldenGain (golden sample gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
            {
                0,    // i4R
                0,    // i4G
                0    // i4B
            },
            // rD65Gain (D65 WB gain: 1.0 = 512)
            {
                673,    // i4R
                512,    // i4G
                578    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                0,    // i4X
                0    // i4Y
            },
            // Horizon
            {
                -474,    // i4X
                -222    // i4Y
            },
            // A
            {
                -361,    // i4X
                -225    // i4Y
            },
            // TL84
            {
                -219,    // i4X
                -223    // i4Y
            },
            // CWF
            {
                -164,    // i4X
                -312    // i4Y
            },
            // DNP
            {
                -1,    // i4X
                -212    // i4Y
            },
            // D65
            {
                56,    // i4X
                -145    // i4Y
            },
            // DF
            {
                0,    // i4X
                0    // i4Y
            }
        },
        // Rotated XY coordinate of AWB light source
        {
            // Strobe
            {
                0,    // i4X
                0    // i4Y
            },
            // Horizon
            {
                -505,    // i4X
                -137    // i4Y
            },
            // A
            {
                -394,    // i4X
                -159    // i4Y
            },
            // TL84
            {
                -254,    // i4X
                -182    // i4Y
            },
            // CWF
            {
                -215,    // i4X
                -279    // i4Y
            },
            // DNP
            {
                -37,    // i4X
                -209    // i4Y
            },
            // D65
            {
                30,    // i4X
                -152    // i4Y
            },
            // DF
            {
                0,    // i4X
                0    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                512,    // i4R
                512,    // i4G
                512    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                721,    // i4G
                1850    // i4B
            },
            // A 
            {
                512,    // i4R
                616,    // i4G
                1361    // i4B
            },
            // TL84 
            {
                515,    // i4R
                512,    // i4G
                931    // i4B
            },
            // CWF 
            {
                626,    // i4R
                512,    // i4G
                976    // i4B
            },
            // DNP 
            {
                682,    // i4R
                512,    // i4G
                683    // i4B
            },
            // D65 
            {
                673,    // i4R
                512,    // i4G
                578    // i4B
            },
            // DF 
            {
                512,    // i4R
                512,    // i4G
                512    // i4B
            }
        },
        // Rotation matrix parameter
        {
            10,    // i4RotationAngle
            252,    // i4Cos
            44    // i4Sin
        },
        // Daylight locus parameter
        {
            -182,    // i4SlopeNumerator
            128    // i4SlopeDenominator
        },
        // AWB light area
        {
            // Strobe:FIXME
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            },
            // Tungsten
            {
            -304,    // i4RightBound
            -954,    // i4LeftBound
            -98,    // i4UpperBound
            -198    // i4LowerBound
            },
            // Warm fluorescent
            {
            -304,    // i4RightBound
            -954,    // i4LeftBound
            -198,    // i4UpperBound
            -318    // i4LowerBound
            },
            // Fluorescent
            {
            -100,    // i4RightBound
            -304,    // i4LeftBound
            -85,    // i4UpperBound
            -230    // i4LowerBound
            },
            // CWF
            {
            -100,    // i4RightBound
            -304,    // i4LeftBound
            -230,    // i4UpperBound
            -380    // i4LowerBound
            },
            // Daylight
            {
            115,    // i4RightBound
            -100,    // i4LeftBound
            -72,    // i4UpperBound
            -420    // i4LowerBound
            },
            // Shade
            {
            415,    // i4RightBound
            115,    // i4LeftBound
            -72,    // i4UpperBound
            -350    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            415,    // i4RightBound
            -954,    // i4LeftBound
            0,    // i4UpperBound
            -420    // i4LowerBound
            },
            // Daylight
            {
            140,    // i4RightBound
            -100,    // i4LeftBound
            -72,    // i4UpperBound
            -420    // i4LowerBound
            },
            // Cloudy daylight
            {
            240,    // i4RightBound
            65,    // i4LeftBound
            -72,    // i4UpperBound
            -420    // i4LowerBound
            },
            // Shade
            {
            340,    // i4RightBound
            65,    // i4LeftBound
            -72,    // i4UpperBound
            -420    // i4LowerBound
            },
            // Twilight
            {
            -100,    // i4RightBound
            -260,    // i4LeftBound
            -72,    // i4UpperBound
            -420    // i4LowerBound
            },
            // Fluorescent
            {
            80,    // i4RightBound
            -354,    // i4LeftBound
            -102,    // i4UpperBound
            -329    // i4LowerBound
            },
            // Warm fluorescent
            {
            -294,    // i4RightBound
            -494,    // i4LeftBound
            -102,    // i4UpperBound
            -329    // i4LowerBound
            },
            // Incandescent
            {
            -294,    // i4RightBound
            -494,    // i4LeftBound
            -72,    // i4UpperBound
            -420    // i4LowerBound
            },
            // Gray World
            {
            5000,    // i4RightBound
            -5000,    // i4LeftBound
            5000,    // i4UpperBound
            -5000    // i4LowerBound
            }
        },
        // PWB default gain	
        {
            // Daylight
            {
            770,    // i4R
            512,    // i4G
            651    // i4B
            },
            // Cloudy daylight
            {
            891,    // i4R
            512,    // i4G
            529    // i4B
            },
            // Shade
            {
            941,    // i4R
            512,    // i4G
            489    // i4B
            },
            // Twilight
            {
            617,    // i4R
            512,    // i4G
            890    // i4B
            },
            // Fluorescent
            {
            617,    // i4R
            512,    // i4G
            805    // i4B
            },
            // Warm fluorescent
            {
            465,    // i4R
            512,    // i4G
            1204    // i4B
            },
            // Incandescent
            {
            488,    // i4R
            512,    // i4G
            1245    // i4B
            },
            // Gray World
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        // AWB preference color	
        {
            // Tungsten
            {
            0,    // i4SliderValue
            6700    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            5623    // i4OffsetThr
            },
            // Shade
            {
            0,    // i4SliderValue
            2177    // i4OffsetThr
            },
            // Daylight WB gain
            {
            624,    // i4R
            512,    // i4G
            641    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: warm fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: CWF
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: shade
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        {// CCT estimation
            {// CCT
                2300,    // i4CCT[0]
                2850,    // i4CCT[1]
                4100,    // i4CCT[2]
                5100,    // i4CCT[3]
                6500    // i4CCT[4]
            },
            {// Rotated X coordinate
                -535,    // i4RotatedXCoordinate[0]
                -424,    // i4RotatedXCoordinate[1]
                -284,    // i4RotatedXCoordinate[2]
                -67,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
            }
        }
    },
    {0}
};

#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace


typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
{
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
                                             sizeof(NVRAM_CAMERA_3A_STRUCT),
                                             sizeof(NVRAM_CAMERA_SHADING_STRUCT),
                                             sizeof(NVRAM_LENS_PARA_STRUCT),
                                             sizeof(AE_PLINETABLE_T)};

    if (CameraDataType > CAMERA_DATA_AE_PLINETABLE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
    {
        return 1;
    }

    switch(CameraDataType)
    {
        case CAMERA_NVRAM_DATA_ISP:
            memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_3A:
            memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_SHADING:
            memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
            break;
        case CAMERA_DATA_AE_PLINETABLE:
            memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
            break;
        default:
            break;
    }
    return 0;
}}; // NSFeature


