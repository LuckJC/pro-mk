#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov8865mipiraw.h"
#include "camera_info_ov8865mipiraw.h"
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
        81925,    // i4R_AVG
        18686,    // i4R_STD
        105175,    // i4B_AVG
        25390,    // i4B_STD
        {  // i4P00[9]
            4680000, -1947500, -170000, -775000, 3560000, -227500, -60000, -2087500, 4707500
        },
        {  // i4P10[9]
            279742, -671674, 393892, 239218, -607196, 359594, 140658, -127325, -20362
        },
        {  // i4P01[9]
            363445, -655808, 289821, -2501, -492016, 486315, -44001, -980661, 1021474
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
            1144,    // u4MinGain, 1024 base = 1x
            8192,    // u4MaxGain, 16x
            80,    // u4MiniISOGain, ISOxx  
            128,    // u4GainStepUnit, 1x/8 
            27,    // u4PreExpUnit 
            31,    // u4PreMaxFrameRate
            18,    // u4VideoExpUnit  
            31,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            14,    // u4CapExpUnit 
            30,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            22,    // u4LensFno, Fno = 2.8
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
            50,    // u4AETarget
            0,    // u4StrobeAETarget
            50,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            2,    // u4BlackLightStrengthIndex
            2,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            90,    // u4InDoorEV = 9.0, 10 base 
            -10,    // i4BVOffset delta BV = value/10 
            80,    // u4PreviewFlareOffset
            80,    // u4CaptureFlareOffset
            5,    // u4CaptureFlareThres
            80,    // u4VideoFlareOffset
            5,    // u4VideoFlareThres
            80,    // u4StrobeFlareOffset
            2,    // u4StrobeFlareThres
            4,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            4,    // u4VideoMaxFlareThres
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
                1000,    // i4R
                512,    // i4G
                752    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                105,    // i4X
                -389    // i4Y
            },
            // Horizon
            {
                -372,    // i4X
                -441    // i4Y
            },
            // A
            {
                -256,    // i4X
                -457    // i4Y
            },
            // TL84
            {
                -156,    // i4X
                -406    // i4Y
            },
            // CWF
            {
                -86,    // i4X
                -492    // i4Y
            },
            // DNP
            {
                105,    // i4X
                -389    // i4Y
            },
            // D65
            {
                105,    // i4X
                -389    // i4Y
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
                57,    // i4X
                -399    // i4Y
            },
            // Horizon
            {
                -422,    // i4X
                -393    // i4Y
            },
            // A
            {
                -309,    // i4X
                -422    // i4Y
            },
            // TL84
            {
                -204,    // i4X
                -384    // i4Y
            },
            // CWF
            {
                -145,    // i4X
                -478    // i4Y
            },
            // DNP
            {
                57,    // i4X
                -399    // i4Y
            },
            // D65
            {
                57,    // i4X
                -399    // i4Y
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
                1000,    // i4R
                512,    // i4G
                752    // i4B
            },
            // Horizon 
            {
                562,    // i4R
                512,    // i4G
                1539    // i4B
            },
            // A 
            {
                672,    // i4R
                512,    // i4G
                1345    // i4B
            },
            // TL84 
            {
                718,    // i4R
                512,    // i4G
                1096    // i4B
            },
            // CWF 
            {
                887,    // i4R
                512,    // i4G
                1118    // i4B
            },
            // DNP 
            {
                1000,    // i4R
                512,    // i4G
                752    // i4B
            },
            // D65 
            {
                1000,    // i4R
                512,    // i4G
                752    // i4B
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
            7,    // i4RotationAngle
            254,    // i4Cos
            31    // i4Sin
        },
        // Daylight locus parameter
        {
            -164,    // i4SlopeNumerator
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
            -254,    // i4RightBound
            -904,    // i4LeftBound
            -357,    // i4UpperBound
            -457    // i4LowerBound
            },
            // Warm fluorescent
            {
            -254,    // i4RightBound
            -904,    // i4LeftBound
            -457,    // i4UpperBound
            -577    // i4LowerBound
            },
            // Fluorescent
            {
            -80,    // i4RightBound
            -254,    // i4LeftBound
            -319,    // i4UpperBound
            -431    // i4LowerBound
            },
            // CWF
            {
            -80,    // i4RightBound
            -254,    // i4LeftBound
            -431,    // i4UpperBound
            -528    // i4LowerBound
            },
            // Daylight
            {
            82,    // i4RightBound
            -80,    // i4LeftBound
            -319,    // i4UpperBound
            -479    // i4LowerBound
            },
            // Shade
            {
            442,    // i4RightBound
            82,    // i4LeftBound
            -319,    // i4UpperBound
            -479    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            130,    // i4RightBound
            -80,    // i4LeftBound
            -479,    // i4UpperBound
            -577    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            442,    // i4RightBound
            -904,    // i4LeftBound
            0,    // i4UpperBound
            -577    // i4LowerBound
            },
            // Daylight
            {
            107,    // i4RightBound
            -80,    // i4LeftBound
            -319,    // i4UpperBound
            -479    // i4LowerBound
            },
            // Cloudy daylight
            {
            207,    // i4RightBound
            32,    // i4LeftBound
            -319,    // i4UpperBound
            -479    // i4LowerBound
            },
            // Shade
            {
            307,    // i4RightBound
            32,    // i4LeftBound
            -319,    // i4UpperBound
            -479    // i4LowerBound
            },
            // Twilight
            {
            -80,    // i4RightBound
            -240,    // i4LeftBound
            -319,    // i4UpperBound
            -479    // i4LowerBound
            },
            // Fluorescent
            {
            107,    // i4RightBound
            -304,    // i4LeftBound
            -334,    // i4UpperBound
            -528    // i4LowerBound
            },
            // Warm fluorescent
            {
            -209,    // i4RightBound
            -409,    // i4LeftBound
            -334,    // i4UpperBound
            -528    // i4LowerBound
            },
            // Incandescent
            {
            -209,    // i4RightBound
            -409,    // i4LeftBound
            -319,    // i4UpperBound
            -479    // i4LowerBound
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
            950,    // i4R
            512,    // i4G
            803    // i4B
            },
            // Cloudy daylight
            {
            1076,    // i4R
            512,    // i4G
            685    // i4B
            },
            // Shade
            {
            1142,    // i4R
            512,    // i4G
            635    // i4B
            },
            // Twilight
            {
            774,    // i4R
            512,    // i4G
            1044    // i4B
            },
            // Fluorescent
            {
            873,    // i4R
            512,    // i4G
            988    // i4B
            },
            // Warm fluorescent
            {
            681,    // i4R
            512,    // i4G
            1357    // i4B
            },
            // Incandescent
            {
            649,    // i4R
            512,    // i4G
            1307    // i4B
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
            50,    // i4SliderValue
            4270    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            50,    // i4SliderValue
            4270    // i4OffsetThr
            },
            // Shade
            {
            50,    // i4SliderValue
            343    // i4OffsetThr
            },
            // Daylight WB gain
            {
            1000,    // i4R
            512,    // i4G
            752    // i4B
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
                -479,    // i4RotatedXCoordinate[0]
                -366,    // i4RotatedXCoordinate[1]
                -261,    // i4RotatedXCoordinate[2]
                0,    // i4RotatedXCoordinate[3]
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


