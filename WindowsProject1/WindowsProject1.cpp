#include <windows.h>
// #include <iostream>

#define internal static
#define local_persist static
#define global_variable static

global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void
WinResizeDIBSection(int Width, int Height)
{
    // todo free

    if (BitmapHandle)
    {
        DeleteObject(BitmapHandle);
    }
    if (!BitmapDeviceContext)
    {
        BitmapDeviceContext = CreateCompatibleDC(0);
    }

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    BitmapHandle = CreateDIBSection(
        BitmapDeviceContext, &BitmapInfo,
        DIB_RGB_COLORS,
        &BitmapMemory,
        0,
        0);
}

internal void
WinUpdateWindow(HDC DeviceContext, int x, int y, int w, int h)
{
    StretchDIBits(
        DeviceContext,
        x, y, w, h,
        x, y, w, h,
        BitmapMemory,
        &BitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY // DWORD dwRop
    );
}

LRESULT CALLBACK WinMainWindowCallback(
    HWND Window,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam)
{
    LRESULT Result = 0;
    switch (Message)
    {
    case WM_SIZE:
    {
        RECT ClientRect;
        GetClientRect(Window, &ClientRect);
        int Width = ClientRect.right - ClientRect.left;
        int Height = ClientRect.bottom - ClientRect.top;
        WinResizeDIBSection(Width, Height);
        OutputDebugStringA("RESIZE\n");
    }
    break;
    case WM_DESTROY:
    {
        OutputDebugStringA("DESTROY\n");
        Running = false;
    }
    break;
    case WM_CLOSE:
    {
        OutputDebugStringA("CLOSE\n");
        Running = false;
    }
    break;
    case WM_ACTIVATEAPP:
    {
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT Paint;
        HDC DevideContext = BeginPaint(Window, &Paint);
        int X = Paint.rcPaint.left;
        int w = Paint.rcPaint.right - X;
        int Y = Paint.rcPaint.top;
        int h = Paint.rcPaint.bottom - Y;
        WinUpdateWindow(DevideContext, X, Y, w, h);
        EndPaint(Window, &Paint);
    }
    break;
    default:
    {
        Result = DefWindowProc(Window, Message, WParam, LParam);
    }
    break;
    }
    return Result;
}

// void foo(void) {   OutputDebugStringA("I am programming in C on Windows\n");}
int CALLBACK WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode)
{

    WNDCLASS WindowClass = {};
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WinMainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = CLASS_NAME;

    if (RegisterClass(&WindowClass))
    {

        HWND WindowHandle = CreateWindowEx(
            0,
            CLASS_NAME,
            L"My window",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            Instance,
            0);
        if (WindowHandle)
        {
            Running = true;
            while (Running)
            {
                MSG Message = {};
                BOOL Result = GetMessage(&Message, 0, 0, 0);
                if (Result > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else
                {
                    break;
                }
            }
        }
    }
    else
    {
    }
    return 0;
}
