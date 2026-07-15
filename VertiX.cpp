#include <windows.h>
#include <gdiplus.h>
#include <cstdio>
using namespace Gdiplus;

#pragma comment(lib, "gdiplus.lib")

// Configuration structure
struct OverlayConfig {
    int crosshair_size = 40;
    int crosshair_thickness = 2;
    COLORREF crosshair_color = RGB(255, 0, 0);
    int crosshair_alpha = 255;
    bool show_crosshair = true;
    int axis_thickness = 2;
    COLORREF axis_color = RGB(0, 200, 0);
    int axis_alpha = 255;
    int axis_h_percent = 100;
    int axis_v_percent = 100;
    bool is_running = true;
};

OverlayConfig g_config;
HWND g_overlay_hwnd = NULL;
HWND g_config_hwnd = NULL;
int g_screen_width = 0;
int g_screen_height = 0;

ULONG_PTR g_gdiplusToken = 0;

// Draw axes without arrows, with adjustable lengths
void DrawAxes(Graphics& graphics, int center_x, int center_y, int thickness, COLORREF color, int alpha,
              int h_percent, int v_percent) {
    if (thickness <= 0) return;
    int r = GetRValue(color);
    int g = GetGValue(color);
    int b = GetBValue(color);
    Pen pen(Color(alpha, r, g, b), (REAL)thickness);
    pen.SetLineJoin(LineJoinRound);

    int h_len = (int)((double)g_screen_width * h_percent / 100.0);
    int v_len = (int)((double)g_screen_height * v_percent / 100.0);

    if (h_percent > 0) {
        graphics.DrawLine(&pen, 0, center_y, h_len, center_y);
        graphics.DrawLine(&pen, g_screen_width - h_len, center_y, g_screen_width, center_y);
    }
    if (v_percent > 0) {
        graphics.DrawLine(&pen, center_x, 0, center_x, v_len);
        graphics.DrawLine(&pen, center_x, g_screen_height - v_len, center_x, g_screen_height);
    }
}

// Draw crosshair with transparency
void DrawCrosshair(Graphics& graphics, int center_x, int center_y, int size, int thickness, COLORREF color, int alpha) {
    if (size <= 0 || thickness <= 0) return;
    int r = GetRValue(color);
    int g = GetGValue(color);
    int b = GetBValue(color);
    Pen pen(Color(alpha, r, g, b), (REAL)thickness);
    pen.SetLineJoin(LineJoinRound);
    graphics.DrawLine(&pen, center_x - size, center_y, center_x + size, center_y);
    graphics.DrawLine(&pen, center_x, center_y - size, center_x, center_y + size);
}

// Overlay window procedure
LRESULT CALLBACK OverlayProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            Graphics graphics(hdc);

            SolidBrush blackBrush(Color(255, 0, 0, 0));
            graphics.FillRectangle(&blackBrush, 0, 0, g_screen_width, g_screen_height);

            int cx = g_screen_width / 2;
            int cy = g_screen_height / 2;

            DrawAxes(graphics, cx, cy, g_config.axis_thickness, g_config.axis_color, g_config.axis_alpha,
                     g_config.axis_h_percent, g_config.axis_v_percent);

            if (g_config.show_crosshair) {
                DrawCrosshair(graphics, cx, cy,
                              g_config.crosshair_size, g_config.crosshair_thickness,
                              g_config.crosshair_color, g_config.crosshair_alpha);
            }

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Helper: safely update int from edit control
void UpdateIntFromEdit(HWND hEdit, int& target, int minVal, int maxVal, int defaultVal) {
    char buf[32];
    GetWindowText(hEdit, buf, 32);
    int val = atoi(buf);
    if (val >= minVal && val <= maxVal) {
        target = val;
    } else {
        if (val < minVal) val = minVal;
        else if (val > maxVal) val = maxVal;
        target = val;
        char newBuf[32];
        sprintf(newBuf, "%d", val);
        SetWindowText(hEdit, newBuf);
    }
}

// Configuration window procedure
LRESULT CALLBACK ConfigProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hCrosshairSize, hCrosshairThick, hCrosshairAlpha;
    static HWND hAxisThick, hAxisAlpha, hAxisHPercent, hAxisVPercent;
    static HWND hShowCrosshair;
    static HWND hCrosshairColorRed, hCrosshairColorGreen, hCrosshairColorBlue;
    static HWND hAxisColorRed, hAxisColorGreen, hAxisColorBlue;

    switch (uMsg) {
        case WM_CREATE: {
            // Crosshair Settings
            CreateWindow("STATIC", "Crosshair Settings", WS_CHILD | WS_VISIBLE,
                         10, 10, 200, 20, hwnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Size:", WS_CHILD | WS_VISIBLE,
                         10, 35, 60, 20, hwnd, NULL, NULL, NULL);
            hCrosshairSize = CreateWindow("EDIT", "40", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                                          70, 33, 60, 22, hwnd, (HMENU)1, NULL, NULL);

            CreateWindow("STATIC", "Thickness:", WS_CHILD | WS_VISIBLE,
                         10, 65, 60, 20, hwnd, NULL, NULL, NULL);
            hCrosshairThick = CreateWindow("EDIT", "2", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                                           70, 63, 60, 22, hwnd, (HMENU)2, NULL, NULL);

            CreateWindow("STATIC", "Alpha (0-255):", WS_CHILD | WS_VISIBLE,
                         10, 95, 90, 20, hwnd, NULL, NULL, NULL);
            hCrosshairAlpha = CreateWindow("EDIT", "255", WS_CHILD | WS_VISIBLE | WS_BORDER,
                                           105, 93, 40, 22, hwnd, (HMENU)12, NULL, NULL);

            hShowCrosshair = CreateWindow("BUTTON", "Show Crosshair", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                                          10, 125, 130, 25, hwnd, (HMENU)3, NULL, NULL);
            SendMessage(hShowCrosshair, BM_SETCHECK, BST_CHECKED, 0);

            // Axis Settings
            CreateWindow("STATIC", "Axis Settings (Always On)", WS_CHILD | WS_VISIBLE,
                         10, 165, 200, 20, hwnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Thickness:", WS_CHILD | WS_VISIBLE,
                         10, 190, 60, 20, hwnd, NULL, NULL, NULL);
            hAxisThick = CreateWindow("EDIT", "2", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                                      70, 188, 60, 22, hwnd, (HMENU)4, NULL, NULL);

            CreateWindow("STATIC", "Alpha (0-255):", WS_CHILD | WS_VISIBLE,
                         10, 220, 90, 20, hwnd, NULL, NULL, NULL);
            hAxisAlpha = CreateWindow("EDIT", "255", WS_CHILD | WS_VISIBLE | WS_BORDER,
                                      105, 218, 40, 22, hwnd, (HMENU)13, NULL, NULL);

            CreateWindow("STATIC", "H Length (%):", WS_CHILD | WS_VISIBLE,
                         10, 250, 90, 20, hwnd, NULL, NULL, NULL);
            hAxisHPercent = CreateWindow("EDIT", "100", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                                         105, 248, 40, 22, hwnd, (HMENU)16, NULL, NULL);

            CreateWindow("STATIC", "V Length (%):", WS_CHILD | WS_VISIBLE,
                         10, 280, 90, 20, hwnd, NULL, NULL, NULL);
            hAxisVPercent = CreateWindow("EDIT", "100", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                                         105, 278, 40, 22, hwnd, (HMENU)17, NULL, NULL);

            // Colors - Crosshair
            CreateWindow("STATIC", "Crosshair Color", WS_CHILD | WS_VISIBLE,
                         10, 315, 120, 20, hwnd, NULL, NULL, NULL);

            hCrosshairColorRed = CreateWindow("BUTTON", "Red", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
                                              10, 340, 60, 25, hwnd, (HMENU)6, NULL, NULL);
            SendMessage(hCrosshairColorRed, BM_SETCHECK, BST_CHECKED, 0);

            hCrosshairColorGreen = CreateWindow("BUTTON", "Green", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                                                80, 340, 60, 25, hwnd, (HMENU)7, NULL, NULL);

            hCrosshairColorBlue = CreateWindow("BUTTON", "Blue", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                                               150, 340, 60, 25, hwnd, (HMENU)8, NULL, NULL);

            // Colors - Axis
            CreateWindow("STATIC", "Axis Color", WS_CHILD | WS_VISIBLE,
                         10, 375, 120, 20, hwnd, NULL, NULL, NULL);

            hAxisColorRed = CreateWindow("BUTTON", "Red", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
                                         10, 400, 60, 25, hwnd, (HMENU)9, NULL, NULL);

            hAxisColorGreen = CreateWindow("BUTTON", "Green", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                                           80, 400, 60, 25, hwnd, (HMENU)10, NULL, NULL);
            SendMessage(hAxisColorGreen, BM_SETCHECK, BST_CHECKED, 0);

            hAxisColorBlue = CreateWindow("BUTTON", "Blue", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                                          150, 400, 60, 25, hwnd, (HMENU)11, NULL, NULL);

            // Close button (only button, no Apply)
            CreateWindow("BUTTON", "Close", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                         100, 435, 80, 30, hwnd, (HMENU)20, NULL, NULL);

            // Set initial values from g_config into edit controls
            char buf[32];
            sprintf(buf, "%d", g_config.crosshair_size);
            SetWindowText(hCrosshairSize, buf);
            sprintf(buf, "%d", g_config.crosshair_thickness);
            SetWindowText(hCrosshairThick, buf);
            sprintf(buf, "%d", g_config.crosshair_alpha);
            SetWindowText(hCrosshairAlpha, buf);
            sprintf(buf, "%d", g_config.axis_thickness);
            SetWindowText(hAxisThick, buf);
            sprintf(buf, "%d", g_config.axis_alpha);
            SetWindowText(hAxisAlpha, buf);
            sprintf(buf, "%d", g_config.axis_h_percent);
            SetWindowText(hAxisHPercent, buf);
            sprintf(buf, "%d", g_config.axis_v_percent);
            SetWindowText(hAxisVPercent, buf);

            // Set radio buttons according to current colors
            if (g_config.crosshair_color == RGB(255,0,0))
                SendMessage(hCrosshairColorRed, BM_SETCHECK, BST_CHECKED, 0);
            else if (g_config.crosshair_color == RGB(0,255,0))
                SendMessage(hCrosshairColorGreen, BM_SETCHECK, BST_CHECKED, 0);
            else if (g_config.crosshair_color == RGB(0,0,255))
                SendMessage(hCrosshairColorBlue, BM_SETCHECK, BST_CHECKED, 0);

            if (g_config.axis_color == RGB(255,0,0))
                SendMessage(hAxisColorRed, BM_SETCHECK, BST_CHECKED, 0);
            else if (g_config.axis_color == RGB(0,200,0))
                SendMessage(hAxisColorGreen, BM_SETCHECK, BST_CHECKED, 0);
            else if (g_config.axis_color == RGB(0,0,255))
                SendMessage(hAxisColorBlue, BM_SETCHECK, BST_CHECKED, 0);

            SendMessage(hShowCrosshair, BM_SETCHECK, g_config.show_crosshair ? BST_CHECKED : BST_UNCHECKED, 0);

            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int code = HIWORD(wParam);

            // Handle edit changes (EN_CHANGE) – real-time
            if (code == EN_CHANGE) {
                HWND hEdit = (HWND)lParam;
                if (hEdit == hCrosshairSize) {
                    UpdateIntFromEdit(hEdit, g_config.crosshair_size, 1, 1000, 40);
                    InvalidateRect(g_overlay_hwnd, NULL, TRUE);
                }
                else if (hEdit == hCrosshairThick) {
                    UpdateIntFromEdit(hEdit, g_config.crosshair_thickness, 1, 20, 2);
                    InvalidateRect(g_overlay_hwnd, NULL, TRUE);
                }
                else if (hEdit == hCrosshairAlpha) {
                    UpdateIntFromEdit(hEdit, g_config.crosshair_alpha, 0, 255, 255);
                    InvalidateRect(g_overlay_hwnd, NULL, TRUE);
                }
                else if (hEdit == hAxisThick) {
                    UpdateIntFromEdit(hEdit, g_config.axis_thickness, 1, 20, 2);
                    InvalidateRect(g_overlay_hwnd, NULL, TRUE);
                }
                else if (hEdit == hAxisAlpha) {
                    UpdateIntFromEdit(hEdit, g_config.axis_alpha, 0, 255, 255);
                    InvalidateRect(g_overlay_hwnd, NULL, TRUE);
                }
                else if (hEdit == hAxisHPercent) {
                    UpdateIntFromEdit(hEdit, g_config.axis_h_percent, 0, 100, 100);
                    InvalidateRect(g_overlay_hwnd, NULL, TRUE);
                }
                else if (hEdit == hAxisVPercent) {
                    UpdateIntFromEdit(hEdit, g_config.axis_v_percent, 0, 100, 100);
                    InvalidateRect(g_overlay_hwnd, NULL, TRUE);
                }
                return 0;
            }

            // Handle checkbox
            if (wmId == 3) {
                g_config.show_crosshair = (SendMessage(hShowCrosshair, BM_GETCHECK, 0, 0) == BST_CHECKED);
                InvalidateRect(g_overlay_hwnd, NULL, TRUE);
                return 0;
            }

            // Handle radio buttons – Crosshair color
            if (wmId >= 6 && wmId <= 8) {
                if (wmId == 6) g_config.crosshair_color = RGB(255, 0, 0);
                else if (wmId == 7) g_config.crosshair_color = RGB(0, 255, 0);
                else if (wmId == 8) g_config.crosshair_color = RGB(0, 0, 255);
                InvalidateRect(g_overlay_hwnd, NULL, TRUE);
                return 0;
            }
            // Radio buttons – Axis color
            if (wmId >= 9 && wmId <= 11) {
                if (wmId == 9) g_config.axis_color = RGB(255, 0, 0);
                else if (wmId == 10) g_config.axis_color = RGB(0, 200, 0);
                else if (wmId == 11) g_config.axis_color = RGB(0, 0, 255);
                InvalidateRect(g_overlay_hwnd, NULL, TRUE);
                return 0;
            }

            // Handle Close button
            if (wmId == 20) {
                g_config.is_running = false;
                PostQuitMessage(0);
                return 0;
            }
            break;
        }

        case WM_DESTROY:
            g_config.is_running = false;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Create overlay window
HWND CreateOverlayWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = OverlayProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "OverlayWindow";
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        "OverlayWindow",
        "Overlay",
        WS_POPUP,
        0, 0, g_screen_width, g_screen_height,
        NULL, NULL, hInstance, NULL
    );

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    return hwnd;
}

// Create configuration window
HWND CreateConfigWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = ConfigProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "ConfigWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        "ConfigWindow",
        "Overlay Configuration",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 260, 500,   // Height increased for Close button
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    return hwnd;
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

    g_screen_width = GetSystemMetrics(SM_CXSCREEN);
    g_screen_height = GetSystemMetrics(SM_CYSCREEN);

    printf("============================================\n");
    printf("  Overlay Configuration Tool\n");
    printf("============================================\n");
    printf("Screen: %dx%d\n", g_screen_width, g_screen_height);
    printf("All settings update in real-time.\n");
    printf("Click 'Close' button to exit.\n");
    printf("============================================\n\n");

    g_overlay_hwnd = CreateOverlayWindow(hInstance);
    if (!g_overlay_hwnd) {
        MessageBox(NULL, "Failed to create overlay window", "Error", MB_OK);
        GdiplusShutdown(g_gdiplusToken);
        return 1;
    }

    g_config_hwnd = CreateConfigWindow(hInstance);
    if (!g_config_hwnd) {
        MessageBox(NULL, "Failed to create config window", "Error", MB_OK);
        DestroyWindow(g_overlay_hwnd);
        GdiplusShutdown(g_gdiplusToken);
        return 1;
    }

    MSG msg;
    while (g_config.is_running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);
    }

    DestroyWindow(g_overlay_hwnd);
    DestroyWindow(g_config_hwnd);
    GdiplusShutdown(g_gdiplusToken);
    return 0;
}