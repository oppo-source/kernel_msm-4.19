ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)
    dtbo-$(CONFIG_ARCH_LITO) += kunlun-20111-mtp-overlay.dtbo \
                    kunlun-20111-v2-mtp-overlay.dtbo \
                    kunlun-20111_t0-mtp-overlay.dtbo \
                    kunlun-20111_t0-v2-mtp-overlay.dtbo \
                    kunlun-20251-mtp-overlay.dtbo \
                    kunlun-20251-v2-mtp-overlay.dtbo \
                    kunlun-20251_t0-mtp-overlay.dtbo \
                    kunlun-20251_t0-v2-mtp-overlay.dtbo

    kunlun-20111-mtp-overlay.dtbo-base := lito.dtb
    kunlun-20111-v2-mtp-overlay.dtbo-base := lito-v2.dtb
    kunlun-20111_t0-mtp-overlay.dtbo-base := lito.dtb
    kunlun-20111_t0-v2-mtp-overlay.dtbo-base := lito-v2.dtb
    kunlun-20251-mtp-overlay.dtbo-base := lito.dtb
    kunlun-20251-v2-mtp-overlay.dtbo-base := lito-v2.dtb
    kunlun-20251_t0-mtp-overlay.dtbo-base := lito.dtb
    kunlun-20251_t0-v2-mtp-overlay.dtbo-base := lito-v2.dtb
else
    dtb-$(CONFIG_ARCH_LITO) += lito-mtp.dtb
endif

always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files    := *.dtb *.dtbo

