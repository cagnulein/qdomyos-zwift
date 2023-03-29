#include "qtesseract.h"
#ifdef Q_OS_WIN
void qtesseract::captureWindow() {

    RECT rc;
    HWND hwnd = ::FindWindow(0, TEXT("Prompt dei comandi")); //::FindWindow(0,_T("ScreenCapture (Running) - Microsoft
                                                             //:Visual Studio"));//::FindWindow(0, _T("Calculator"));//=
                                                             //:FindWindow("Notepad", NULL);    //You get the ideal?
    if (hwnd == NULL) {
        return;
    }
    GetClientRect(hwnd, &rc);

    // create
    HDC hdcScreen = GetDC(NULL);
    HDC hdc = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmp = CreateCompatibleBitmap(hdcScreen, rc.right - rc.left, rc.bottom - rc.top);
    SelectObject(hdc, hbmp);

    // Print to memory hdc
    PrintWindow(hwnd, hdc, PW_CLIENTONLY);

    // copy to clipboard
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_BITMAP, hbmp);
    CloseClipboard();

    // release
    DeleteDC(hdc);
    DeleteObject(hbmp);
    ReleaseDC(NULL, hdcScreen);
}
#endif
