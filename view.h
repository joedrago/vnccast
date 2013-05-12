#pragma once

#include <windows.h>

char *viewGetDesktopName();

bool viewInit(int port, char *password, int maxx, int maxy);
void viewShutdown(void);
void viewUpdate(bool bActive);

void viewMove(int x, int y);
void viewResize(int w, int h);

void viewRender(HWND hWnd, HDC hdc);

void viewPlot(int x, int y);
