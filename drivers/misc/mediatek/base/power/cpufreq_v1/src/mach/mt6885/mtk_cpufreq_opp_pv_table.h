/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#include "mtk_cpufreq_config.h"
#include "mtk_cpufreq_platform.h"

#define NR_FREQ		16
#define ARRAY_COL_SIZE	4

static unsigned int FY_6885Tbl[NR_FREQ * NR_MT_CPU_DVFS][ARRAY_COL_SIZE] = {
	/* Freq, Vproc, post_div, clk_div */
	/* LL */
	{ 2000, 160, 1, 1 },
	{ 1895, 155, 1, 1 },
	{ 1791, 149, 1, 1 },
	{ 1708, 145, 1, 1 },
	{ 1625, 141, 1, 1 },
	{ 1500, 134, 2, 1 },
	{ 1393, 130, 2, 1 },
	{ 1287, 126, 2, 1 },
	{ 1181, 123, 2, 1 },
	{ 1048, 120, 2, 1 },
	{ 968, 115, 2, 1 },
	{ 862, 111, 2, 1 },
	{ 756, 108, 2, 1 },
	{ 703, 106, 4, 1 },
	{ 650, 104, 4, 1 },
	{ 500, 104, 4, 1 },




	/* L */
	{ 2202, 160, 1, 1 },
	{ 2106, 154, 1, 1 },
	{ 2050, 151, 1, 1 },
	{ 1975, 147, 1, 1 },
	{ 1900, 143, 1, 1 },
	{ 1803, 137, 1, 1 },
	{ 1750, 134, 1, 1 },
	{ 1622, 130, 1, 1 },
	{ 1526, 127, 2, 1 },
	{ 1367, 123, 2, 1 },
	{ 1271, 120, 2, 1 },
	{ 1176, 117, 2, 1 },
	{ 1048, 113, 2, 1 },
	{ 921, 110, 2, 1 },
	{ 825, 107, 2, 1 },
	{ 730, 104, 4, 1 },




	/* CCI */
	{ 1540, 160, 2, 1 },
	{ 1469, 155, 2, 1 },
	{ 1426, 151, 2, 1 },
	{ 1370, 147, 2, 1 },
	{ 1313, 143, 2, 1 },
	{ 1256, 139, 2, 1 },
	{ 1195, 134, 2, 1 },
	{ 1115, 130, 2, 1 },
	{ 1030, 127, 2, 1 },
	{ 945, 123, 2, 1 },
	{ 881, 120, 2, 1 },
	{ 817, 117, 2, 1 },
	{ 711, 112, 4, 1 },
	{ 668, 111, 4, 1 },
	{ 583, 107, 4, 1 },
	{ 520, 104, 4, 1 },


};

static unsigned int TB_6885Tbl[NR_FREQ * NR_MT_CPU_DVFS][ARRAY_COL_SIZE] = {
	/* Freq, Vproc, post_div, clk_div */
	/* LL */
	{ 2000, 160, 1, 1 },
	{ 1895, 155, 1, 1 },
	{ 1791, 149, 1, 1 },
	{ 1708, 145, 1, 1 },
	{ 1625, 141, 1, 1 },
	{ 1500, 134, 2, 1 },
	{ 1393, 129, 2, 1 },
	{ 1287, 124, 2, 1 },
	{ 1181, 120, 2, 1 },
	{ 1048, 114, 2, 1 },
	{ 968, 110, 2, 1 },
	{ 862, 105, 2, 1 },
	{ 756, 101, 2, 1 },
	{ 703, 98, 4, 1 },
	{ 650, 96, 4, 1 },
	{ 500, 96, 4, 1 },


	/* L */
	{ 2400, 160, 1, 1 },
	{ 2300, 160, 1, 1 },
	{ 2106, 154, 1, 1 },
	{ 2012, 149, 1, 1 },
	{ 1900, 143, 1, 1 },
	{ 1803, 137, 1, 1 },
	{ 1750, 134, 1, 1 },
	{ 1622, 129, 1, 1 },
	{ 1526, 126, 2, 1 },
	{ 1367, 120, 2, 1 },
	{ 1271, 116, 2, 1 },
	{ 1176, 113, 2, 1 },
	{ 1048, 108, 2, 1 },
	{ 921,  103, 2, 1 },
	{ 825,  100, 2, 1 },
	{ 730,  96, 4, 1 },

	/* CCI */
	{ 1540, 160, 2, 1 },
	{ 1469, 155, 2, 1 },
	{ 1426, 151, 2, 1 },
	{ 1370, 147, 2, 1 },
	{ 1313, 143, 2, 1 },
	{ 1256, 139, 2, 1 },
	{ 1195, 134, 2, 1 },
	{ 1115, 129, 2, 1 },
	{ 1030, 125, 2, 1 },
	{ 945, 120, 2, 1 },
	{ 881, 116, 2, 1 },
	{ 817, 113, 2, 1 },
	{ 711, 107, 4, 1 },
	{ 668, 104, 4, 1 },
	{ 583, 100, 4, 1 },
	{ 520, 96, 4, 1 },


};

static unsigned int FY_6885TTbl[NR_FREQ * NR_MT_CPU_DVFS][ARRAY_COL_SIZE] = {
	/* Freq, Vproc, post_div, clk_div */
	/* LL */
	{ 2000, 160, 1, 1 },
	{ 1895, 155, 1, 1 },
	{ 1791, 149, 1, 1 },
	{ 1708, 145, 1, 1 },
	{ 1625, 141, 1, 1 },
	{ 1500, 134, 2, 1 },
	{ 1393, 130, 2, 1 },
	{ 1287, 126, 2, 1 },
	{ 1181, 123, 2, 1 },
	{ 1048, 120, 2, 1 },
	{ 968, 115, 2, 1 },
	{ 862, 111, 2, 1 },
	{ 756, 108, 2, 1 },
	{ 703, 106, 4, 1 },
	{ 650, 104, 4, 1 },
	{ 500, 104, 4, 1 },



	/* L */
	{ 2600, 160, 1, 1 },
	{ 2529, 158, 1, 1 },
	{ 2387, 153, 1, 1 },
	{ 2245, 149, 1, 1 },
	{ 2068, 144, 1, 1 },
	{ 1927, 139, 1, 1 },
	{ 1750, 134, 1, 1 },
	{ 1622, 130, 1, 1 },
	{ 1526, 127, 2, 1 },
	{ 1367, 123, 2, 1 },
	{ 1271, 120, 2, 1 },
	{ 1176, 117, 2, 1 },
	{ 1048, 113, 2, 1 },
	{ 921, 110, 2, 1 },
	{ 825, 107, 2, 1 },
	{ 730, 104, 4, 1 },



	/* CCI */
	{ 1540, 160, 2, 1 },
	{ 1469, 155, 2, 1 },
	{ 1426, 151, 2, 1 },
	{ 1370, 147, 2, 1 },
	{ 1313, 143, 2, 1 },
	{ 1256, 139, 2, 1 },
	{ 1195, 134, 2, 1 },
	{ 1115, 130, 2, 1 },
	{ 1030, 127, 2, 1 },
	{ 945, 123, 2, 1 },
	{ 881, 120, 2, 1 },
	{ 817, 117, 2, 1 },
	{ 711, 112, 4, 1 },
	{ 668, 111, 4, 1 },
	{ 583, 107, 4, 1 },
	{ 520, 104, 4, 1 },

};


static unsigned int FY_6883Tbl[NR_FREQ * NR_MT_CPU_DVFS][ARRAY_COL_SIZE] = {
	/* Freq, Vproc, post_div, clk_div */
	/* LL */
	{ 2000, 160, 1, 1 },
	{ 1895, 155, 1, 1 },
	{ 1791, 149, 1, 1 },
	{ 1708, 145, 1, 1 },
	{ 1625, 141, 1, 1 },
	{ 1500, 134, 2, 1 },
	{ 1393, 130, 2, 1 },
	{ 1287, 126, 2, 1 },
	{ 1181, 123, 2, 1 },
	{ 1048, 120, 2, 1 },
	{ 968, 115, 2, 1 },
	{ 862, 111, 2, 1 },
	{ 756, 108, 2, 1 },
	{ 703, 106, 4, 1 },
	{ 650, 104, 4, 1 },
	{ 500, 104, 4, 1 },



	/* L */
	{ 2000, 149, 1, 1 },
	{ 1975, 148, 1, 1 },
	{ 1900, 143, 1, 1 },
	{ 1843, 140, 1, 1 },
	{ 1803, 137, 1, 1 },
	{ 1750, 134, 1, 1 },
	{ 1622, 130, 1, 1 },
	{ 1526, 127, 2, 1 },
	{ 1463, 126, 2, 1 },
	{ 1367, 123, 2, 1 },
	{ 1271, 120, 2, 1 },
	{ 1176, 117, 2, 1 },
	{ 1048, 113, 2, 1 },
	{ 921, 110, 2, 1 },
	{ 825, 107, 2, 1 },
	{ 730, 104, 4, 1 },



	/* CCI */
	{ 1540, 160, 2, 1 },
	{ 1469, 155, 2, 1 },
	{ 1426, 151, 2, 1 },
	{ 1370, 147, 2, 1 },
	{ 1313, 143, 2, 1 },
	{ 1256, 139, 2, 1 },
	{ 1195, 134, 2, 1 },
	{ 1115, 130, 2, 1 },
	{ 1030, 127, 2, 1 },
	{ 945, 123, 2, 1 },
	{ 881, 120, 2, 1 },
	{ 817, 117, 2, 1 },
	{ 711, 112, 4, 1 },
	{ 668, 111, 4, 1 },
	{ 583, 107, 4, 1 },
	{ 520, 104, 4, 1 },

};


unsigned int *xrecordTbl[NUM_CPU_LEVEL] = {
	[CPU_LEVEL_0] = &FY_6885Tbl[0][0],
	[CPU_LEVEL_1] = &TB_6885Tbl[0][0],
	[CPU_LEVEL_2] = &FY_6885TTbl[0][0],
	[CPU_LEVEL_3] = &FY_6883Tbl[0][0],
};

#ifdef CCI_MAP_TBL_SUPPORT
static unsigned char CCI_6885Tbl[32][16] = {
	/* Normal CCI Map */
	{ 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{ 7, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{ 7, 8, 8, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
	{ 7, 8, 8, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
	{ 7, 8, 8, 9, 9, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{ 7, 8, 8, 9, 9, 10, 10, 11, 12, 12, 12, 12, 12, 12, 12, 12},
	{ 7, 8, 8, 9, 9, 10, 10, 11, 12, 12, 12, 12, 12, 12, 12, 12},
	{ 7, 8, 8, 9, 9, 10, 10, 11, 12, 13, 13, 13, 13, 13, 13, 13},
	{ 7, 8, 8, 9, 9, 10, 10, 11, 12, 13, 13, 14, 14, 14, 14, 14},
	{ 7, 8, 8, 9, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 7, 8, 8, 9, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 7, 8, 8, 9, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 7, 8, 8, 9, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 7, 8, 8, 9, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 7, 8, 8, 9, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 7, 8, 8, 9, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},

	/* Perf CCI Map */
	{ 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	{ 0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
	{ 0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
	{ 0, 1, 2, 3, 4, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 11},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 12, 12},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},

};

static unsigned char CCI_6885TBTbl[32][16] = {
	/* Normal CCI Map */
	{ 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{ 7, 7, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{ 7, 7, 8, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
	{ 7, 7, 8, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
	{ 7, 7, 8, 8, 9, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{ 7, 7, 8, 8, 9, 10, 10, 11, 12, 12, 12, 12, 12, 12, 12, 12},
	{ 7, 7, 8, 8, 9, 10, 10, 11, 12, 12, 12, 12, 12, 12, 12, 12},
	{ 7, 7, 8, 8, 9, 10, 10, 11, 12, 13, 13, 13, 13, 13, 13, 13},
	{ 7, 7, 8, 8, 9, 10, 10, 11, 12, 13, 13, 14, 14, 14, 14, 14},
	{ 7, 7, 8, 8, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 7, 7, 8, 8, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 7, 7, 8, 8, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 7, 7, 8, 8, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 7, 7, 8, 8, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 7, 7, 8, 8, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 7, 7, 8, 8, 9, 10, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},

	/* Perf CCI Map */
	{ 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	{ 0, 0, 1, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
	{ 0, 0, 1, 3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
	{ 0, 0, 1, 3, 4, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
	{ 0, 0, 1, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7},
	{ 0, 0, 1, 3, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8},
	{ 0, 0, 1, 3, 4, 5, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9},
	{ 0, 0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10},
	{ 0, 0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 11},
	{ 0, 0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 12, 12},
	{ 0, 0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13},
	{ 0, 0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14},
	{ 0, 0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{ 0, 0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{ 0, 0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{ 0, 0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},

};

static unsigned char CCI_6885TTbl[32][16] = {
	/* Normal CCI Map */
	{ 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{ 4, 5, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{ 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
	{ 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
	{ 4, 5, 6, 7, 8, 9, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{ 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 12, 12, 12, 12, 12, 12},
	{ 4, 5, 6, 7, 8, 9, 10, 11, 12, 12, 12, 12, 12, 12, 12, 12},
	{ 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13, 13, 13, 13, 13},
	{ 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 14, 14, 14, 14, 14},
	{ 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 14, 15, 15, 15, 15},

	/* Perf CCI Map */
	{ 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	{ 0, 0, 0, 0, 1, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
	{ 0, 0, 0, 0, 1, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
	{ 0, 0, 0, 0, 1, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
	{ 0, 0, 0, 0, 1, 3, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7},
	{ 0, 0, 0, 0, 1, 3, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8},
	{ 0, 0, 0, 0, 1, 3, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9},
	{ 0, 0, 0, 0, 1, 3, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10},
	{ 0, 0, 0, 0, 1, 3, 6, 7, 8, 9, 10, 11, 11, 11, 11, 11},
	{ 0, 0, 0, 0, 1, 3, 6, 7, 8, 9, 10, 11, 12, 12, 12, 12},
	{ 0, 0, 0, 0, 1, 3, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13},
	{ 0, 0, 0, 0, 1, 3, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14},
	{ 0, 0, 0, 0, 1, 3, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{ 0, 0, 0, 0, 1, 3, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{ 0, 0, 0, 0, 1, 3, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{ 0, 0, 0, 0, 1, 3, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},

};

static unsigned char CCI_6883Tbl[32][16] = {
	/* Normal CCI Map */
	{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{ 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	{ 8, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
	{ 8, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
	{ 8, 9, 9, 9, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11},
	{ 8, 9, 9, 9, 10, 10, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12},
	{ 8, 9, 9, 9, 10, 10, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12},
	{ 8, 9, 9, 9, 10, 10, 11, 12, 12, 13, 13, 13, 13, 13, 13, 13},
	{ 8, 9, 9, 9, 10, 10, 11, 12, 12, 13, 13, 14, 14, 14, 14, 14},
	{ 8, 9, 9, 9, 10, 10, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 8, 9, 9, 9, 10, 10, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 8, 9, 9, 9, 10, 10, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 8, 9, 9, 9, 10, 10, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 8, 9, 9, 9, 10, 10, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 8, 9, 9, 9, 10, 10, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15},
	{ 8, 9, 9, 9, 10, 10, 11, 12, 12, 13, 13, 14, 15, 15, 15, 15},

	/* Perf CCI Map */
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	{ 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
	{ 2, 3, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
	{ 2, 3, 4, 4, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
	{ 2, 3, 4, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
	{ 2, 3, 4, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
	{ 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 9, 9, 9, 9, 9, 9},
	{ 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 10, 10, 10, 10, 10, 10},
	{ 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 10, 11, 11, 11, 11, 11},
	{ 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 12, 12, 12},
	{ 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 13, 13, 13},
	{ 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 14},
	{ 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 15},
	{ 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 15},
	{ 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 15},
	{ 2, 3, 4, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 15},

};

static unsigned char enratio_FYTbl[(NR_FREQ + 1) * IMAX_EN_RATIO_TBL_NUM] = {
	5, 22, 23, 25, 26, 28, 30, 0, 0,
	    0,  0,  0,  0,  0,  0, 0, 0,
	5, 22, 23, 25, 26, 28, 30, 0, 0,
	    0,  0,  0,  0,  0,  0, 0, 0,
};

unsigned char *xrecord_IMAX_Tbl[NUM_CPU_LEVEL] = {
	[CPU_LEVEL_0] = enratio_FYTbl,
	[CPU_LEVEL_1] = enratio_FYTbl,
	[CPU_LEVEL_2] = enratio_FYTbl,
	[CPU_LEVEL_3] = enratio_FYTbl,
};
unsigned char *xrecord_CCI_Tbl[NUM_CPU_LEVEL] = {
	[CPU_LEVEL_0] = &CCI_6885Tbl[0][0],
	[CPU_LEVEL_1] = &CCI_6885TBTbl[0][0],
	[CPU_LEVEL_2] = &CCI_6885TTbl[0][0],
	[CPU_LEVEL_3] = &CCI_6883Tbl[0][0],
};
#endif
