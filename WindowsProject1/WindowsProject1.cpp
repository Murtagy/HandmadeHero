#include <windows.h>
// #include <iostream>


LRESULT CALLBACK MainWindowCallback(
    HWND Window,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
) {
    LRESULT Result = 0;
    switch(Message) {
        case WM_SIZE:
        {
        OutputDebugStringA("RESIZE\n");
        } break;
        case WM_DESTROY: {} break;
        case WM_CLOSE: {} break;
        case WM_ACTIVATEAPP: {} break;
        default: {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    return Result;
}


// void foo(void) {   OutputDebugStringA("I am programming in C on Windows\n");}
int CALLBACK WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode
)
{

    WNDCLASS WindowClass = {};
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";
    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = CLASS_NAME;
    
    if (RegisterClass(&WindowClass)) {

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
            0
        );
        if (WindowHandle) {

            for (;;) {
                MSG Message = {};
                BOOL Result = GetMessage(&Message, 0, 0, 0);
                if (Result > 0) 
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else {break;}
            }
        
        } 
    }else {
    }
    return 0;
}


