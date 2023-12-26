#include <windows.h>
#include <stdint.h>
// #include <iostream>

#define internal static
#define local_persist static
#define global_variable static



global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;

global_variable int BytesPerPixel = 4;


internal void RenderWeirdGradient(int xOffset, int yOffset) {
    int Width = BitmapWidth;
    int Height = BitmapHeight;

    int Pitch = Width * BytesPerPixel;
    uint8_t* Row = (uint8_t*)BitmapMemory;
    for (int Y = 0;
        Y < BitmapHeight;
        ++Y)
    {
        uint32_t* Pixel = (uint32_t*)Row;
        for (int X = 0; X < BitmapWidth; ++X)
        {

           //uint8_t Blue = (X + xOffset);
           //uint8_t Green = (Y + yOffset);
           // mem: BB GG RR xx
           uint8_t Blue = X + xOffset;
           uint8_t Green = Y + yOffset;
           uint8_t Red = X - Y;


           *Pixel++ = (Blue | (Green << 8) | (Red << 16));
        }

        Row += Pitch;
    }
}

/*
Pixel in memory: BB GG RR xx
Little endian architecture - most significant byte first
0x xxRRGGBB


*/
/*

*/

internal void
WinResizeDIBSection(int Width, int Height)
{
    if (BitmapMemory) {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    int BytesPerPixel = 4;
    int BitmapMemorySize = BytesPerPixel * Width * Height;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    RenderWeirdGradient(128, 0);
}

internal void
WinUpdateWindow(HDC DeviceContext, RECT *ClientRect, int x, int y, int w, int h)
{
    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;
    StretchDIBits(
        DeviceContext,
        // x, y, w, h,
        // x, y, w, h,
        0, 0, BitmapWidth, BitmapHeight,
        0,0, WindowWidth, WindowHeight,
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
        OutputDebugStringA("Paint\n");
        PAINTSTRUCT Paint;
        HDC DevideContext = BeginPaint(Window, &Paint);
        int X = Paint.rcPaint.left;
        int w = Paint.rcPaint.right - X;
        int Y = Paint.rcPaint.top;
        int h = Paint.rcPaint.bottom - Y;

        RECT ClientRect;
        GetClientRect(Window, &ClientRect);

        WinUpdateWindow(DevideContext, &ClientRect, X, Y, w, h);
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

        HWND Window = CreateWindowEx(
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
        if (Window)
        {
            Running = true;
            int xOffset = 0;
            int yOffset = 0;
            while (Running)
            {
                MSG Message = {};
                while ( PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                HDC DeviceContext = GetDC(Window);
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                RenderWeirdGradient(xOffset, yOffset);
                WinUpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                ++xOffset;
            }
        }
    }
    else
    {
    }
    return 0;
}
