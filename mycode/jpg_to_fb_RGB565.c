#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <jpeglib.h>

#define RGB565(r, g, b) (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3))

// 读取JPEG文件并解码为RGB格式
int decode_jpeg(const char *filename, unsigned char **out_buffer, int *width, int *height)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *infile;
    JSAMPARRAY buffer;
    int row_stride;

    if ((infile = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    *width = cinfo.output_width;
    *height = cinfo.output_height;
    row_stride = cinfo.output_width * cinfo.output_components;

    *out_buffer = (unsigned char *)malloc(row_stride * cinfo.output_height);
    if (*out_buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }

    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
    unsigned char *pixel = *out_buffer;

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(pixel, buffer[0], row_stride);
        pixel += row_stride;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);

    return 0;
}

// 将RGB图像转换为RGB565格式
void convert_to_rgb565(unsigned char *rgb_data, unsigned short *rgb565_data, int width, int height)
{
    int i, j, idx;
    unsigned char r, g, b;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            idx = (i * width + j) * 3;  // RGB图像每个像素3字节
            r = rgb_data[idx];
            g = rgb_data[idx + 1];
            b = rgb_data[idx + 2];

            rgb565_data[i * width + j] = RGB565(r, g, b);
        }
    }
}

// 发送图像数据到Framebuffer
int write_to_fb(const char *fb_path, unsigned short *rgb565_data, int width, int height)
{
    int fb = open(fb_path, O_RDWR);
    if (fb == -1) {
        perror("Opening framebuffer device");
        return -1;
    }

    struct fb_var_screeninfo vinfo;
    if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Error reading variable information");
        close(fb);
        return -1;
    }

    vinfo.bits_per_pixel = 16;  // RGB565格式
    vinfo.xres = width;
    vinfo.yres = height;

    if (ioctl(fb, FBIOPUT_VSCREENINFO, &vinfo)) {
        perror("Error setting variable information");
        close(fb);
        return -1;
    }

    unsigned short *fb_buffer = (unsigned short *)mmap(NULL, width * height * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if ((intptr_t)fb_buffer == -1) {
        perror("Error mapping framebuffer");
        close(fb);
        return -1;
    }

    memcpy(fb_buffer, rgb565_data, width * height * 2);  // 将RGB565数据拷贝到Framebuffer
    munmap(fb_buffer, width * height * 2);

    close(fb);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: %s <jpg_file> <fb_device_path>\n", argv[0]);
        return 1;
    }

    unsigned char *rgb_data = NULL;
    unsigned short *rgb565_data = NULL;
    int width, height;

    // 解码JPEG文件为RGB格式
    if (decode_jpeg(argv[1], &rgb_data, &width, &height) != 0) {
        return 1;
    }

    // 为RGB565格式数据分配内存
    rgb565_data = (unsigned short *)malloc(width * height * sizeof(unsigned short));
    if (rgb565_data == NULL) {
        free(rgb_data);
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // 转换为RGB565格式
    convert_to_rgb565(rgb_data, rgb565_data, width, height);

    // 将RGB565数据写入Framebuffer设备
    if (write_to_fb(argv[2], rgb565_data, width, height) != 0) {
        free(rgb_data);
        free(rgb565_data);
        return 1;
    }

    printf("JPEG image successfully displayed on framebuffer\n");

    free(rgb_data);
    free(rgb565_data);
    return 0;
}

