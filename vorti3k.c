#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>

float A = 0, B = 0, C = 0;
float cubeWidth;
int width = 80, height = 44;
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
    xp = (int) (width/2 + K1 * ooz * x * 2);
    yp = (int) (height/2 + K1 * ooz * y);
    idx = xp + yp * width;
    if (idx >= 0 && idx < width * height) {
        if (ooz > zBuffer[idx]) {
            zBuffer[idx] = ooz;
            buffer[idx] = ch;
        }
    }
}

int main() {
    initscr();
    // cbreak();          // Disable line buffering
    // noecho();          // Disable echoing of input characters
    nodelay(stdscr, TRUE); // Make getch() non-blocking
    keypad(stdscr, TRUE); 
    clear();

    getmaxyx(stdscr, height, width);

    cubeWidth = (float)fmin(width, height) * 0.8f; // Adjust cubeWidth dynamically
    //ensure cubewidth is even for cleaner output
    if((int)cubeWidth % 2 != 0){
        cubeWidth -= 1;}
    

    zBuffer = (float *)malloc(width * height * sizeof(float));
    buffer = (char *)malloc(width * height * sizeof(char));

    while(1) {
        int ch = getch();
        if (ch== 'r') r = !r;
        if (ch == KEY_UP) {A+=0.05;}
        if (ch == KEY_DOWN) {A-=0.05;}
        if (ch == KEY_LEFT) {B += 0.05;}
        if (ch == KEY_RIGHT) {B -= 0.05;}
        

        memset(buffer, backgroundASCIICode, width * height * sizeof(char));
        memset(zBuffer, 0, width * height * sizeof(float));

        for (float i = -cubeWidth/2; i < cubeWidth/2; i+= 0.6) {
            for (float j = -cubeWidth/2; j < cubeWidth/2; j+= 0.6) {
                calculateForPoint(i, j, -cubeWidth/2, '@');
                calculateForPoint(cubeWidth/2, j, i, '$');
                calculateForPoint(-cubeWidth/2, j, -i, '~');
                calculateForPoint(-i, j, cubeWidth/2, '#');
                calculateForPoint(i, -cubeWidth/2, -j, ';');
                calculateForPoint(i, cubeWidth/2, j, '+');
            }
        }
        move(0,0);
        for (int k = 0; k < width * height; k++) {
            if (k % width) {
                addch(buffer[k]);
            } else {
                addch('\n');
            }
        }
        if (r) {A += 0.05;
        B += 0.05;
        C += 0.01;}
        usleep(8000 * 2);
        refresh();
    }

    return 0;
}