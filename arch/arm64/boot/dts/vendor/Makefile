# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2018-2020 Oplus. All rights reserved.
vendor := $(srctree)/$(src)

#ifndef OPLUS_BUG_STABILITY
#/*Fei.Mo@BSP.kernel.Stability, 2019/07/11, Add for spllit dtbo*/
#ifneq "$(wildcard $(vendor)/qcom)" ""
#	subdir-y += qcom
#endif
#else
subdir-y += qcom

subdir-y += oplus
#endif
