/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */
#include "gps_dl_config.h"
#include "gps_dl_hw_dep_api.h"
#include "../gps_dl_hw_priv_util.h"

/* MT6885,MT6889 */
#include "gps_dl_hw_dep_macro.h"

bool gps_dl_hw_dep_set_dsp_on_and_poll_ack(enum gps_dl_link_id_enum link_id)
{
	bool poll_okay = false;

	if (GPS_DATA_LINK_ID0 == link_id) {
		GDL_HW_SET_GPS_ENTRY(
			GPS_RGU_ON_GPS_L1_CR_RGU_GPS_L1_ON, 1);
		GDL_HW_SET_GPS_ENTRY(
			GPS_RGU_ON_GPS_L1_CR_RGU_GPS_L1_SOFT_RST_B, 1);
		GDL_HW_POLL_GPS_ENTRY(
			GPS_CFG_ON_GPS_L1_SLP_PWR_CTL_GPS_L1_SLP_PWR_CTL_CS, 3,
			POLL_DEFAULT, &poll_okay);
	} else if (GPS_DATA_LINK_ID1 == link_id) {
		GDL_HW_SET_GPS_ENTRY(
			GPS_RGU_ON_GPS_L5_CR_RGU_GPS_L5_ON, 1);
		GDL_HW_SET_GPS_ENTRY(
			GPS_RGU_ON_GPS_L5_CR_RGU_GPS_L5_SOFT_RST_B, 1);
		GDL_HW_POLL_GPS_ENTRY(
			GPS_CFG_ON_GPS_L5_SLP_PWR_CTL_GPS_L5_SLP_PWR_CTL_CS, 3,
			POLL_DEFAULT, &poll_okay);
	}
	if (!poll_okay)
		return false;

	if (GPS_DATA_LINK_ID0 == link_id) {
		GDL_HW_SET_GPS_ENTRY(
			GPS_RGU_ON_GPS_L1_MEM_DLY_CTL_RGU_GPSSYS_L1_MEM_ADJ_DLY_EN, 1);
		GDL_HW_SET_GPS_ENTRY(
			GPS_RGU_ON_GPS_L1_DLY_CHAIN_CTL_RGU_GPS_L1_MEM_PDN_DELAY_DUMMY_NUM, 5);
	} else if (GPS_DATA_LINK_ID1 == link_id) {
		GDL_HW_SET_GPS_ENTRY(
			GPS_RGU_ON_GPS_L5_MEM_DLY_CTL_RGU_GPSSYS_L5_MEM_ADJ_DLY_EN, 1);
		GDL_HW_SET_GPS_ENTRY(
			GPS_RGU_ON_GPS_L5_DLY_CHAIN_CTL_RGU_GPS_L5_MEM_PDN_DELAY_DUMMY_NUM, 9);
	}
	return poll_okay;
}

void gps_dl_hw_dep_set_dsp_off(enum gps_dl_link_id_enum link_id)
{
	if (GPS_DATA_LINK_ID0 == link_id) {
		GDL_HW_SET_GPS_ENTRY(
			GPS_RGU_ON_GPS_L1_CR_RGU_GPS_L1_SOFT_RST_B, 0);
		GDL_HW_SET_GPS_ENTRY(
			GPS_RGU_ON_GPS_L1_CR_RGU_GPS_L1_ON, 0);
	} else if (GPS_DATA_LINK_ID1 == link_id) {
		GDL_HW_SET_GPS_ENTRY(
			GPS_RGU_ON_GPS_L5_CR_RGU_GPS_L5_SOFT_RST_B, 0);
		GDL_HW_SET_GPS_ENTRY(
			GPS_RGU_ON_GPS_L5_CR_RGU_GPS_L5_ON, 0);
	}
}

void gps_dl_hw_dep_cfg_dsp_mem(enum dsp_ctrl_enum ctrl)
{
	switch (ctrl) {
	case GPS_L1_DSP_ENTER_DSLEEP:
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPPRAM_PDN_EN_RGU_GPS_L1_PRAM_HWCTL_PDN, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPPRAM_SLP_EN_RGU_GPS_L1_PRAM_HWCTL_SLP, 0x1FF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPXRAM_PDN_EN_RGU_GPS_L1_XRAM_HWCTL_PDN, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPXRAM_SLP_EN_RGU_GPS_L1_XRAM_HWCTL_SLP, 0xF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPYRAM_PDN_EN_RGU_GPS_L1_YRAM_HWCTL_PDN, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPYRAM_SLP_EN_RGU_GPS_L1_YRAM_HWCTL_SLP, 0x1FF);
		break;
	case GPS_L1_DSP_ENTER_DSTOP:
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPPRAM_PDN_EN_RGU_GPS_L1_PRAM_HWCTL_PDN, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPPRAM_SLP_EN_RGU_GPS_L1_PRAM_HWCTL_SLP, 0x1FF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPXRAM_PDN_EN_RGU_GPS_L1_XRAM_HWCTL_PDN, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPXRAM_SLP_EN_RGU_GPS_L1_XRAM_HWCTL_SLP, 0xF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPYRAM_PDN_EN_RGU_GPS_L1_YRAM_HWCTL_PDN, 0x1FF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPYRAM_SLP_EN_RGU_GPS_L1_YRAM_HWCTL_SLP, 0);
		break;
	case GPS_L1_DSP_OFF:
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPPRAM_PDN_EN_RGU_GPS_L1_PRAM_HWCTL_PDN, 0x1FF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPPRAM_SLP_EN_RGU_GPS_L1_PRAM_HWCTL_SLP, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPXRAM_PDN_EN_RGU_GPS_L1_XRAM_HWCTL_PDN, 0xF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPXRAM_SLP_EN_RGU_GPS_L1_XRAM_HWCTL_SLP, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPYRAM_PDN_EN_RGU_GPS_L1_YRAM_HWCTL_PDN, 0x1FF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L1_DSPYRAM_SLP_EN_RGU_GPS_L1_YRAM_HWCTL_SLP, 0);
		break;
	case GPS_L5_DSP_ENTER_DSLEEP:
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPPRAM_PDN_EN_RGU_GPS_L5_PRAM_HWCTL_PDN, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPPRAM_SLP_EN_RGU_GPS_L5_PRAM_HWCTL_SLP, 0x3FF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPXRAM_PDN_EN_RGU_GPS_L5_XRAM_HWCTL_PDN, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPXRAM_SLP_EN_RGU_GPS_L5_XRAM_HWCTL_SLP, 0xF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPYRAM_PDN_EN_RGU_GPS_L5_YRAM_HWCTL_PDN, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPYRAM_SLP_EN_RGU_GPS_L5_YRAM_HWCTL_SLP, 0x3FF);
		break;
	case GPS_L5_DSP_ENTER_DSTOP:
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPPRAM_PDN_EN_RGU_GPS_L5_PRAM_HWCTL_PDN, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPPRAM_SLP_EN_RGU_GPS_L5_PRAM_HWCTL_SLP, 0x3FF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPXRAM_PDN_EN_RGU_GPS_L5_XRAM_HWCTL_PDN, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPXRAM_SLP_EN_RGU_GPS_L5_XRAM_HWCTL_SLP, 0xF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPYRAM_PDN_EN_RGU_GPS_L5_YRAM_HWCTL_PDN, 0x3FF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPYRAM_SLP_EN_RGU_GPS_L5_YRAM_HWCTL_SLP, 0);
		break;
	case GPS_L5_DSP_OFF:
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPPRAM_PDN_EN_RGU_GPS_L5_PRAM_HWCTL_PDN, 0x3FF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPPRAM_SLP_EN_RGU_GPS_L5_PRAM_HWCTL_SLP, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPXRAM_PDN_EN_RGU_GPS_L5_XRAM_HWCTL_PDN, 0xF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPXRAM_SLP_EN_RGU_GPS_L5_XRAM_HWCTL_SLP, 0);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPYRAM_PDN_EN_RGU_GPS_L5_YRAM_HWCTL_PDN, 0x3FF);
		GDL_HW_SET_GPS_ENTRY(GPS_RGU_ON_GPS_L5_DSPYRAM_SLP_EN_RGU_GPS_L5_YRAM_HWCTL_SLP, 0);
		break;
	default:
		/* Do nothing */
		break;
	}
}

