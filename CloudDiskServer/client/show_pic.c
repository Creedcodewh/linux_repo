#include "head.h"

int load_pic(unsigned int *width, unsigned int *height, BGR **stream, char *pic_file_name) {
    FILE *pic_file = NULL;
    unsigned int size;
    unsigned int offset;
    unsigned int sup;
    unsigned int i;

    pic_file = fopen(pic_file_name, "rb");
    if (pic_file == NULL) {
        printf("open file failed !\n");
        return 0;
    }

    fseek(pic_file, 2, SEEK_SET);
    fread(&size, sizeof(unsigned int), 1, pic_file);

    fseek(pic_file, 10, SEEK_SET);
    fread(&offset, sizeof(unsigned int), 1, pic_file);

    fseek(pic_file, 18, SEEK_SET);
    fread(width, sizeof(unsigned int), 1, pic_file);

    fseek(pic_file, 22, SEEK_SET);
    fread(height, sizeof(unsigned int), 1, pic_file);

    *stream = (BGR *)malloc(*width * *height * 3);
    if (*stream == NULL) {
        printf("malloc error !");
        return 0;
    }

    sup = (4 - ((*width * 3) % 4)) % 4; // 填充长度

    for (i = 0; i < *height; i++) {
        fseek(pic_file, offset, SEEK_SET);
        fread((*stream + *width * i), (*width) * sizeof(BGR), 1, pic_file);
        offset += *width * 3 + sup;
    }
    fclose(pic_file);

    return 1;
}

int e2b(unsigned char in) {
    return in>127?1:0;
}

void print_color(BGR bgr_f, BGR bgr_b) {
    int color_f, color_b;
    color_f = 30 + e2b(bgr_f.B) * 4 + e2b(bgr_f.G) * 2 + e2b(bgr_f.R);
    color_b = 40 + e2b(bgr_b.B) * 4 + e2b(bgr_b.G) * 2 + e2b(bgr_b.R);
     printf("\033[%d;%dm▀", color_f, color_b);
    return;
}

int printf_login(char *argv) {
    char *pic_file_name = NULL;
    unsigned int width;
    unsigned int height;
    BGR *stream;
    BGR BLACK = {0, 0, 0};
    struct winsize ws;
    unsigned int shell_width;
    unsigned int loop_width;
    double ratio;
    int i, j;

    pic_file_name = (char *)malloc(strlen(argv) + 1);
    pic_file_name = argv;



    if (load_pic(&width, &height, &stream, pic_file_name) == 0) {
        return -1;
    }

    ioctl(0, TIOCGWINSZ, &ws);
    shell_width = ws.ws_col;

    ratio = (double)width / shell_width;
    loop_width = shell_width;


    for (i = 0; i < height; i += (unsigned int)(2 * ratio + 0.5)) {
        for (j = 0; j < loop_width; j++) {
            if (i + (unsigned int)(ratio + 0.25) >= height){
                print_color(stream[width * (height - i - 1) + (unsigned int)(j * ratio)], BLACK);   
                continue;
            }
            print_color(stream[width * (height - i - 1) + (unsigned int)(j * ratio)], stream[width * (height - i - (unsigned int)(ratio + 0.25) - 1) + (unsigned int)(j * ratio)]);   
        }
        printf("\033[39;49m\n");
    }

    return 0;
}
