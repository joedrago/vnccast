#include "view.h"
#include "rfb/rfb.h"

#define REFRESH_INTERVAL (250)

static rfbScreenInfoPtr spServer = NULL;
static char *spRenderBuffer = NULL;
static char sDesktopName[1024];
static RECT sBox = {0};
static int sMaxX = 0;
static int sMaxY = 0;

char *viewGetDesktopName()
{
	return sDesktopName;
}

bool viewInit(int port, char *password, int maxx, int maxy)
{
    static char *passwordList[2] = { NULL, NULL };
	int argc=1;
	char *argv[] = { "wut" };
	char computerName[1024];
	DWORD size = 1024;

	spServer = rfbGetScreen(&argc,argv,200,200,8,3,4);
	if(maxx == 0 || maxy == 0)
	{
		MessageBox(NULL, "Cannot detect size of monitors!", "vnccast Failure", MB_OK);
		return false;
	}

	spServer->port = port;
    spServer->alwaysShared = TRUE;

    if(password[0])
    {
        passwordList[0] = password;
        spServer->authPasswdData = (void *)passwordList;
        spServer->passwordCheck = rfbCheckPasswordByList;
    }

	computerName[0] = 0;
	GetComputerName(computerName, &size);
	computerName[1023] = 0;
	sprintf(sDesktopName, "VNCCast from %s", computerName);
	spServer->desktopName = sDesktopName;

//    spServer->passwordCheck

	sMaxX = maxx;
	sMaxY = maxy;
	spServer->frameBuffer = malloc(maxx*maxy*4);
	spRenderBuffer = malloc(maxx*maxy*4);
	rfbInitServer(spServer);
    if(spServer->listenSock < 0)
    {
        MessageBox(NULL, "Cannot listen on port!", "vnccast Failure", MB_OK);
        return false;
    }
	return true;
}

void viewShutdown()
{
}

void viewCapture()
{
	BITMAPINFOHEADER bi = {0};

	int w = sBox.right - sBox.left;
	int h = sBox.bottom - sBox.top;
	int x, y;

    HWND hDesktopWnd = GetDesktopWindow();
    HDC hDesktopDC = GetDC(hDesktopWnd);
    HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);
    HBITMAP hCaptureBitmap =CreateCompatibleBitmap(hDesktopDC, 
                            w, h);
    SelectObject(hCaptureDC,hCaptureBitmap); 

    BitBlt(hCaptureDC,0,0,w,h,
           hDesktopDC,sBox.left,sBox.top,SRCCOPY); 

    bi.biSize = sizeof(BITMAPINFOHEADER);    
    bi.biWidth = w;
    bi.biHeight = -h;
    bi.biPlanes = 1;    
    bi.biBitCount = 32;    
    bi.biCompression = BI_RGB;

 	GetDIBits(hCaptureDC, hCaptureBitmap, 0, h, spServer->frameBuffer, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    ReleaseDC(hDesktopWnd,hDesktopDC);
    DeleteDC(hCaptureDC);
    DeleteObject(hCaptureBitmap);

	// BGR -> RGB
	for(y=0; y<h; y++)
	{
		for(x=0; x<w; x++)
		{
			int offset = 4 * ((y * w) + x);
			unsigned char c = spServer->frameBuffer[offset+0];
			spServer->frameBuffer[offset+0] = spServer->frameBuffer[offset+2];
			spServer->frameBuffer[offset+2] = c;
		}
	}

	rfbMarkRectAsModified(spServer, 0, 0, w, h);
}

void viewUpdate(bool bActive)
{
	static unsigned int lastTick = 0;
	unsigned int tick = GetTickCount();

	if(lastTick == 0)
		lastTick = GetTickCount();

	if(!bActive)
	{
		if(tick > lastTick + REFRESH_INTERVAL)
		{
			viewCapture();
			lastTick = tick;
		}
	}

	rfbProcessEvents(spServer, 5000);
}

void viewMove(int x, int y)
{
	sBox.right  += (x - sBox.left);
	sBox.bottom += (y - sBox.top);
	sBox.left = x;
	sBox.top  = y;
}

void viewResize(int w, int h)
{
	if(w < 1) w = 1;
	if(h < 1) h = 1;
	if(w > sMaxX) w = sMaxX;
	if(h > sMaxY) h = sMaxY;

	rfbNewFramebuffer(spServer, spServer->frameBuffer, w, h, 8, 3, 4);
	sBox.right  = w + sBox.left;
	sBox.bottom = h + sBox.top;
	memset(spServer->frameBuffer, 0, w*h*4);
}

void viewRender(HWND hWnd, HDC hdc)
{
	int x,y;
	int w = sBox.right - sBox.left;
	int h = sBox.bottom - sBox.top;
	BITMAPINFOHEADER bi = {0};

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = w;
    bi.biHeight = -h;
    bi.biPlanes = 1;    
    bi.biBitCount = 32;    
    bi.biCompression = BI_RGB;

	for(y=0; y<h; y++)
	{
		for(x=0; x<w; x++)
		{
			int offset = 4 * ((y * w) + x);
			spRenderBuffer[offset+0] = spServer->frameBuffer[offset+2];
			spRenderBuffer[offset+1] = spServer->frameBuffer[offset+1];
			spRenderBuffer[offset+2] = spServer->frameBuffer[offset+0];
			spRenderBuffer[offset+3] = spServer->frameBuffer[offset+3];
		}
	}

	StretchDIBits(hdc, 0,0, w, h, 0, 0, w, h, spRenderBuffer, (BITMAPINFO*)&bi, DIB_RGB_COLORS, SRCCOPY);
}

void viewPlot(int x, int y)
{
	int w = sBox.right - sBox.left;
	int h = sBox.bottom - sBox.top;
	int offset = 4 * ((y * w) + x);
	spServer->frameBuffer[offset+0] = 255;
	spServer->frameBuffer[offset+1] = 0;
	spServer->frameBuffer[offset+2] = 0;
	spServer->frameBuffer[offset+3] = 0;
	rfbMarkRectAsModified(spServer, x, y, x+1, y+1);
}
