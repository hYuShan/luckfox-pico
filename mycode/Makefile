

OBJECTS = 
MYCODE_DIR = $(shell pwd)
MEDIA_DIR = $(MYCODE_DIR)/../media/

CC              = $(CROSS_COMPILE)gcc
LD              = $(CROSS_COMPILE)ld
AR              = $(CROSS_COMPILE)ar
NM              = $(CROSS_COMPILE)nm
OBJCOPY         = $(CROSS_COMPILE)objcopy
OBJDUMP         = $(CROSS_COMPILE)objdump
READELF         = $(CROSS_COMPILE)readelf
STRIP           = $(CROSS_COMPILE)strip

OBJECTS += jpg_to_fb_RGB565

JPG_TO_FB_CFLAGS = -g -Wall -I$(MEDIA_DIR)/iva/iva/demo/command_line_demo/rockiva_demo/3rdparty/jpeg_turbo/include/

JPG_TO_FB_LD_FLAGS = -L$(MEDIA_DIR)/iva/iva/demo/command_line_demo/rockiva_demo/3rdparty/jpeg_turbo/lib_uclibc/ -lturbojpeg

jpg_to_fb: jpg_to_fb_RGB565.c
	@mkdir -p out
	@echo "$(CC) $^ -o out/$@ $(JPG_TO_FB_CFLAGS) $(JPG_TO_FB_LD_FLAGS)"
	$(CC) $^ -o out/$@ $(JPG_TO_FB_CFLAGS) $(JPG_TO_FB_LD_FLAGS)

$(warning "here add the debug info $(OBJECTS)")
all: $(OBJECTS)

clean:
	@rm -rf out
