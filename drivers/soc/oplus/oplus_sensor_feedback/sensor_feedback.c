// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */


#define pr_fmt(fmt) "<sensor_feedback>" fmt

#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/soc/qcom/smem.h>
#include <linux/platform_device.h>
#include <linux/of.h>


#include "sensor_feedback.h"
#include <soc/oplus/system/kernel_fb.h>

#define SENSOR_DEVICE_TYPE      "10002"
#define SENSOR_POWER_TYPE       "10003"
#define SENSOR_STABILITY_TYPE   "10004"
#define SENSOR_PFMC_TYPE        "10005"
#define SENSOR_MEMORY_TYPE      "10006"


#define SENSOR_DEBUG_DEVICE_TYPE      "20002"
#define SENSOR_DEBUG_POWER_TYPE       "20003"
#define SENSOR_DEBUG_STABILITY_TYPE   "20004"
#define SENSOR_DEBUG_PFMC_TYPE        "20005"
#define SENSOR_DEBUG_MEMORY_TYPE      "20006"


static struct sensor_fb_cxt *g_sensor_fb_cxt = NULL;

/*fb_field :maxlen 19*/
struct sensor_fb_conf g_fb_conf[] = {
	{PS_INIT_FAIL_ID, "device_ps_init_fail", SENSOR_DEVICE_TYPE},
	{PS_I2C_ERR_ID, "device_ps_i2c_err", SENSOR_DEVICE_TYPE},
	{PS_ALLOC_FAIL_ID, "device_ps_alloc_fail", SENSOR_DEVICE_TYPE},
	{PS_ESD_REST_ID, "device_ps_esd_reset", SENSOR_DEVICE_TYPE},
	{PS_NO_INTERRUPT_ID, "device_ps_no_rpt", SENSOR_DEVICE_TYPE},
	{PS_FIRST_REPORT_DELAY_COUNT_ID, "device_ps_rpt_delay", SENSOR_DEVICE_TYPE},
	{PS_ORIGIN_DATA_TO_ZERO_ID, "device_ps_to_zero", SENSOR_DEVICE_TYPE},

	{ALS_INIT_FAIL_ID, "device_als_init_fail", SENSOR_DEVICE_TYPE},
	{ALS_I2C_ERR_ID, "device_als_i2c_err", SENSOR_DEVICE_TYPE},
	{ALS_ALLOC_FAIL_ID, "device_als_alloc_fail", SENSOR_DEVICE_TYPE},
	{ALS_ESD_REST_ID, "device_als_esd_reset", SENSOR_DEVICE_TYPE},
	{ALS_NO_INTERRUPT_ID, "device_als_no_rpt", SENSOR_DEVICE_TYPE},
	{ALS_FIRST_REPORT_DELAY_COUNT_ID, "device_als_rpt_delay", SENSOR_DEVICE_TYPE},
	{ALS_ORIGIN_DATA_TO_ZERO_ID, "device_als_to_zero", SENSOR_DEVICE_TYPE},

	{ACCEL_INIT_FAIL_ID, "device_acc_init_fail", SENSOR_DEVICE_TYPE},
	{ACCEL_I2C_ERR_ID, "device_acc_i2c_err", SENSOR_DEVICE_TYPE},
	{ACCEL_ALLOC_FAIL_ID, "device_acc_alloc_fail", SENSOR_DEVICE_TYPE},
	{ACCEL_ESD_REST_ID, "device_acc_esd_reset", SENSOR_DEVICE_TYPE},
	{ACCEL_NO_INTERRUPT_ID, "device_acc_no_rpt", SENSOR_DEVICE_TYPE},
	{ACCEL_FIRST_REPORT_DELAY_COUNT_ID, "device_acc_rpt_delay", SENSOR_DEVICE_TYPE},
	{ACCEL_ORIGIN_DATA_TO_ZERO_ID, "device_acc_to_zero", SENSOR_DEVICE_TYPE},

	{GYRO_INIT_FAIL_ID, "device_gyro_init_fail", SENSOR_DEVICE_TYPE},
	{GYRO_I2C_ERR_ID, "device_gyro_i2c_err", SENSOR_DEVICE_TYPE},
	{GYRO_ALLOC_FAIL_ID, "device_gyro_alloc_fail", SENSOR_DEVICE_TYPE},
	{GYRO_ESD_REST_ID, "device_gyro_esd_reset", SENSOR_DEVICE_TYPE},
	{GYRO_NO_INTERRUPT_ID, "device_gyro_no_rpt", SENSOR_DEVICE_TYPE},
	{GYRO_FIRST_REPORT_DELAY_COUNT_ID, "device_gyro_rpt_delay", SENSOR_DEVICE_TYPE},
	{GYRO_ORIGIN_DATA_TO_ZERO_ID, "device_gyro_to_zero", SENSOR_DEVICE_TYPE},

	{MAG_INIT_FAIL_ID, "device_mag_init_fail", SENSOR_DEVICE_TYPE},
	{MAG_I2C_ERR_ID, "device_mag_i2c_err", SENSOR_DEVICE_TYPE},
	{MAG_ALLOC_FAIL_ID, "device_mag_alloc_fail", SENSOR_DEVICE_TYPE},
	{MAG_ESD_REST_ID, "device_mag_esd_reset", SENSOR_DEVICE_TYPE},
	{MAG_NO_INTERRUPT_ID, "device_mag_no_rpt", SENSOR_DEVICE_TYPE},
	{MAG_FIRST_REPORT_DELAY_COUNT_ID, "device_mag_rpt_delay", SENSOR_DEVICE_TYPE},
	{MAG_ORIGIN_DATA_TO_ZERO_ID, "device_mag_to_zero", SENSOR_DEVICE_TYPE},

	{POWER_SENSOR_INFO_ID, "debug_power_sns_info", SENSOR_DEBUG_POWER_TYPE},
	{POWER_ACCEL_INFO_ID, "debug_power_acc_info", SENSOR_DEBUG_POWER_TYPE},
	{POWER_GYRO_INFO_ID, "debug_power_gyro_info", SENSOR_DEBUG_POWER_TYPE},
	{POWER_MAG_INFO_ID, "debug_power_mag_info", SENSOR_DEBUG_POWER_TYPE},
	{POWER_PROXIMITY_INFO_ID, "debug_power_prox_info", SENSOR_DEBUG_POWER_TYPE},
	{POWER_LIGHT_INFO_ID, "debug_power_light_info", SENSOR_DEBUG_POWER_TYPE},
	{POWER_WISE_LIGHT_INFO_ID, "debug_power_wiseligt_info", SENSOR_DEBUG_POWER_TYPE},
	{POWER_WAKE_UP_RATE_ID, "debug_power_wakeup_rate", SENSOR_DEBUG_POWER_TYPE},
	{POWER_ADSP_SLEEP_RATIO_ID, "power_adsp_sleep_ratio", SENSOR_POWER_TYPE},

	{ALAILABLE_SENSOR_LIST_ID, "available_sensor_list", SENSOR_DEBUG_DEVICE_TYPE}

};


static ssize_t adsp_notify_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_fb_cxt *sensor_fb_cxt = g_sensor_fb_cxt;
	uint16_t adsp_event_counts = 0;
	spin_lock(&sensor_fb_cxt->rw_lock);
	adsp_event_counts = sensor_fb_cxt->adsp_event_counts;
	spin_unlock(&sensor_fb_cxt->rw_lock);
	pr_info(" adsp_value = %d\n", adsp_event_counts);

	return snprintf(buf, PAGE_SIZE, "%d\n", adsp_event_counts);
}

static ssize_t adsp_notify_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct sensor_fb_cxt *sensor_fb_cxt = g_sensor_fb_cxt;
	uint16_t adsp_event_counts = 0;
	uint16_t node_type = 0;
	int err = 0;

	err = sscanf(buf, "%hu %hu", &node_type, &adsp_event_counts);

	if (err < 0) {
		pr_err("adsp_notify_store error: err = %d\n", err);
		return err;
	}

	spin_lock(&sensor_fb_cxt->rw_lock);
	sensor_fb_cxt->adsp_event_counts = adsp_event_counts;
	sensor_fb_cxt->node_type = node_type;
	spin_unlock(&sensor_fb_cxt->rw_lock);
	pr_info("adsp_notify_store adsp_value = %d, node_type=%d\n", adsp_event_counts,
		node_type);

	set_bit(THREAD_WAKEUP, (unsigned long *)&sensor_fb_cxt->wakeup_flag);
	/*wake_up_interruptible(&sensor_fb_cxt->wq);*/
	wake_up(&sensor_fb_cxt->wq);

	return count;
}


static ssize_t hal_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t hal_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	pr_info("hal_info_store count = %d\n", count);
	return count;
}

static ssize_t test_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	pr_info("test_id_show\n");
	return 0;
}

static ssize_t test_id_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct sensor_fb_cxt *sensor_fb_cxt = g_sensor_fb_cxt;
	uint16_t adsp_event_counts = 0;
	uint16_t node_type = 0;
	uint16_t event_id = 0;
	uint16_t event_data = 0;
	int err = 0;

	err = sscanf(buf, "%hu %hu %hu %hu", &node_type, &adsp_event_counts, &event_id,
			&event_data);

	if (err < 0) {
		pr_err("test_id_store error: err = %d\n", err);
		return err;
	}

	spin_lock(&sensor_fb_cxt->rw_lock);
	sensor_fb_cxt->adsp_event_counts = adsp_event_counts;
	sensor_fb_cxt->node_type = node_type;
	spin_unlock(&sensor_fb_cxt->rw_lock);

	sensor_fb_cxt->fb_smem.event[0].event_id = event_id;
	sensor_fb_cxt->fb_smem.event[0].count = event_data;

	pr_info("test_id_store adsp_value = %d, node_type=%d \n", adsp_event_counts,
		node_type);
	pr_info("test_id_store event_id = %d, event_data=%d \n", event_id, event_data);


	set_bit(THREAD_WAKEUP, (unsigned long *)&sensor_fb_cxt->wakeup_flag);
	/*wake_up_interruptible(&sensor_fb_cxt->wq);*/
	wake_up(&sensor_fb_cxt->wq);

	return count;
}

static ssize_t sensor_list_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sensor_fb_cxt *sensor_fb_cxt = g_sensor_fb_cxt;
	uint16_t sensor_list[2] = {0x00};
	spin_lock(&sensor_fb_cxt->rw_lock);
	sensor_list[0] = sensor_fb_cxt->sensor_list[0];
	sensor_list[1] = sensor_fb_cxt->sensor_list[1];
	spin_unlock(&sensor_fb_cxt->rw_lock);
	pr_info("phy = 0x%x, virt = 0x%x\n", sensor_list[0], sensor_list[1]);

	return snprintf(buf, PAGE_SIZE, "phy = 0x%x, virt = 0x%x\n", sensor_list[0], sensor_list[1]);

}


static ssize_t sensor_list_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	pr_info("sensor_list_store\n");
	return count;
}



DEVICE_ATTR(adsp_notify, 0644, adsp_notify_show, adsp_notify_store);
DEVICE_ATTR(hal_info, 0644, hal_info_show, hal_info_store);
DEVICE_ATTR(test_id, 0644, test_id_show, test_id_store);
DEVICE_ATTR(sensor_list, 0644, sensor_list_show, sensor_list_store);



static struct attribute *sensor_feedback_attributes[] = {
	&dev_attr_adsp_notify.attr,
	&dev_attr_hal_info.attr,
	&dev_attr_test_id.attr,
	&dev_attr_sensor_list.attr,
	NULL
};



static struct attribute_group sensor_feedback_attribute_group = {
	.attrs = sensor_feedback_attributes
};

#define SMEM_SENSOR_FEEDBACK (128)
static int read_data_from_share_mem(struct sensor_fb_cxt *sensor_fb_cxt)
{
	int ret = 0;
	size_t smem_size = 0;
	void *smem_addr = NULL;
	struct fb_event_smem *fb_event = NULL;
	smem_addr = qcom_smem_get(QCOM_SMEM_HOST_ANY,
			SMEM_SENSOR_FEEDBACK,
			&smem_size);

	if (IS_ERR(smem_addr)) {
		pr_err("unable to acquire smem SMEM_SENSOR_FEEDBACK entry\n");
		return -1;
	}

	fb_event = (struct fb_event_smem *)smem_addr;

	if (fb_event == ERR_PTR(-EPROBE_DEFER)) {
		fb_event = NULL;
		return -2;
	}

	memcpy((void *)&sensor_fb_cxt->fb_smem, (void *)fb_event, smem_size);
	return ret;
}


static int find_event_id(int16_t event_id)
{
	int len = sizeof(g_fb_conf) / sizeof(g_fb_conf[0]);
	int ret = -1;
	int index = 0;

	for (index = 0; index < len; index++) {
		if (g_fb_conf[index].event_id == event_id) {
			ret = index;
		}
	}

	return ret;
}
/*
static unsigned int BKDRHash(char *str, unsigned int len)
{
    unsigned int seed = 131;
        // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
    unsigned int i    = 0;

    if (str == NULL) {
        return 0;
    }

    for(i = 0; i < len; str++, i++) {
        hash = (hash * seed) + (*str);
    }

    return hash;
}*/

int procce_special_event_id(unsigned short event_id, int count ,struct sensor_fb_cxt *sensor_fb_cxt){
	int ret = 0;
	if (event_id == ALAILABLE_SENSOR_LIST_ID) {
		sensor_fb_cxt->sensor_list[0] = (uint32_t)sensor_fb_cxt->fb_smem.event[count].buff[0];
		sensor_fb_cxt->sensor_list[1] = (uint32_t)sensor_fb_cxt->fb_smem.event[count].buff[1];
		pr_info("sensor_list virt_sns = 0x%x, phy_sns = 0x%x\n", sensor_fb_cxt->sensor_list[0], sensor_fb_cxt->sensor_list[1]);
		ret = 1;
	}
	return ret;
}


static int parse_shr_info(struct sensor_fb_cxt *sensor_fb_cxt)
{
	int ret = 0;
	int count = 0;
	uint16_t event_id = 0;
	int index = 0;
	unsigned char payload[1024] = {0x00};
	int fb_len = 0;
	unsigned char detail_buff[128] = {0x00};

	for (count = 0; count < sensor_fb_cxt->adsp_event_counts; count ++) {
		event_id = sensor_fb_cxt->fb_smem.event[count].event_id;
		pr_info("event_id =%d, count =%d\n", event_id, count);

		index = find_event_id(event_id);

		if (index == -1) {
			pr_info("event_id =%d, count =%d\n", event_id, count);
			continue;
		}
		ret = procce_special_event_id(event_id, count, sensor_fb_cxt);
		if (ret == 1) {
			continue;
		}
		memset(payload, 0, sizeof(payload));
		memset(detail_buff, 0, sizeof(detail_buff));
		snprintf(detail_buff, sizeof(detail_buff), "%d %d %d",
			sensor_fb_cxt->fb_smem.event[count].buff[0],
			sensor_fb_cxt->fb_smem.event[count].buff[1],
			sensor_fb_cxt->fb_smem.event[count].buff[2]);
		fb_len += scnprintf(payload, sizeof(payload),
				"NULL$$EventField@@%s$$FieldData@@%d$$detailData@@%s",
				g_fb_conf[index].fb_field,
				sensor_fb_cxt->fb_smem.event[count].count,
				detail_buff);
		pr_info("payload =%s\n", payload);
		oplus_kevent_fb(FB_SENSOR, g_fb_conf[index].fb_event_id, payload);
	}

	return ret;
}


static int sensor_report_thread(void *arg)
{
	int ret = 0;
	struct sensor_fb_cxt *sensor_fb_cxt = (struct sensor_fb_cxt *)arg;
	pr_info("sensor_feedback: sensor_report_thread step1!\n");

	while (!kthread_should_stop()) {
		wait_event_interruptible(sensor_fb_cxt->wq, test_bit(THREAD_WAKEUP,
				(unsigned long *)&sensor_fb_cxt->wakeup_flag));

		clear_bit(THREAD_WAKEUP, (unsigned long *)&sensor_fb_cxt->wakeup_flag);
		set_bit(THREAD_SLEEP, (unsigned long *)&sensor_fb_cxt->wakeup_flag);

		if (sensor_fb_cxt->node_type == 0) {
			ret = read_data_from_share_mem(sensor_fb_cxt);

		} else {
			pr_info("sensor_feedback test from node \n");
		}

		ret = parse_shr_info(sensor_fb_cxt);
	}

	memset((void *)&sensor_fb_cxt->fb_smem, 0, sizeof(struct fb_event_smem));
	pr_info("sensor_feedback ret =%s\n", ret);
	return ret;
}

#ifdef CONFIG_FB
#ifdef CONFIG_DRM_MSM
static int fb_notifier_callback(struct notifier_block *nb,
		unsigned long event, void *data)
{
	int blank;
	struct msm_drm_notifier *evdata = data;
	struct sensor_fb_cxt *sns_cxt = container_of(nb, struct sensor_fb_cxt, fb_notif);
	struct timespec now_time;
	if (!sns_cxt) {
		return 0;
	}
	if (!evdata || (evdata->id != 0)){
		return 0;
	}
	//if(event == MSM_DRM_EARLY_EVENT_BLANK || event == MSM_DRM_EVENT_BLANK)
	if (event == MSM_DRM_EARLY_EVENT_BLANK) {
		blank = *(int *)(evdata->data);
		if (blank == MSM_DRM_BLANK_UNBLANK) { //resume
			now_time = current_kernel_time();
			sns_cxt->end_time = (now_time.tv_sec * 1000 + now_time.tv_nsec / 1000000);
			sns_cxt->sleep_time = (sns_cxt->end_time- sns_cxt->start_time);
			pr_info("%s: ap_sleep time: %ld ms\n", __func__, sns_cxt->sleep_time);
		} else if (blank == MSM_DRM_BLANK_POWERDOWN) { //suspend
			now_time = current_kernel_time();
			sns_cxt->start_time = (now_time.tv_sec * 1000 + now_time.tv_nsec / 1000000);
		} else {
			pr_info("%s: receives wrong data EARLY_BLANK:%d\n", __func__, blank);
		}
	}
	return 0;
}
#else
static int fb_notifier_callback(struct notifier_block *nb,
		unsigned long event, void *data)
{
	int blank;
	struct fb_event *evdata = data;
	struct sensor_fb_cxt *sns_cxt = container_of(nb, struct sensor_fb_cxt, fb_notif);
	struct timespec now_time;
	if (!sns_cxt) {
		return 0;
	}
	if (evdata && evdata->data) {
		//if(event == FB_EARLY_EVENT_BLANK || event == FB_EVENT_BLANK)
		if (event == FB_EVENT_BLANK) {
			blank = *(int *)evdata->data;
			if (blank == FB_BLANK_UNBLANK) { //resume
				now_time = current_kernel_time();
				sns_cxt->end_time = (now_time.tv_sec * 1000 + now_time.tv_nsec / 1000000);
				sns_cxt->sleep_time = (sns_cxt->end_time- sns_cxt->start_time);
				pr_info("%s: ap_sleep time: %ld ms\n", __func__, sns_cxt->sleep_time);
			} else if (blank == FB_BLANK_POWERDOWN) { //suspend
				now_time = current_kernel_time();
				sns_cxt->start_time = (now_time.tv_sec * 1000 + now_time.tv_nsec / 1000000);
			} else {
				pr_info("%s: receives wrong data EARLY_BLANK:%d\n", __func__, blank);
			}
		}
	}
	return 0;
}
#endif /* CONFIG_DRM_MSM */
#endif /* CONFIG_FB */



static int sensor_feedback_probe(struct platform_device *pdev)
{
	int err = 0;
	struct sensor_fb_cxt *sensor_fb_cxt = NULL;

	sensor_fb_cxt = kzalloc(sizeof(struct sensor_fb_cxt), GFP_KERNEL);

	if (sensor_fb_cxt == NULL) {
		pr_err("kzalloc g_sensor_fb_cxt failed\n");
		err = -ENOMEM;
		goto alloc_sensor_fb_failed;
	}

	g_sensor_fb_cxt = sensor_fb_cxt;


	spin_lock_init(&sensor_fb_cxt->rw_lock);
	init_waitqueue_head(&sensor_fb_cxt->wq);

	sensor_fb_cxt->sensor_fb_dev = pdev;
	err = sysfs_create_group(&sensor_fb_cxt->sensor_fb_dev->dev.kobj,
			&sensor_feedback_attribute_group);

	if (err < 0) {
		pr_err("unable to create sensor_feedback_attribute_group file err=%d\n", err);
		goto sysfs_create_failed;
	}

	kobject_uevent(&sensor_fb_cxt->sensor_fb_dev->dev.kobj, KOBJ_ADD);

	#if defined(CONFIG_DRM_MSM)
	sensor_fb_cxt->fb_notif.notifier_call = fb_notifier_callback;
	err = msm_drm_register_client(&sensor_fb_cxt->fb_notif);
	if (err) {
		pr_err("Unable to register fb_notifier: %d\n", err);
	}
	#elif defined(CONFIG_FB)
	sensor_fb_cxt->fb_notif.notifier_call = fb_notifier_callback;
	err = fb_register_client(&sensor_fb_cxt->fb_notif);
	if (err) {
		pr_err("Unable to register fb_notifier: %d\n", err);
	}
	#endif/*CONFIG_FB*/


	init_waitqueue_head(&sensor_fb_cxt->wq);

	set_bit(THREAD_SLEEP, (unsigned long *)&sensor_fb_cxt->wakeup_flag);

	sensor_fb_cxt->report_task = kthread_create(sensor_report_thread,
			(void *)sensor_fb_cxt,
			"sensor_feedback_task");

	if (IS_ERR(sensor_fb_cxt->report_task)) {
		err = PTR_ERR(sensor_fb_cxt->report_task);
		goto create_task_failed;
	}

	platform_set_drvdata(pdev, sensor_fb_cxt);
	wake_up_process(sensor_fb_cxt->report_task);

	pr_info("sensor_feedback_init success\n");
	return 0;
create_task_failed:
	sysfs_remove_group(&sensor_fb_cxt->sensor_fb_dev->dev.kobj,
		&sensor_feedback_attribute_group);
sysfs_create_failed:
	kfree(sensor_fb_cxt);
	g_sensor_fb_cxt = NULL;
alloc_sensor_fb_failed:
	return err;
}


static int sensor_feedback_remove(struct platform_device *pdev)
{
	struct sensor_fb_cxt *sensor_fb_cxt = g_sensor_fb_cxt;
	sysfs_remove_group(&sensor_fb_cxt->sensor_fb_dev->dev.kobj,
		&sensor_feedback_attribute_group);
	kfree(sensor_fb_cxt);
	g_sensor_fb_cxt = NULL;
	return 0;
}

static const struct of_device_id of_drv_match[] = {
	{ .compatible = "oplus,sensor-feedback"},
	{},
};
MODULE_DEVICE_TABLE(of, of_drv_match);

static struct platform_driver _driver = {
	.probe      = sensor_feedback_probe,
	.remove     = sensor_feedback_remove,
	.driver     = {
		.name       = "sensor_feedback",
		.of_match_table = of_drv_match,
	},
};

static int __init sensor_feedback_init(void)
{
	pr_info("sensor_feedback_init call\n");

	platform_driver_register(&_driver);
	return 0;
}

core_initcall(sensor_feedback_init);


MODULE_AUTHOR("JangHua.Tang");
MODULE_LICENSE("GPL v2");

