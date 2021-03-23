// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */
#ifndef _OPPO_DISPLAY_PANEL_COMMON_H_
#define _OPPO_DISPLAY_PANEL_COMMON_H_

#include <linux/err.h>
#include "dsi_display.h"
#include "dsi_panel.h"
#include "dsi_ctrl.h"
#include "dsi_ctrl_hw.h"
#include "dsi_drm.h"
#include "dsi_clk.h"
#include "dsi_pwr.h"
#include "sde_dbg.h"

#define PANEL_REG_MAX_LENS 28
#define PANEL_TX_MAX_BUF 112

struct panel_id
{
	uint32_t DA;
	uint32_t DB;
	uint32_t DC;
};

struct panel_info
{
	char version[32];
	char manufacture[32];
};

struct panel_serial_number
{
	uint32_t date;		 /*year:month:day:hour  8bit uint*/
	uint32_t precise_time; /* minute:second:reserved:reserved 8bit uint*/
};

struct display_timing_info {
	uint32_t h_active;
	uint32_t v_active;
	uint32_t refresh_rate;
	uint32_t clk_rate_hz_h32;  /* the high 32bit of clk_rate_hz */
	uint32_t clk_rate_hz_l32;  /* the low 32bit of clk_rate_hz */
};

enum {
	NONE_TYPE = 0,
	LCM_DC_MODE_TYPE,
	LCM_BRIGHTNESS_TYPE,
	MAX_INFO_TYPE,
};

enum {
	REG_WRITE = 0,
	REG_READ,
	REG_X,
};

struct panel_reg_get {
	uint32_t reg_rw[PANEL_REG_MAX_LENS];
	uint32_t lens; /*reg_rw lens, lens represent for u32 to user space*/
};

struct panel_reg_rw {
	uint32_t rw_flags; /*1 for read, 0 for write*/
	uint32_t cmd;
	uint32_t lens;     /*lens represent for u8 to kernel space*/
	uint32_t value[PANEL_REG_MAX_LENS]; /*for read, value is empty, just user get function for read the value*/
};

int oppo_display_panel_get_id(void *buf);
int oppo_display_panel_get_max_brightness(void *buf);
int oppo_display_panel_set_max_brightness(void *buf);
int oppo_display_panel_get_vendor(void *buf);
int oppo_display_panel_get_ccd_check(void *buf);
int oppo_display_panel_get_serial_number(void *buf);
int oplus_display_set_audio_ready(void *data);
int oplus_display_dump_info(void *data);
int oplus_display_get_panel_dsc(void *data);
int oplus_display_get_closebl_flag(void *data);
int oplus_display_set_closebl_flag(void *data);
int oplus_display_get_panel_reg(void *data);
int oplus_display_set_panel_reg(void *data);
int oplus_display_notify_panel_blank(void *data);
int oplus_display_set_spr(void *data);
int oplus_display_get_spr(void *data);
int oplus_display_get_roundcorner(void *data);
int oplus_display_set_dynamic_osc_clock(void *data);
int oplus_display_get_dynamic_osc_clock(void *data);
int __oplus_display_set_mca(int mode);
int dsi_display_set_mca(struct dsi_display *display);
int oplus_display_get_softiris_color_status(void *data);
#endif /*_OPPO_DISPLAY_PANEL_COMMON_H_*/
