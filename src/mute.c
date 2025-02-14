#define UNICODE
#define WIN32_LEAN_AND_MEAN

#include <WINDOWS.H>
#include <COMBASEAPI.h>
#include <MMDEVICEAPI.h>
#include <ENDPOINTVOLUME.h>

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

static CONST GUID IID_IMMDeviceEnumerator = { 0xA95664D2, 0x9614, 0x4F35, { 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6 } };
static CONST GUID IID_MMDeviceEnumerator = { 0xBCDE0395, 0xE52F, 0x467C, { 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E } };
static CONST GUID IID_IAudioEndpointVolume = { 0x5CDF2C82, 0x841E, 0x4546, { 0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A } };

static BOOL AllMicrophonesMuted(BOOL toggle)
{
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    IMMDeviceCollection *deviceCollection = NULL;
    UINT deviceCount = 0;
    IMMDevice *device = NULL;
    IAudioEndpointVolume *endpointVolume = NULL;
    BOOL allMuted = FALSE;
	// Accumulate HRESULTs and return false if not 100% sure that all microphones were muted
	BOOL success = SUCCEEDED(CoCreateInstance(&IID_MMDeviceEnumerator, NULL, CLSCTX_INPROC_SERVER, &IID_IMMDeviceEnumerator, (VOID *)&deviceEnumerator));
	if (success && deviceEnumerator) success = SUCCEEDED(deviceEnumerator->lpVtbl->EnumAudioEndpoints(deviceEnumerator, eCapture, DEVICE_STATE_ACTIVE, &deviceCollection));
	if (success && deviceCollection) success = SUCCEEDED(deviceCollection->lpVtbl->GetCount(deviceCollection, &deviceCount));
	if (success && deviceCount > 0) {
        // Check if all audio capture devices are muted
		for (INT i = 0 ; i < deviceCount && success; i++) {
			device = NULL;
			endpointVolume = NULL;
            success = SUCCEEDED(deviceCollection->lpVtbl->Item(deviceCollection, i, &device));
			if (success && device) success = SUCCEEDED(device->lpVtbl->Activate(device, &IID_IAudioEndpointVolume, CLSCTX_INPROC_SERVER, NULL, (VOID *)&endpointVolume));
			if (success && endpointVolume) success = SUCCEEDED(endpointVolume->lpVtbl->GetMute(endpointVolume, &allMuted));
			if (!allMuted) break;
		}
        if (toggle) {
            // If any audio capture device is not muted, mute all audio capture devices; otherwise unmute all audio capture devices
		    for (INT i = 0 ; i < deviceCount && success; i++) {
			    device = NULL;
			    endpointVolume = NULL;
                success = SUCCEEDED(deviceCollection->lpVtbl->Item(deviceCollection, i, &device));
			    if (success && device) success = SUCCEEDED(device->lpVtbl->Activate(device, &IID_IAudioEndpointVolume, CLSCTX_INPROC_SERVER, NULL, (VOID *)&endpointVolume));
			    if (success && endpointVolume) success = SUCCEEDED(endpointVolume->lpVtbl->SetMute(endpointVolume, !allMuted, NULL));
		    }
		    if (success) allMuted = !allMuted;
        }
	}
	if (endpointVolume) endpointVolume->lpVtbl->Release(endpointVolume);
	if (device) device->lpVtbl->Release(device);
	if (deviceCollection) deviceCollection->lpVtbl->Release(deviceCollection);
	if (deviceEnumerator) deviceEnumerator->lpVtbl->Release(deviceEnumerator);
    // Beware that FALSE is overloaded with two meanings:
    //  1) Either not all audio capture devices are muted
    //  2) Or there was an error determining whether all audio capture devices are muted
    // Both cases of FALSE mean that some audio capture devices may be recording.
    return success && (allMuted || !deviceCount);
}

typedef struct HOTKEY { UINT id, mod, key; } HOTKEY;

static CONST HOTKEY Hotkey1 = { 1, MOD_ALT,             0x4D }; // Alt+M to toggle mute
static CONST HOTKEY Hotkey2 = { 2, MOD_ALT | MOD_SHIFT, 0x4D }; // Alt+Shift+M to quit

#define THEME3

#if defined THEME1
static CONST INT FontSize = 16;
static CONST WCHAR FontName[] = L"Segoe UI";
static CONST WCHAR MutedText[] = L"●";
static CONST WCHAR UnmutedText[] = L"●";
static CONST COLORREF MutedColor = RGB(64, 255, 64); // Green
static CONST COLORREF UnmutedColor = RGB(255, 64, 64); // Red
static CONST UINT TextPosition = DT_RIGHT | DT_TOP;
#elif defined THEME2
static CONST INT FontSize = 64;
static CONST WCHAR FontName[] = L"Segoe UI";
static CONST WCHAR MutedText[] = L"|●";
static CONST WCHAR UnmutedText[] = L"●";
static CONST COLORREF MutedColor = RGB(64, 255, 64); // Green
static CONST COLORREF UnmutedColor = RGB(255, 64, 64); // Red
static CONST UINT TextPosition = DT_LEFT | DT_TOP;
#elif defined THEME3
static CONST INT FontSize = 64;
static CONST WCHAR FontName[] = L"Segoe UI";
static CONST WCHAR MutedText[] = L"🔇MUTED"; // Some cool alternatives: ⏺⏸︎🔇🎙
static CONST WCHAR UnmutedText[] = L"🎙LIVE";
static CONST COLORREF MutedColor = RGB(128, 128, 128); // Gray
static CONST COLORREF UnmutedColor = RGB(255, 64, 64); // Red
static CONST UINT TextPosition = DT_CENTER | DT_VCENTER;
#endif

static CONST UINT ShowDuration = 1000; // Milliseconds

static CONST DWORD WindowStyle = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
static CONST WPARAM TimerID = 1;

static HWND Window;
static RECT Rect;
static HDC Context;
static HBITMAP Bitmap;
static HFONT Font;
static HPEN Pen;
static WCHAR *Text; // Points to either MutedText or UnmutedText
static COLORREF *TextColor; // Points to either MutedColor or UnmutedColor

static VOID DrawProc(void)
{
    INT save = SaveDC(Context);
    FillRect(Context, &Rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
    SelectObject(Context, Font);
    SelectObject(Context, Pen);
    SetBkMode(Context, TRANSPARENT);
    SetTextColor(Context, TextColor);
    BeginPath(Context);
    DrawTextW(Context, Text, -1, &Rect, TextPosition | DT_SINGLELINE);
    EndPath(Context);
    StrokePath(Context);
    DrawTextW(Context, Text, -1, &Rect, TextPosition | DT_SINGLELINE);
    RestoreDC(Context, save);
}

static VOID ShowOSD(BOOL toggleMuted)
{
    // Check mic state (and toggle, if specified), then make OSD visible for 1 second
    if (AllMicrophonesMuted(toggleMuted)) {
        Text = MutedText;
        TextColor = MutedColor;
    } else {
        Text = UnmutedText;
        TextColor = UnmutedColor;
    }
    DrawProc();
    InvalidateRect(Window, &Rect, TRUE);
    ShowWindow(Window, SW_SHOW);
    SetTimer(Window, TimerID, ShowDuration, NULL);
    PAINTSTRUCT ps;
    if (BeginPaint(Window, &ps)) {
        BitBlt(ps.hdc, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, Context, 0, 0, SRCCOPY);
        EndPaint(Window, &ps);
    }
}

static VOID OnWindowCreate(HWND hwnd)
{
    Window = hwnd;
    // Clear all window styles for very plain window
    SetWindowLongW(Window, GWL_STYLE, 0);
    // Position at the center of the primary monitor
    MONITORINFO mi = {sizeof mi};
    GetMonitorInfoW(MonitorFromPoint((POINT){0, 0}, MONITOR_DEFAULTTOPRIMARY), &mi);
    // Asynchronously calls WM_SIZE:
    SetWindowPos(Window, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_HIDEWINDOW);
    // Create font & pen
    Font = CreateFontW(FontSize, 0, 0, 0, FW_EXTRABOLD, FALSE, FALSE, FALSE, 0, 0, 0, ANTIALIASED_QUALITY, 0, FontName);
    Pen = CreatePen(PS_SOLID, 4, RGB(1, 1, 1));

    if (TRUE) {
        // Make BLACK the transparency color
        SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 0, LWA_COLORKEY);
    } else {
        // I need to find the correct way to use UpdateLayeredWindow instead of SetLayeredWindowAttributes.
        // The reason is that drawing of layered windows work differently and may often briefly flash a
        // cached graphical output upon showing the window, which doesn't look too good.
        RECT cr;
        RECT wr;
        GetClientRect(hwnd, &cr);
        GetWindowRect(hwnd, &wr);
        int W = wr.right - wr.left;
        int H = wr.bottom - wr.top;
        HDC hsdc = GetDC(NULL); // Note that this is not GetDC(hwnd) because UpdateLayeredWindow requires DC of screen
        HDC hdc  = GetDC(hwnd);
        HDC hbdc = CreateCompatibleDC(hdc); // Should create from window DC rather than screen DC 
        HBITMAP bmp = CreateCompatibleBitmap(hsdc, W, H);
        HGDIOBJ oldbmp = SelectObject(hbdc, (HGDIOBJ)bmp);
        FillRect(hbdc, &cr, (HBRUSH)GetStockObject(BLACK_BRUSH));
        UpdateLayeredWindow(hwnd, hsdc, &(POINT){wr.left, wr.top}, &(SIZE){W, H}, hbdc, &(POINT){0, 0}, RGB(0, 0, 0), NULL, ULW_COLORKEY);
        SelectObject(hbdc, oldbmp);
        DeleteObject(bmp);
        DeleteDC(hbdc);
        ReleaseDC(hwnd, hdc);
        ReleaseDC(NULL, hsdc);
    }
    
    // Show OSD initially to inform mic state and indicate program started
    ShowOSD(FALSE);
}

static VOID OnWindowDestroy(void)
{
    DeleteObject(Bitmap);
    DeleteDC(Context);
    DeleteObject(Pen);
    DeleteObject(Font);
}

static VOID OnWindowResize(void)
{
    RECT rect;
    GetClientRect(Window, &rect);
    if (!EqualRect(&Rect, &rect)) {
        DeleteObject(Bitmap);
        DeleteDC(Context);
        HDC context = GetDC(Window);
        Rect = rect;
        Context = CreateCompatibleDC(context);
        Bitmap = CreateCompatibleBitmap(context, Rect.right - Rect.left, Rect.bottom - Rect.top);
        DeleteObject(SelectObject(Context, Bitmap));
        ReleaseDC(Window, context);
    }
}

static VOID OnHotkey(WPARAM id)
{
    if (id == Hotkey1.id) {
        ShowOSD(TRUE);
    }
    if (id == Hotkey2.id) {
        if (MessageBoxW(NULL, L"Do you want to quit mute?", L"mute", MB_OKCANCEL | MB_ICONINFORMATION | MB_SYSTEMMODAL) == IDOK) {
            PostMessageW(Window, WM_SYSCOMMAND, SC_CLOSE, 0);
        }
    }
}

static VOID OnTimer(WPARAM id)
{
    ShowWindow(Window, SW_HIDE);
    KillTimer(Window, id);
}

static LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
    case WM_CREATE:  OnWindowCreate(hwnd); break;
    case WM_DESTROY: OnWindowDestroy();    break;
    case WM_SIZE:    OnWindowResize();     break;
    case WM_HOTKEY:  OnHotkey(wparam);     break;
    case WM_TIMER:   OnTimer(wparam);      break;
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

INT wWinMain(HINSTANCE instance, HINSTANCE previnstance, WCHAR *args, INT show)
{
    static CONST WCHAR *class = L"StiansMuteUtilityClass";
    HANDLE mutex = CreateMutexW(NULL, 0, class);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (MessageBoxW(NULL, L"Do you want to quit mute?", L"mute", MB_OKCANCEL | MB_ICONINFORMATION | MB_SYSTEMMODAL) == IDOK) {
            PostMessageW(FindWindowExW(NULL, NULL, class, NULL), WM_SYSCOMMAND, SC_CLOSE, 0);
        }
        ExitProcess(0);
    }
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof wcex;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = instance;
    wcex.lpszClassName = class;
    MSG msg = {0};
    BOOL com = SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)); // Init COM
    HWND hwnd = CreateWindowExW(WindowStyle, RegisterClassExW(&wcex), NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, instance, NULL);
	BOOL hotkey = RegisterHotKey(hwnd, Hotkey1.id, Hotkey1.mod | MOD_NOREPEAT, Hotkey1.key) && RegisterHotKey(hwnd, Hotkey2.id, Hotkey2.mod | MOD_NOREPEAT, Hotkey2.key);
	if (com && hwnd && hotkey) while (GetMessageW(&msg, hwnd, 0, 0) > 0) DispatchMessageW(&msg);
	if (hotkey) UnregisterHotKey(hwnd, Hotkey1.id) && UnregisterHotKey(hwnd, Hotkey2.id);
	if (com) CoUninitialize();
	return msg.wParam;
}
