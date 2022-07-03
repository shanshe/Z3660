enum zz_video_modes {
	ZZVMODE_1280x720,
	ZZVMODE_800x600,
	ZZVMODE_640x480,
	ZZVMODE_1024x768,
	ZZVMODE_1280x1024,
	ZZVMODE_1920x1080_60,
	ZZVMODE_720x576,		// 50Hz
	ZZVMODE_1920x1080_50,	// 50Hz
	ZZVMODE_720x480,
	ZZVMODE_640x512,
	ZZVMODE_1600x1200,
	ZZVMODE_2560x1440_30,
	ZZVMODE_CUSTOM,
	ZZVMODE_NUM,
};

struct zz_video_mode {
	int hres, vres;
	int hstart, hend, hmax;
	int vstart, vend, vmax;
	int polarity;
	int mhz, phz, vhz;
	int hdmi;
	int mul, div, div2;
};

enum custom_vmode_params {
	VMODE_PARAM_HRES,
	VMODE_PARAM_VRES,
	VMODE_PARAM_HSTART,
	VMODE_PARAM_HEND,
	VMODE_PARAM_HMAX,
	VMODE_PARAM_VSTART,
	VMODE_PARAM_VEND,
	VMODE_PARAM_VMAX,
	VMODE_PARAM_POLARITY,
	VMODE_PARAM_MHZ,
	VMODE_PARAM_PHZ,
	VMODE_PARAM_VHZ,
	VMODE_PARAM_HDMI,
	VMODE_PARAM_MUL,
	VMODE_PARAM_DIV,
	VMODE_PARAM_DIV2,
	VMODE_PARAM_NUM,
};


struct zz_video_mode preset_video_modes[ZZVMODE_NUM] = {
    //   HRES       VRES    HSTART  HEND    HMAX    VSTART  VEND    VMAX    POLARITY    MHZ     PIXELCLOCK HZ   VERTICAL HZ     HDMI    MUL/DIV/DIV2
    {    1280,      720,    1390,   1430,   1650,   725,    730,    750,    0,          75,     75000000,       60,             0,      15, 1, 20 },
    {    800,       600,    840,    968,    1056,   601,    605,    628,    0,          40,     40000000,       60,             0,      14, 1, 35 },
    {    640,       480,    656,    752,    800,    490,    492,    525,    0,          25,     25175000,       60,             0,      15, 1, 60 },
    {    1024,      768,    1048,   1184,   1344,   771,    777,    806,    0,          65,     65000000,       60,             0,      13, 1, 20 },
    {    1280,      1024,   1328,   1440,   1688,   1025,   1028,   1066,   0,          108,    108000000,      60,             0,      54, 5, 10 },
    {    1920,      1080,   2008,   2052,   2200,   1084,   1089,   1125,   0,          150,    150000000,      60,             0,      15, 1, 10 },
    {    720,       576,    732,    796,    864,    581,    586,    625,    1,          27,     27000000,       50,             0,      45, 2, 83 },
    {    1920,      1080,   2448,   2492,   2640,   1084,   1089,   1125,   0,          150,    150000000,      50,             0,      15, 1, 10 },
    {    720,       480,    720,    752,    800,    490,    492,    525,    0,          25,     25175000,       60,             0,      15, 1, 60 },
    {    640,       512,    840,    968,    1056,   601,    605,    628,    0,          40,     40000000,       60,             0,      14, 1, 35 },
	{	 1600,		1200,	1704,	1880,	2160,	1201,	1204,	1242,	0,			161,	16089999,		60,				0,		21, 1, 13 },
	{	 2560,		1440,	2680,	2944,	3328,	1441,	1444,	1465,	0,			146,	15846000,		30,				0,		41, 2, 14 },
    // The final exntry here is the custom video mode, accessible through registers for debug purposes.
    {    1280,      720,    1390,   1430,   1650,   725,    730,    750,    0,          75,     75000000,       60,             0,      15, 1, 20 },
};
