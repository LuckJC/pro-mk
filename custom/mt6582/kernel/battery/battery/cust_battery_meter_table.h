#ifndef _CUST_BATTERY_METER_TABLE_H
#define _CUST_BATTERY_METER_TABLE_H

#include <mach/mt_typedefs.h>

// ============================================================
// define
// ============================================================
#define BAT_NTC_10 1
#define BAT_NTC_47 0

#if (BAT_NTC_10 == 1)
#define RBAT_PULL_UP_R             16900	
#define RBAT_PULL_DOWN_R		   27000	
#endif

#if (BAT_NTC_47 == 1)
#define RBAT_PULL_UP_R             61900	
#define RBAT_PULL_DOWN_R		  100000	
#endif
#define RBAT_PULL_UP_VOLT          1800



// ============================================================
// ENUM
// ============================================================

// ============================================================
// structure
// ============================================================

// ============================================================
// typedef
// ============================================================
typedef struct _BATTERY_PROFILE_STRUC
{
    kal_int32 percentage;
    kal_int32 voltage;
} BATTERY_PROFILE_STRUC, *BATTERY_PROFILE_STRUC_P;

typedef struct _R_PROFILE_STRUC
{
    kal_int32 resistance; // Ohm
    kal_int32 voltage;
} R_PROFILE_STRUC, *R_PROFILE_STRUC_P;

typedef enum
{
    T1_0C,
    T2_25C,
    T3_50C
} PROFILE_TEMPERATURE;

// ============================================================
// External Variables
// ============================================================

// ============================================================
// External function
// ============================================================

// ============================================================
// <DOD, Battery_Voltage> Table
// ============================================================
#if (BAT_NTC_10 == 1)
    BATT_TEMPERATURE Batt_Temperature_Table[] = {
        {-20,68237},
        {-15,53650},
        {-10,42506},
        { -5,33892},
        {  0,27219},
        {  5,22021},
        { 10,17926},
        { 15,14674},
        { 20,12081},
        { 25,10000},
        { 30,8315},
        { 35,6948},
        { 40,5834},
        { 45,4917},
        { 50,4161},
        { 55,3535},
        { 60,3014}
    };
#endif

#if (BAT_NTC_47 == 1)
    BATT_TEMPERATURE Batt_Temperature_Table[] = {
        {-20,483954},
        {-15,360850},
        {-10,271697},
        { -5,206463},
        {  0,158214},
        {  5,122259},
        { 10,95227},
        { 15,74730},
        { 20,59065},
        { 25,47000},
        { 30,37643},
        { 35,30334},
        { 40,24591},
        { 45,20048},
        { 50,16433},
        { 55,13539},
        { 60,11210}        
    };
#endif

// T0 -10C
BATTERY_PROFILE_STRUC battery_profile_t0[] =
{
	{0   , 4300},         
	{2   , 4277},         
	{4   , 4258},         
	{6   , 4238},         
	{8   , 4219},         
	{9   , 4197},         
	{11  , 4174},         
	{13  , 4148},         
	{15  , 4121},         
	{17  , 4097},         
	{19  , 4077},         
	{21  , 4059},         
	{23  , 4042},         
	{25  , 4024},         
	{26  , 4007},         
	{28  , 3991},         
	{30  , 3976},         
	{32  , 3962},         
	{34  , 3948},         
	{36  , 3934},         
	{38  , 3920},         
	{40  , 3906},         
	{42  , 3894},         
	{43  , 3883},         
	{45  , 3872},         
	{47  , 3863},         
	{49  , 3855},         
	{51  , 3846},         
	{53  , 3838},         
	{55  , 3832},         
	{57  , 3824},         
	{59  , 3818},         
	{61  , 3811},         
	{62  , 3806},         
	{64  , 3799},         
	{66  , 3794},         
	{68  , 3788},         
	{70  , 3783},         
	{72  , 3777},         
	{74  , 3771},         
	{76  , 3766},         
	{78  , 3758},         
	{79  , 3749},         
	{81  , 3741},         
	{83  , 3732},         
	{85  , 3723},         
	{87  , 3716},         
	{89  , 3709},         
	{90  , 3703},         
	{91  , 3699},         
	{92  , 3697},         
	{93  , 3694},         
	{94  , 3692},         
	{94  , 3690},         
	{95  , 3687},         
	{95  , 3685},         
	{96  , 3682},         
	{96  , 3680},         
	{97  , 3677},         
	{97  , 3673},         
	{97  , 3670},          
  {98  , 3666},
  {98  , 3662},
  {98  , 3658},
  {98  , 3654},
  {99  , 3650},
  {99  , 3647},
  {99  , 3643},
  {100  , 3640},
};      
        
// T1 0C 
BATTERY_PROFILE_STRUC battery_profile_t1[] =
{
	{0   , 4328},         
	{2   , 4290},         
	{3   , 4245},         
	{5   , 4219},         
	{7   , 4200},         
	{8   , 4184},         
	{10  , 4167},         
	{11  , 4151},         
	{13  , 4136},         
	{15  , 4121},         
	{16  , 4107},         
	{18  , 4093},         
	{20  , 4080},         
	{21  , 4066},         
	{23  , 4052},         
	{25  , 4038},         
	{26  , 4023},         
	{28  , 4008},         
	{29  , 3995},         
	{31  , 3982},         
	{33  , 3969},         
	{34  , 3955},         
	{36  , 3940},         
	{38  , 3924},         
	{39  , 3910},         
	{41  , 3896},         
	{43  , 3886},         
	{44  , 3876},         
	{46  , 3867},         
	{47  , 3859},         
	{49  , 3852},         
	{51  , 3844},         
	{52  , 3837},         
	{54  , 3830},         
	{56  , 3824},         
	{57  , 3819},         
	{59  , 3813},         
	{61  , 3807},         
	{62  , 3802},         
	{64  , 3797},         
	{65  , 3791},         
	{67  , 3787},         
	{69  , 3782},         
	{70  , 3777},         
	{72  , 3772},         
	{74  , 3766},         
	{75  , 3759},         
	{77  , 3749},         
	{79  , 3738},         
	{80  , 3726},         
	{82  , 3713},         
	{83  , 3706},         
	{85  , 3701},         
	{87  , 3697},         
	{88  , 3693},         
	{90  , 3688},         
	{92  , 3677},         
	{93  , 3644},         
  {95  , 3598},
	{96  , 3566},         
	{97  , 3547},          
  {97  , 3532},
  {98  , 3520},
  {98  , 3507},
  {98  , 3498},
  {99  , 3490},
  {99  , 3483},
  {99  , 3477},
  {100  , 3471},
};           

// T2 25C
BATTERY_PROFILE_STRUC battery_profile_t2[] =
{
	{0   , 4334},         
	{1   , 4313},         
	{3   , 4295},         
	{4   , 4278},         
	{6   , 4262},         
	{7   , 4246},         
	{9   , 4230},         
	{10  , 4214},         
	{12  , 4199},         
	{13  , 4183},         
	{15  , 4169},         
	{16  , 4153},         
	{18  , 4138},         
	{19  , 4123},         
	{21  , 4109},         
	{22  , 4095},         
	{24  , 4081},         
	{25  , 4068},         
	{27  , 4055},         
	{28  , 4041},         
	{30  , 4028},         
	{31  , 4014},         
	{33  , 4002},         
	{34  , 3991},         
	{36  , 3979},         
	{37  , 3967},         
	{39  , 3953},         
	{40  , 3936},         
	{41  , 3919},         
	{43  , 3904},         
	{44  , 3891},         
	{46  , 3881},         
	{47  , 3872},         
	{49  , 3863},         
	{50  , 3856},         
	{52  , 3849},         
	{53  , 3842},         
	{55  , 3835},         
	{56  , 3829},         
	{58  , 3823},         
	{59  , 3817},         
	{61  , 3811},         
	{62  , 3806},         
	{64  , 3800},         
	{65  , 3795},         
	{67  , 3790},         
	{68  , 3784},         
	{70  , 3778},         
	{71  , 3770},         
	{73  , 3761},         
	{74  , 3752},         
	{76  , 3743},         
	{77  , 3731},         
	{79  , 3718},         
	{80  , 3704},         
	{81  , 3694},         
	{83  , 3691},         
	{84  , 3688},         
	{86  , 3687},         
	{87  , 3683},         
	{89  , 3676},          
  {90  , 3653},
  {92  , 3630},
  {93  , 3601},
  {95  , 3565},
  {96  , 3522},
  {98  , 3475},
  {99  , 3429},
  {100 , 3393},

};     

// T3 50C
BATTERY_PROFILE_STRUC battery_profile_t3[] =
{
	{0   , 4339},         
	{2   , 4322},         
	{3   , 4305},         
	{5   , 4289},         
	{6   , 4272},         
	{8   , 4256},         
	{9   , 4240},         
	{11  , 4224},         
	{12  , 4209},         
	{14  , 4192},         
	{15  , 4177},         
	{17  , 4162},         
	{18  , 4146},         
	{20  , 4132},         
	{21  , 4117},         
	{23  , 4103},         
	{24  , 4089},         
	{26  , 4075},         
	{27  , 4062},         
	{29  , 4048},         
	{31  , 4035},         
	{32  , 4022},         
	{34  , 4010},         
	{35  , 3997},         
	{37  , 3984},         
	{38  , 3971},         
	{40  , 3952},         
	{41  , 3932},         
	{43  , 3917},         
	{44  , 3906},         
	{46  , 3895},         
	{47  , 3885},         
	{49  , 3876},         
	{50  , 3868},         
	{52  , 3860},         
	{53  , 3853},         
	{55  , 3845},         
	{56  , 3838},         
	{58  , 3832},         
	{60  , 3825},         
	{61  , 3819},         
	{63  , 3812},         
	{64  , 3805},         
	{66  , 3795},         
	{67  , 3783},         
	{69  , 3775},         
	{70  , 3766},         
	{72  , 3757},         
	{73  , 3749},         
	{75  , 3739},         
	{76  , 3729},         
	{78  , 3714},         
	{79  , 3701},         
	{81  , 3686},         
	{82  , 3680},         
	{84  , 3677},         
	{86  , 3675},         
	{87  , 3672},         
	{89  , 3669},         
	{90  , 3650},         
	{92  , 3632},          
  {93  , 3603},
  {95  , 3566},
  {96  , 3523},
  {98  , 3475},
  {99  , 3428},
  {100 , 3370},
  {100 , 3327},
  {100 , 3300},
 
};           

// battery profile for actual temperature. The size should be the same as T1, T2 and T3
BATTERY_PROFILE_STRUC battery_profile_temperature[] =
{
  {0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },  
	{0  , 0 }, 
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },  
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },

};    

// ============================================================
// <Rbat, Battery_Voltage> Table
// ============================================================
// T0 -10C
R_PROFILE_STRUC r_profile_t0[] =
{
	{588  , 4300},         
	{588  , 4277},         
	{600  , 4258},         
	{608  , 4238},         
	{623  , 4219},         
	{643  , 4197},         
	{665  , 4174},         
	{698  , 4148},         
	{745  , 4121},         
	{825  , 4097},         
	{883  , 4077},         
	{910  , 4059},         
	{923  , 4042},         
	{923  , 4024},         
	{923  , 4007},         
	{920  , 3991},         
	{920  , 3976},         
	{920  , 3962},         
	{918  , 3948},         
	{918  , 3934},         
	{915  , 3920},         
	{910  , 3906},         
	{910  , 3894},         
	{913  , 3883},         
	{913  , 3872},         
	{915  , 3863},         
	{918  , 3855},         
	{920  , 3846},         
	{925  , 3838},         
	{933  , 3832},         
	{933  , 3824},         
	{943  , 3818},         
	{945  , 3811},         
	{953  , 3806},         
	{958  , 3799},         
	{965  , 3794},         
	{968  , 3788},         
	{975  , 3783},         
	{980  , 3777},         
	{988  , 3771},         
	{1005 , 3766},         
	{1020 , 3758},         
	{1035 , 3749},         
	{1063 , 3741},         
	{1093 , 3732},         
	{1130 , 3723},         
	{1180 , 3716},         
	{1238 , 3709},         
	{1260 , 3703},         
	{1248 , 3699},         
	{1243 , 3697},         
	{1235 , 3694},         
	{1233 , 3692},         
	{1225 , 3690},         
	{1220 , 3687},         
	{1213 , 3685},         
	{1205 , 3682},         
	{1203 , 3680},         
	{1193 , 3677},         
	{1185 , 3673},         
	{1180 , 3670},          
  {1170 , 3666},
  {1155 , 3662},
  {1148 , 3658},
  {1140 , 3654},
  {1125 , 3650},
  {1120 , 3647},
  {1113 , 3643},
  {1105 , 3640},

};      

// T1 0C
R_PROFILE_STRUC r_profile_t1[] =
{
	{308 , 4328},         
	{308 , 4290},         
	{388 , 4245},         
	{443 , 4219},         
	{458 , 4200},         
	{468 , 4184},         
	{473 , 4167},         
	{478 , 4151},         
	{485 , 4136},         
	{490 , 4121},         
	{495 , 4107},         
	{495 , 4093},         
	{505 , 4080},         
	{520 , 4066},         
	{530 , 4052},         
	{538 , 4038},         
	{538 , 4023},         
	{533 , 4008},         
	{528 , 3995},         
	{528 , 3982},         
	{523 , 3969},         
	{515 , 3955},         
	{510 , 3940},         
	{500 , 3924},         
	{495 , 3910},         
	{485 , 3896},         
	{488 , 3886},         
	{488 , 3876},         
	{488 , 3867},         
	{493 , 3859},         
	{495 , 3852},         
	{498 , 3844},         
	{500 , 3837},         
	{503 , 3830},         
	{505 , 3824},         
	{510 , 3819},         
	{513 , 3813},         
	{515 , 3807},         
	{520 , 3802},         
	{523 , 3797},         
	{523 , 3791},         
	{528 , 3787},         
	{528 , 3782},         
	{530 , 3777},         
	{530 , 3772},         
	{533 , 3766},         
	{540 , 3759},         
	{545 , 3749},         
	{555 , 3738},         
	{570 , 3726},         
	{590 , 3713},         
	{620 , 3706},         
	{655 , 3701},         
	{700 , 3697},         
	{748 , 3693},         
	{798 , 3688},         
	{843 , 3677},         
	{865 , 3644},         
	{920 , 3598},         
	{918 , 3566},         
	{870 , 3547},          
  {835 , 3532},
  {803 , 3520},
  {770 , 3507},
  {748 , 3498},
  {730 , 3490},
  {710 , 3483},
  {695 , 3477},
  {678 , 3471},

};     

// T2 25C
R_PROFILE_STRUC r_profile_t2[] =
{
	{183 , 4334},         
	{183 , 4313},         
	{185 , 4295},         
	{188 , 4278},         
	{190 , 4262},         
	{193 , 4246},         
	{193 , 4230},         
	{195 , 4214},         
	{200 , 4199},         
	{200 , 4183},         
	{205 , 4169},         
	{205 , 4153},         
	{208 , 4138},         
	{210 , 4123},         
	{215 , 4109},         
	{218 , 4095},         
	{220 , 4081},         
	{225 , 4068},         
	{230 , 4055},         
	{233 , 4041},         
	{235 , 4028},         
	{240 , 4014},         
	{243 , 4002},         
	{245 , 3991},         
	{245 , 3979},         
	{245 , 3967},         
	{240 , 3953},         
	{228 , 3936},         
	{213 , 3919},         
	{203 , 3904},         
	{195 , 3891},         
	{195 , 3881},         
	{198 , 3872},         
	{195 , 3863},         
	{198 , 3856},         
	{200 , 3849},         
	{200 , 3842},         
	{200 , 3835},         
	{205 , 3829},         
	{208 , 3823},         
	{210 , 3817},         
	{208 , 3811},         
	{213 , 3806},         
	{213 , 3800},         
	{213 , 3795},         
	{215 , 3790},         
	{213 , 3784},         
	{213 , 3778},         
	{208 , 3770},         
	{203 , 3761},         
	{198 , 3752},         
	{198 , 3743},         
	{195 , 3731},         
	{198 , 3718},         
	{195 , 3704},         
	{195 , 3694},         
	{198 , 3691},         
	{208 , 3688},         
	{223 , 3687},         
	{235 , 3683},         
	{250 , 3676},          
  {275 , 3653},
  {320 , 3630},
  {338 , 3601},
  {358 , 3565},
  {390 , 3522},
  {445 , 3475},
  {525 , 3429},
  {488 , 3393},

}; 

// T3 50C
R_PROFILE_STRUC r_profile_t3[] =
{
	{143 , 4339},         
	{143 , 4322},         
	{143 , 4305},         
	{145 , 4289},         
	{143 , 4272},         
	{145 , 4256},         
	{148 , 4240},         
	{148 , 4224},         
	{153 , 4209},         
	{150 , 4192},         
	{153 , 4177},         
	{153 , 4162},         
	{155 , 4146},         
	{158 , 4132},         
	{160 , 4117},         
	{160 , 4103},         
	{163 , 4089},         
	{165 , 4075},         
	{168 , 4062},         
	{170 , 4048},         
	{173 , 4035},         
	{175 , 4022},         
	{180 , 4010},         
	{183 , 3997},         
	{185 , 3984},         
	{190 , 3971},         
	{175 , 3952},         
	{158 , 3932},         
	{150 , 3917},         
	{153 , 3906},         
	{150 , 3895},         
	{150 , 3885},         
	{150 , 3876},         
	{153 , 3868},         
	{155 , 3860},         
	{158 , 3853},         
	{160 , 3845},         
	{163 , 3838},         
	{165 , 3832},         
	{168 , 3825},         
	{170 , 3819},         
	{170 , 3812},         
	{170 , 3805},         
	{163 , 3795},         
	{155 , 3783},         
	{158 , 3775},         
	{158 , 3766},         
	{155 , 3757},         
	{160 , 3749},         
	{160 , 3739},         
	{160 , 3729},         
	{155 , 3714},         
	{160 , 3701},         
	{155 , 3686},         
	{153 , 3680},         
	{155 , 3677},         
	{160 , 3675},         
	{165 , 3672},         
	{175 , 3669},         
	{190 , 3650},         
	{228 , 3632},          
  {230 , 3603},
  {238 , 3566},
  {250 , 3523},
  {268 , 3475},
  {300 , 3428},
  {330 , 3370},
  {318 , 3327},
  {250 , 3300},

}; 

// r-table profile for actual temperature. The size should be the same as T1, T2 and T3
R_PROFILE_STRUC r_profile_temperature[] =
{
  {0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },  
	{0  , 0 }, 
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },  
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },
	{0  , 0 },

};    

// ============================================================
// function prototype
// ============================================================
int fgauge_get_saddles(void);
BATTERY_PROFILE_STRUC_P fgauge_get_profile(kal_uint32 temperature);

int fgauge_get_saddles_r_table(void);
R_PROFILE_STRUC_P fgauge_get_profile_r_table(kal_uint32 temperature);

#endif	//#ifndef _CUST_BATTERY_METER_TABLE_H

