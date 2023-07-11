#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <windows.h>
#include <unistd.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define SEED_COUNT 100
#define SEED_SIZE 2
#define SEED_COLOR WHITE_COLOR

#define RED_COLOR 0xFF0000FF
#define BLUE_COLOR 0xFFFF0000
#define WHITE_COLOR 0xFFFFFFFF

#define IMAGE_WIDTH 3840
#define IMAGE_HEIGHT 2160
#define OUTPUT_FILE_PPM "./output.ppm"
#define OUTPUT_FILE_PNG "\\output.png"

typedef uint32_t Color32;

typedef struct {
    int x, y;
    Color32 color;
} Seed;

Color32 palette[] = {0x91c4f2, 0x8ca0d7, 0x9d79bc, 0xa14da0, 0x7e1f86, 0x231651, 0x4dccbd, 0x2374ab, 0xff8484};

Color32 image[IMAGE_HEIGHT][IMAGE_WIDTH];
Seed seeds[SEED_COUNT];

void create_seeds(void) {
    srand(time(NULL));
    int x, y;
    Color32 color;
    for (int i = 0; i < SEED_COUNT; i++) {
        x = rand() % IMAGE_WIDTH;
        y = rand() % IMAGE_HEIGHT;
        color = palette[rand() % (sizeof(palette) / sizeof(Color32))];
        seeds[i].x = x;
        seeds[i].y = y;
        seeds[i].color = (color);
    }
}

int sqr_distance(int x1, int y1, int x2, int y2) {
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

void draw_seeds(void) {
    for (int i = 0; i < SEED_COUNT; i++) {
        int left = MAX(seeds[i].x - SEED_SIZE, 0);
        int right = seeds[i].x + SEED_SIZE;
        int top = MAX(seeds[i].y - SEED_SIZE, 0);
        int bottom = seeds[i].y + SEED_SIZE;

        for (int y = top; y <= bottom; y++) {
            for (int x = left; x <= right; x++) {
                if (sqr_distance(seeds[i].x, seeds[i].y, x, y) <= SEED_SIZE * SEED_SIZE) {
                    image[y][x] = (Color32)SEED_COLOR;
                }
            }
        }
    }
}

void fill_image(Color32 color) {
    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            image[y][x] = color;
        }
    }
}

void draw_voronoi() {
    Seed *closest = &seeds[0];
    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            for (size_t s = 0; s < SEED_COUNT; s++) {
                if (sqr_distance(x, y, seeds[s].x, seeds[s].y) < sqr_distance(x, y, closest->x, closest->y)) {
                    closest = &seeds[s];
                }
            }
            image[y][x] = closest->color;
        }
    }
}

void render_image(char *filepath) {
    FILE *file = fopen(OUTPUT_FILE_PPM, "wb");
    if (file == NULL) {
        fprintf(stderr, "Can't open the file. %s\n", strerror(errno));
        exit(1);
    }
    int wresult = fprintf(file, "P6 %d %d 255 ", IMAGE_WIDTH, IMAGE_HEIGHT);
    if (wresult <= 0) {
        fprintf(stderr, "some error while writing to file. %s\n", strerror(errno));
        exit(1);
    }

    int closed;
    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            Color32 pixel = image[y][x];
            uint8_t color_bytes[3] = {
                (pixel & 0x0000FF) >> 8 * 0,
                (pixel & 0x00FF00) >> 8 * 1,
                (pixel & 0xFF0000) >> 8 * 2,
            };
            wresult = fwrite(color_bytes, sizeof(color_bytes), 1, file);
            if (wresult <= 0) {
                fprintf(stderr, "some error while writing to file. %s\n", strerror(errno));
                closed = fclose(file);
                exit(1);
            }
        }
    }

    closed = fclose(file);
    if (closed != 0) {
        fprintf(stderr, "can't close the file. %s\n", strerror(errno));
        exit(1);
    }
}

void change_windows_bg() {
    char path[PATH_MAX];
    if (getcwd(path, sizeof(path)) == NULL) {
        fprintf(stderr, "getcwd() error %s\n", strerror(errno));
    }
    strcat(path, OUTPUT_FILE_PNG);
    printf("path = %s\n", path);

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not set new background. There is no such file as %s\n", path);
        fclose(fp);
        exit(1);
    }

    int closed = fclose(fp);

    if (closed != 0) {
        fprintf(stderr, "can't close the file. %s\n", strerror(errno));
        exit(1);
    }

    int ok = SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, path, SPIF_SENDCHANGE);
    if (!ok) {
        fprintf(stderr, "Could not set new background %ld\n", GetLastError());
        exit(1);
    }
}

int main(int argc, char **argv) {
    // fill_image(REVERSE_COLOR(RED_COLOR));
    create_seeds();
    draw_voronoi();
    // draw_seeds();
    render_image(OUTPUT_FILE_PPM);
    // change_windows_bg();
    return 0;
}
