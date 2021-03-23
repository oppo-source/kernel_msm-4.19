/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */
#ifndef _OPPO_DC_DIMING_H_
#define _OPPO_DC_DIMING_H_

#include <drm/drm_connector.h>

#include "dsi_panel.h"
#include "dsi_defs.h"
#include "oppo_display_panel_hbm.h"

int sde_connector_update_backlight(struct drm_connector *connector, bool post);

int sde_connector_update_hbm(struct drm_connector *connector);

int oppo_seed_bright_to_alpha(int brightness);

struct dsi_panel_cmd_set * oppo_dsi_update_seed_backlight(struct dsi_panel *panel, int brightness,
				enum dsi_cmd_set_type type);
int oppo_display_panel_get_dim_alpha(void *buf);
int oppo_display_panel_set_dim_alpha(void *buf);
int oppo_display_panel_get_dim_dc_alpha(void *buf);
int oplus_display_get_dimlayer_enable(void *data);
int oplus_display_set_dimlayer_enable(void *data);
int dsi_panel_parse_oppo_dc_config(struct dsi_panel *panel);
#endif /*_OPPO_DC_DIMING_H_*/
