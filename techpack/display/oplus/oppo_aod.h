/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */
#ifndef _OPPO_AOD_H_
#define _OPPO_AOD_H_

#include "dsi_display.h"

int dsi_display_aod_on(struct dsi_display *display);

int dsi_display_aod_off(struct dsi_display *display);

int oppo_update_aod_light_mode_unlock(struct dsi_panel *panel);

int oppo_update_aod_light_mode(void);

int oppo_panel_set_aod_light_mode(void *buf);
int oppo_panel_get_aod_light_mode(void *buf);
int __oppo_display_set_aod_light_mode(int mode);
#endif
