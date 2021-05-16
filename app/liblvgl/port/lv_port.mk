CSRCS += lv_port_disp.c
# CSRCS += lv_port_fs.c.c
# CSRCS += lv_port_indev.c

DEPPATH += --dep-path $(LVGL_DIR)/$(LVGL_DIR_NAME)/port
VPATH += :$(LVGL_DIR)/$(LVGL_DIR_NAME)/port

CFLAGS += "-I$(LVGL_DIR)/$(LVGL_DIR_NAME)/port"
