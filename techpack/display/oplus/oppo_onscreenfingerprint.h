/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */
#ifndef _OPPO_ONSCREENFINGERPRINT_H_
#define _OPPO_ONSCREENFINGERPRINT_H_

#include <drm/drm_crtc.h>
#include "dsi_panel.h"
#include "dsi_defs.h"
#include "dsi_parser.h"
#include "sde_encoder_phys.h"


#define FFL_FP_LEVEL 150
#define APOLLO_BACKLIGHT_LENS 8190

struct backlight_apollo_maplist
{
	//uint32_t p_backlight_apollo_vlist[APOLLO_BACKLIGHT_LENS];
	short p_backlight_apollo_panel_list[APOLLO_BACKLIGHT_LENS];
};

struct backlight_apollo_vmaplist
{
	short p_backlight_apollo_vlist[APOLLO_BACKLIGHT_LENS];
};

int oppo_get_panel_brightness(void);

int dsi_panel_parse_oppo_fod_config(struct dsi_panel *panel);

int dsi_panel_parse_oppo_config(struct dsi_panel *panel);

int dsi_panel_parse_oppo_mode_config(struct dsi_display_mode *mode, struct dsi_parser_utils *utils);

bool sde_crtc_get_dimlayer_mode(struct drm_crtc_state *crtc_state);

bool sde_crtc_get_fingerprint_mode(struct drm_crtc_state *crtc_state);

bool sde_crtc_get_fingerprint_pressed(struct drm_crtc_state *crtc_state);

int sde_crtc_set_onscreenfinger_defer_sync(struct drm_crtc_state *crtc_state, bool defer_sync);

int sde_crtc_config_fingerprint_dim_layer(struct drm_crtc_state *crtc_state, int stage);

bool is_skip_pcc(struct drm_crtc *crtc);

bool sde_cp_crtc_update_pcc(struct drm_crtc *crtc);

bool _sde_encoder_setup_dither_for_onscreenfingerprint(struct sde_encoder_phys *phys,
						  void *dither_cfg, int len, struct sde_hw_pingpong *hw_pp);

int sde_plane_check_fingerprint_layer(const struct drm_plane_state *drm_state);
int oplus_display_set_dimlayer_hbm(void *data);
int oplus_display_get_dimlayer_hbm(void *data);
int oplus_apollo_backlight_list_alloc(void);
int oplus_display_set_apollo_backlight_maplist(void *data);
int oplus_display_set_apollo_backlight_vmaplist(void *data);
#endif /*_OPPO_ONSCREENFINGERPRINT_H_*/
