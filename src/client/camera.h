
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include "../common/utils.h"

typedef struct buffer Buffer;

struct buffer {
    void *start;
    size_t length;
};

const char* camera_name = "/dev/video0";

int open_camera() {
    int fd = open(camera_name, O_RDWR);
    if (fd < 0) {
        puts("Error opening camera");
    }

    return fd;
}

int query_capabilities(int fd) {
    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        puts("VIDIOC_QUERYCAP");
        return FAIL;
    }

    printf("Driver Caps:\n"
           "  Driver: \"%s\"\n"
           "  Card: \"%s\"\n"
           "  Bus: \"%s\"\n"
           "  Version: %d\n"
           "  Capabilities: %08x\n",
           cap.driver,
           cap.card,
           cap.bus_info,
           cap.version,
           cap.capabilities);
    
    return SUCCESS;
}

int set_format(int fd, int width, int height, __u32 pixel_format) {
    struct v4l2_format format;
    memset(&format, 0, sizeof(format));

    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = width;
    format.fmt.pix.height = height;
    format.fmt.pix.pixelformat = pixel_format;
    format.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &format) < 0) {
        puts("VIDIOC_S_FMT");
        return FAIL;
    }

    printf("Set format: %dx%d, pixel format = %c%c%c%c\n",
       format.fmt.pix.width,
       format.fmt.pix.height,
       format.fmt.pix.pixelformat & 0xff,
       (format.fmt.pix.pixelformat >> 8) & 0xff,
       (format.fmt.pix.pixelformat >> 16) & 0xff,
       (format.fmt.pix.pixelformat >> 24) & 0xff);

    return SUCCESS;
}

int init_map_buffer(int fd, Buffer* buffer) {

    struct v4l2_requestbuffers request;
    memset(&request, 0, sizeof(request));
    request.count = 1;
    request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    request.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &request) < 0) {
        puts("VIDIOC_REQBUFS");
        return FAIL;
    }

    struct v4l2_buffer v_buffer;
    memset(&v_buffer, 0, sizeof(v_buffer));
    v_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v_buffer.memory = V4L2_MEMORY_MMAP;
    v_buffer.index = 0;

    if (ioctl(fd, VIDIOC_QUERYBUF, &v_buffer) < 0) {
        puts("VIDIOC_QUERYBUF");
        return FAIL;
    }

    buffer->length = v_buffer.length;
    buffer->start = mmap(NULL, v_buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, v_buffer.m.offset);
    
    if (buffer->start == MAP_FAILED) {
        puts("map error");
        return FAIL;
    }

    return SUCCESS;
}

int capture_frame(int fd, Buffer* buffer, const char* filename) {
    struct v4l2_buffer v_buffer;
    memset(&v_buffer, 0, sizeof(v_buffer));
    v_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v_buffer.memory = V4L2_MEMORY_MMAP;
    v_buffer.index = 0;
    
    if (ioctl(fd, VIDIOC_QBUF, &v_buffer) < 0) {
        puts("VIDIOC_QBUF");
        return FAIL;
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        puts("VIDIOC_STREAMON");
        return FAIL;
    }

    if (ioctl(fd, VIDIOC_DQBUF, &v_buffer) < 0) {
        puts("VIDIOC_DQBUF");
        return FAIL;
    }

    FILE *output = fopen(filename, "wb");
    if (!output) {
        puts("fopen");
        return FAIL;
    }

    fwrite(buffer->start, v_buffer.bytesused, 1, output);
    fclose(output);

    printf("Saved frame to %s (%d bytes)\n", filename, v_buffer.bytesused);

    if (ioctl(fd, VIDIOC_STREAMOFF, &type) < 0) {
        puts("VIDIOC_STREAMOFF");
        return FAIL;
    }

    return SUCCESS;
}

void cleanup(Buffer* buffer, int fd) {
    munmap(buffer->start, buffer->length);
    close(fd);
}

int execute() {
    Buffer buffer;

    int fd = open_camera();

    if (fd < 0)
        return FAIL;

    if (query_capabilities(fd) < 0)
        return FAIL;

    if (set_format(fd, 640, 480, V4L2_PIX_FMT_YUYV) < 0)
        return FAIL;

    if (init_map_buffer(fd, &buffer) < 0)
        return FAIL;

    if (capture_frame(fd, &buffer, "frame.raw") < 0)
        return FAIL;

    cleanup(&buffer, fd);
    return SUCCESS;
}
