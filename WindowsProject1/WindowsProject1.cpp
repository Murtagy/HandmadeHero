#include <windows.h>
#include <stdint.h>
// #include <iostream>

#define internal static
#define local_persist static
#define global_variable static

struct win_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

global_variable bool Running;
global_variable win_offscreen_buffer GlobalBackBuffer;

struct win_window_dimensions
{
    int Width;
    int Height;
};

win_window_dimensions GetWindowDimensions(HWND Window)
{
    win_window_dimensions Res;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Res.Width = ClientRect.right - ClientRect.left;
    Res.Height = ClientRect.bottom - ClientRect.top;
    return (Res);
}

internal void RenderWeirdGradient(win_offscreen_buffer Buffer, int xOffset, int yOffset)
{

    int Pitch = Buffer.Width * Buffer.BytesPerPixel;
    uint8_t *Row = (uint8_t *)Buffer.Memory;
    for (int Y = 0;
         Y < Buffer.Height;
         ++Y)
    {
        uint32_t *Pixel = (uint32_t *)Row;
        for (int X = 0; X < Buffer.Width; ++X)
        {

            // uint8_t Blue = (X + xOffset);
            // uint8_t Green = (Y + yOffset);
            //  mem: BB GG RR xx
            uint8_t Blue = X + xOffset;
            uint8_t Green = Y + yOffset;
            uint8_t Red = X - Y;

            *Pixel++ = (Blue | (Green << 8) | (Red << 16));
        }

        Row += Buffer.Pitch;
    }
}

/*
Pixel in memory: BB GG RR xx
Little endian architecture - most significant byte first
0x xxRRGGBB


*/

internal void
WinResizeDIBSection(win_offscreen_buffer *Buffer, int Width, int Height)
{
    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

internal void
WinDisplayBufferInWindow(HDC DeviceContext, int WindowWidth, int WindowHeight, win_offscreen_buffer Buffer, int x, int y, int w, int h)
{
    // TODO: aspect ratio
    StretchDIBits(
        DeviceContext,
        // x, y, w, h,
        // x, y, w, h,
        0, 0, WindowWidth, WindowHeight,
        0, 0, Buffer.Width, Buffer.Height,
        Buffer.Memory,
        &Buffer.Info,
        DIB_RGB_COLORS,
        SRCCOPY);
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

        win_window_dimensions Dimensions = GetWindowDimensions(Window);
        WinDisplayBufferInWindow(DevideContext, Dimensions.Width, Dimensions.Height, GlobalBackBuffer, X, Y, w, h);
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
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
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

            win_window_dimensions Dimensions = GetWindowDimensions(Window);
            WinResizeDIBSection(&GlobalBackBuffer, 1280, 720);
            while (Running)
            {
                MSG Message = {};
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                HDC DeviceContext = GetDC(Window);
                win_window_dimensions Dimensions = GetWindowDimensions(Window);
                RenderWeirdGradient(GlobalBackBuffer, xOffset, yOffset);
                WinDisplayBufferInWindow(DeviceContext, Dimensions.Width, Dimensions.Height, GlobalBackBuffer, 0, 0, Dimensions.Width, Dimensions.Height);

                ++xOffset;
            }
        }
    }
    else
    {
    }
    return 0;
}
