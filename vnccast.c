#include "stdafx.h"
#include "vnccast.h"
#include "view.h"

#define BG_COLOR RGB(32,32,32)
#define WINDOW_CLASS_NAME "VNCCast"

static bool sbActive = false;
static HINSTANCE shInstance;
static int sPort = 5900;
static char sPassword[512] = {0};

ATOM                RegisterMainWindowClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    OptionsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    MSG msg = {0};
    HACCEL hAccelTable;

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if(DialogBox(hInstance, MAKEINTRESOURCE(IDD_OPTIONS), NULL, OptionsProc) != IDOK)
        return 0;

    if(!viewInit(sPort,
                 sPassword,
                 GetSystemMetrics(SM_CXVIRTUALSCREEN), 
                 GetSystemMetrics(SM_CYVIRTUALSCREEN)))
        return 0;

    RegisterMainWindowClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VNCCAST));

    while (msg.message != WM_QUIT) 
    {
        if (PeekMessage( &msg, NULL, 0, 0, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } 
        else 
        {
            viewUpdate(sbActive);
        }
    }

    return (int) msg.wParam;
}

ATOM RegisterMainWindowClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra        = 0;
    wcex.cbWndExtra        = 0;
    wcex.hInstance        = hInstance;
    wcex.hIcon            = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VNCCAST));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH) GetStockObject(NULL_BRUSH);
    wcex.lpszMenuName    = MAKEINTRESOURCE(IDC_VNCCAST);
    wcex.lpszClassName    = WINDOW_CLASS_NAME;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    shInstance = hInstance;

    hWnd = CreateWindow(WINDOW_CLASS_NAME, viewGetDesktopName(), WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_MOVE:
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            viewMove(x,y);
        }
        break;

    case WM_SIZE:
        {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            if(w > 0 && h > 0)
                viewResize(w, h);
        }
        break;

    case WM_ACTIVATE:
        sbActive = (wParam != WA_INACTIVE) ? true : false;
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
        if(message == WM_LBUTTONDOWN || (wParam & MK_LBUTTON))
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            viewPlot(x,y);
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_ERASEBKGND:
        return TRUE;

    case WM_PAINT:
        {
            if(sbActive)
            {
                hdc = BeginPaint(hWnd, &ps);
                viewRender(hWnd, hdc);
                EndPaint(hWnd, &ps);
            }
            else
            {
                HGDIOBJ hOldPen;
                HGDIOBJ hOldBrush;
                RECT r;
                GetClientRect(hWnd, &r);
                hdc = BeginPaint(hWnd, &ps);
                hOldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
                hOldBrush = SelectObject(hdc, GetStockObject(DC_BRUSH));
                SetDCBrushColor(hdc, BG_COLOR);
                Rectangle(hdc, r.left, r.top, r.right+1, r.bottom+1);
                SelectObject(hdc, hOldPen);
                SelectObject(hdc, hOldBrush);
                EndPaint(hWnd, &ps);
            }
            return 0;
        }
        break;

    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void CenterWindowToScreen(HWND hWndToCenter)
{
    RECT rcDlg;
    RECT rcArea;
    RECT rcCenter;
    int xLeft, yTop;
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);

    GetWindowRect(hWndToCenter, &rcDlg);
    GetMonitorInfo(MonitorFromWindow(hWndToCenter, MONITOR_DEFAULTTOPRIMARY), &mi);
    rcCenter = mi.rcWork;
    rcArea = mi.rcWork;

    xLeft = (rcCenter.left + rcCenter.right) / 2 - (rcDlg.right - rcDlg.left) / 2;
    yTop = (rcCenter.top + rcCenter.bottom) / 2 - (rcDlg.bottom - rcDlg.top) / 2;

    if (xLeft < rcArea.left)
        xLeft = rcArea.left;
    else if (xLeft + (rcDlg.right - rcDlg.left) > rcArea.right)
        xLeft = rcArea.right - (rcDlg.right - rcDlg.left);

    if (yTop < rcArea.top)
        yTop = rcArea.top;
    else if (yTop + (rcDlg.bottom - rcDlg.top) > rcArea.bottom)
        yTop = rcArea.bottom - (rcDlg.bottom - rcDlg.top);

    SetWindowPos(hWndToCenter, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

INT_PTR CALLBACK OptionsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        SetWindowText(GetDlgItem(hDlg, IDC_PORT), "5900");
        CenterWindowToScreen(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            char temp[512] = {0};
            GetWindowText(GetDlgItem(hDlg, IDC_PORT), temp, 512);
            temp[511] = 0;
            sPort = atoi(temp);

            GetWindowText(GetDlgItem(hDlg, IDC_PASSWORD), sPassword, 512);
            sPassword[511] = 0;
        }

        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
