// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */
#include "oppo_display_panel_seed.h"
#include "oppo_dsi_support.h"

extern bool oppo_dc_v2_on;

#define PANEL_LOADING_EFFECT_FLAG  100
#define PANEL_LOADING_EFFECT_MODE1 101
#define PANEL_LOADING_EFFECT_MODE2 102
#define PANEL_LOADING_EFFECT_OFF   100

int seed_mode = 0;
DEFINE_MUTEX(oppo_seed_lock);

int oppo_display_get_seed_mode(void)
{
	return seed_mode;
}

int __oppo_display_set_seed(int mode) {
	mutex_lock(&oppo_seed_lock);
	if(mode != seed_mode) {
		seed_mode = mode;
	}
	mutex_unlock(&oppo_seed_lock);
	return 0;
}

int dsi_panel_seed_mode_unlock(struct dsi_panel *panel, int mode)
{
	int rc = 0;

	if (!dsi_panel_initialized(panel))
		return -EINVAL;

	if (oppo_dc_v2_on) {
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SEED_ENTER);

		if (rc) {
			pr_err("[%s] failed to send DSI_CMD_SEED_ENTER cmds, rc=%d\n",
				panel->name, rc);
		}
	} else {
		if(!strcmp(panel->oppo_priv.vendor_name, "AMB655XL08")) {
			int frame_time_us = mult_frac(1000, 1000, panel->cur_mode->timing.refresh_rate);
			dsi_panel_set_backlight(panel, panel->bl_config.bl_level);
			usleep_range(frame_time_us * 2, frame_time_us * 2 + 100);
		}
	}

	switch (mode) {
	case 0:
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SEED_MODE0);
		if (rc) {
			pr_err("[%s] failed to send DSI_CMD_SEED_MODE0 cmds, rc=%d\n",
					panel->name, rc);
		}
		break;
	case 1:
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SEED_MODE1);
		if (rc) {
			pr_err("[%s] failed to send DSI_CMD_SEED_MODE1 cmds, rc=%d\n",
					panel->name, rc);
		}
		break;
	case 2:
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SEED_MODE2);
		if (rc) {
			pr_err("[%s] failed to send DSI_CMD_SEED_MODE2 cmds, rc=%d\n",
					panel->name, rc);
		}
		break;
	case 3:
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SEED_MODE3);
		if (rc) {
			pr_err("[%s] failed to send DSI_CMD_SEED_MODE3 cmds, rc=%d\n",
					panel->name, rc);
		}
		break;
	case 4:
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SEED_MODE4);
		if (rc) {
			pr_err("[%s] failed to send DSI_CMD_SEED_MODE4 cmds, rc=%d\n",
					panel->name, rc);
		}
		break;
	default:
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SEED_OFF);
		if (rc) {
			pr_err("[%s] failed to send DSI_CMD_SEED_OFF cmds, rc=%d\n",
					panel->name, rc);
		}
		pr_err("[%s] seed mode Invalid %d\n",
			panel->name, mode);
	}

	if (!oppo_dc_v2_on) {
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SEED_EXIT);
		if (rc) {
			pr_err("[%s] failed to send DSI_CMD_SEED_EXIT cmds, rc=%d\n",
				panel->name, rc);
		}
	}

	return rc;
}

int dsi_panel_loading_effect_mode_unlock(struct dsi_panel *panel, int mode)
{
	int rc = 0;

	if (!dsi_panel_initialized(panel)) {
		return -EINVAL;
	}

	switch (mode) {
	case PANEL_LOADING_EFFECT_MODE1:
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_LOADING_EFFECT_MODE1);

		if (rc) {
			pr_err("[%s] failed to send PANEL_LOADING_EFFECT_MODE1 cmds, rc=%d\n",
			       panel->name, rc);
		}

		break;

	case PANEL_LOADING_EFFECT_MODE2:
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_LOADING_EFFECT_MODE2);

		if (rc) {
			pr_err("[%s] failed to send PANEL_LOADING_EFFECT_MODE2 cmds, rc=%d\n",
			       panel->name, rc);
		}

		break;

	case PANEL_LOADING_EFFECT_OFF:
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_LOADING_EFFECT_OFF);

		if (rc) {
			pr_err("[%s] failed to send PANEL_LOADING_EFFECT_OFF cmds, rc=%d\n",
			       panel->name, rc);
		}

		break;

	default:
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_LOADING_EFFECT_OFF);

		if (rc) {
			pr_err("[%s] failed to send PANEL_LOADING_EFFECT_OFF cmds, rc=%d\n",
			       panel->name, rc);
		}

		pr_err("[%s] loading effect mode Invalid %d\n",
		       panel->name, mode);
	}

	return rc;
}

int dsi_panel_seed_mode(struct dsi_panel *panel, int mode) {
	int rc = 0;

	if (!panel) {
		pr_err("Invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	if ((!strcmp(panel->oppo_priv.vendor_name, "S6E3HC3")
		|| !strcmp(panel->oppo_priv.vendor_name, "AMB655XL08"))
		&& (mode >= PANEL_LOADING_EFFECT_FLAG)) {
		rc = dsi_panel_loading_effect_mode_unlock(panel, mode);
	} else if (!strcmp(panel->oppo_priv.vendor_name, "ANA6706")
				&& (mode >= PANEL_LOADING_EFFECT_FLAG)) {
		mode = mode - PANEL_LOADING_EFFECT_FLAG;
		rc = dsi_panel_seed_mode_unlock(panel, mode);
		seed_mode = mode;
	} else {
		rc = dsi_panel_seed_mode_unlock(panel, mode);
	}

	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_display_seed_mode(struct dsi_display *display, int mode) {
	int rc = 0;
	if (!display || !display->panel) {
		pr_err("Invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&display->display_lock);

		/* enable the clk vote for CMD mode panels */
	if (display->config.panel_mode == DSI_OP_CMD_MODE) {
		dsi_display_clk_ctrl(display->dsi_clk_handle,
			DSI_CORE_CLK, DSI_CLK_ON);
	}

	rc = dsi_panel_seed_mode(display->panel, mode);
		if (rc) {
			pr_err("[%s] failed to dsi_panel_seed_or_loading_effect, rc=%d\n",
			       display->name, rc);
	}

	if (display->config.panel_mode == DSI_OP_CMD_MODE) {
	rc = dsi_display_clk_ctrl(display->dsi_clk_handle,
				DSI_CORE_CLK, DSI_CLK_OFF);

	}
	mutex_unlock(&display->display_lock);
	return rc;
}

int oppo_dsi_update_seed_mode(void)
{
	struct dsi_display *display = get_main_display();
	int ret = 0;

	if (!display) {
		pr_err("failed for: %s %d\n", __func__, __LINE__);
		return -EINVAL;
	}

	ret = dsi_display_seed_mode(display, seed_mode);

	return ret;
}

int oppo_display_panel_get_seed(void *data)
{
	uint32_t *temp = data;
	printk(KERN_INFO "oppo_display_get_seed = %d\n",seed_mode);

	(*temp) = seed_mode;
	return 0;
}

int oppo_display_panel_set_seed(void *data)
{
	uint32_t *temp_save = data;

	printk(KERN_INFO "%s oppo_display_set_seed = %d\n", __func__, *temp_save);
	seed_mode = *temp_save;

	__oppo_display_set_seed(*temp_save);
	if(get_oppo_display_power_status() == OPPO_DISPLAY_POWER_ON) {
		if(get_main_display() == NULL) {
			printk(KERN_INFO "oppo_display_set_seed and main display is null");
			return -EINVAL;
		}
		dsi_display_seed_mode(get_main_display(), seed_mode);
	} else {
		printk(KERN_ERR	 "%s oppo_display_set_seed = %d, but now display panel status is not on\n", __func__, *temp_save);
	}

	return 0;
}
