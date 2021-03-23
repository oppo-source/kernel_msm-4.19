/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */
#ifndef _OPPO_FFL_H_
#define _OPPO_FFL_H_

#include <linux/kthread.h>


void oppo_ffl_set(int enable);

void oppo_ffl_setting_thread(struct kthread_work *work);

void oppo_start_ffl_thread(void);

void oppo_stop_ffl_thread(void);

int oppo_ffl_thread_init(void);

void oppo_ffl_thread_exit(void);

int oppo_display_panel_set_ffl(void *buf);
int oppo_display_panel_get_ffl(void *buf);
#endif
