#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>

#define PI 3.14159265359f

float A = 0, B = 0, C = 0;
float cubeWidth;
int width, height;
float *zBuffer;
char *buffer;
int backgroundASCIICode = ' ';
int distanceFromCam = 100;
float K1 = 40;
float x, y, z;
float ooz;
int xp, yp;
int idx;
int r = 0;
float cameraY = 0; // Camera Y position

float calculateX(float i, float j, float k) {
    return j * sin(A) * sin(B) * cos(C) - k * cos(A) * sin(B) * cos(C) + j * cos(A) * sin(C) + k * sin(A) * sin(C) + i * cos(B) * cos(C);
}

float calculateY(float i, float j, float k) {
    return j * cos(A) * cos(C) + k * sin(A) * cos(C) - j * sin(A) * sin(B) * sin(C) + k * cos(A) * sin(B) * sin(C) - i * cos(B) * sin(C);
}

float calculateZ(float i, float j, float k) {
    return k * cos(A) * sin(B) - j * sin(A) * cos(B) + i * sin(B);
}

void calculateForPoint(float i, float j, float k, int ch) {
    x = calculateX(i, j, k);
    y = calculateY(i, j, k);
    z = calculateZ(i, j, k) + distanceFromCam;
    ooz = 1 / z;
    xp = (int)(width / 2 + K1 * ooz * x * 2);
    yp = (int)(height / 2 + K1 * ooz * y);
    idx = xp + yp * width;
    if (idx >= 0 && idx < width * height) {
        if (ooz > zBuffer[idx]) {
            zBuffer[idx] = ooz;
            buffer[idx] = ch;
        }
    }
}

void globRotX(float t) {
    float r00 = cos(B) * cos(C);
    float r01 = sin(B) * sin(t) - cos(B) * sin(C) * cos(t);
    float r02 = cos(B) * sin(C) * sin(t) + sin(B) * cos(t);
    float r10 = sin(A) * sin(B) * cos(C) + cos(A) * sin(C);
    float r11 = cos(t) * (cos(A) * cos(C) - sin(A) * sin(B) * sin(C)) - sin(A) * cos(B) * sin(t);
    float r12 = -sin(t) * (cos(A) * cos(C) - sin(A) * sin(B) * sin(C)) - sin(A) * cos(B) * cos(t);
    float r22 = cos(A) * cos(B) * cos(t) - sin(t) * (cos(A) * sin(B) * sin(C) + sin(A) * cos(C));
    
    if (r02 < +1) {
        if (r02 > -1) {
            B = asin(r02);
            A = atan2(-r12, r22);
            C = atan2(-r01, r00);
        } else {
            B = -PI/2;
            A = -atan2(r10,r11);
            C = 0;
        }
    } else {
        B = +PI/2;
        A = atan2(r10,r11);
        C = 0;
    }
}

void globRotY(float t) {
    float r00 = cos(B) * cos(C) * cos(t) - sin(B) * sin(t);
    float r01 = -cos(B) * sin(C);
    float r02 = cos(B) * cos(C) * sin(t) + sin(B) * cos(t);
    float r10 = cos(t) * (sin(A) * sin(B) * cos(C) + cos(A) * sin(C)) + sin(A) * cos(B) * sin(t);
    float r11 = cos(A) * cos(C) - sin(A) * sin(B) * sin(C);
    float r12 = sin(t) * (sin(A) * sin(B) * cos(C) + cos(A) * sin(C)) - sin(A) * cos(B) * cos(t);
    float r22 = sin(t) * (sin(A) * sin(C) - cos(A) * sin(B) * cos(C)) + cos(A) * cos(B) * cos(t);
    
    if (r02 < +1) {
        if (r02 > -1) {
            B = asin(r02);
            A = atan2(-r12, r22);
            C = atan2(-r01, r00);
        } else {
            B = -PI/2;
            A = -atan2(r10,r11);
            C = 0;
        }
    } else {
        B = +PI/2;
        A = atan2(r10,r11);
        C = 0;
    }
}

void cleanup() {
    free(zBuffer);
    free(buffer);
    endwin();
}

void init_ncurses() {

}

int main() {
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, height, width);

    cubeWidth = (float)fmin(width, height) * 0.8f;
    if ((int)cubeWidth % 2 != 0) {
        cubeWidth -= 1;
    }

    zBuffer = (float *)malloc(width * height * sizeof(float));
    buffer = (char *)malloc(width * height * sizeof(char));

    if (zBuffer == NULL || buffer == NULL) {
        endwin();
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    erase();

    while (1) {
        int ch = getch();
        if (ch == 'r') r = !r;
        else if (ch == KEY_UP) globRotX(0.05);
        else if (ch == KEY_DOWN) globRotX(-0.05);
        else if (ch == KEY_LEFT) globRotY(0.05);
        else if (ch == KEY_RIGHT) globRotY(-0.05);
        else if (ch == 'q') {
            cleanup();
            exit(0);
        }

        memset(buffer, backgroundASCIICode, width * height * sizeof(char));
        memset(zBuffer, 0, width * height * sizeof(float));

        for (float i = -cubeWidth / 2; i < cubeWidth / 2; i += 0.6) {
            for (float j = -cubeWidth / 2; j < cubeWidth / 2; j += 0.6) {
                calculateForPoint(i, j - cameraY, -cubeWidth / 2, '@');
                calculateForPoint(cubeWidth / 2, j - cameraY, i, '$');
                calculateForPoint(-cubeWidth / 2, j - cameraY, -i, '~');
                calculateForPoint(-i, j - cameraY, cubeWidth / 2, '#');
                calculateForPoint(i, -cubeWidth / 2 - cameraY, -j, ';');
                calculateForPoint(i, cubeWidth / 2 - cameraY, j - cameraY, '+');
            }
        }
        move(0, 0);
        for (int k = 0; k < width * height; k++) {
            if (k % width) {
                addch(buffer[k]);
            } else {
                addch('\n');
            }
        }
        if (r) {
            A += 0.05;
            B += 0.05;
            C += 0.01;
        }
        usleep(8000 * 2);
        refresh();
    }

    free(zBuffer);
    free(buffer);
    endwin();
    return 0;
}