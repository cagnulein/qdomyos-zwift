#include "qtesseract.h"
#ifdef Q_OS_WIN

#include <Windows.h>
#include <fstream>
#include <iostream>

int WriteBitmapFile(PBITMAPFILEHEADER pFileHeader, PBITMAPINFOHEADER pInfoHeader, PBYTE pImageData,
                    DWORD dwImageDataSize) {
    /*
    The format of a .bmp file is:
    1) File header
    2) Info header
    3) Raw bytes (representing the pixels)
    (https://en.wikipedia.org/wiki/BMP_file_format)
    */

    std::ofstream fout;
    fout.open("temp.bmp", std::ios::out | std::ios::binary);

    if (!fout) {
        printf("ofstream::open failed (err %d)\n", GetLastError());
        return 1;
    }

    fout.write((char *)(pFileHeader), sizeof(BITMAPFILEHEADER));
    fout.write((char *)(pInfoHeader), sizeof(BITMAPINFOHEADER));
    fout.write((char *)(pImageData), dwImageDataSize);
    fout.close();

    return 0;
}

int SaveFromBitmapInfo(PBITMAPINFO pBitmapInfo) {
    // This program currently doesn't support other types of compression
    if (pBitmapInfo->bmiHeader.biCompression != BI_RGB) {
        printf("[!] Bitmap compression is different from BI_RGB\n");
        return 0;
    }

    BITMAPFILEHEADER bmfh = {0};

    bmfh.bfType = 0x4D42;                                                 // BMP magic
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); // 14 and 40 bytes respectively
    bmfh.bfSize = bmfh.bfOffBits + pBitmapInfo->bmiHeader.biWidth * pBitmapInfo->bmiHeader.biHeight *
                                       pBitmapInfo->bmiHeader.biBitCount / 8;

    return WriteBitmapFile(&bmfh, &pBitmapInfo->bmiHeader, (PBYTE)&pBitmapInfo->bmiColors,
                           bmfh.bfSize - bmfh.bfOffBits);
}

int SaveBitmapFromClipboard() {
    HANDLE hGlobal = NULL;
    int rv = 1;

    if (!IsClipboardFormatAvailable(CF_DIB)) {
        printf("[!] CF_DIB is not an avialable format\n");
        return 1;
    }
    printf("[*] BITMAPINFO format is available\n");

    if (!OpenClipboard(NULL)) {
        printf("[!] OpenClipboard failed (err %d)\n", GetLastError());
        return 1;
    }

    hGlobal = GetClipboardData(CF_DIB);
    if (hGlobal == NULL) {
        printf("[!] GetClipboardData failed (err %d)\n", GetLastError());
        return 1;
    }

    PBITMAPINFO pClipboardData = (PBITMAPINFO)GlobalLock(hGlobal);
    if (pClipboardData == NULL) {
        printf("[!] GlobalLock failed (err %d)\n", GetLastError());
        return 1;
    }

    rv = SaveFromBitmapInfo(pClipboardData);

    GlobalUnlock(hGlobal);
    if (!CloseClipboard()) {
        printf("[!] CloseClipboard failed (err %d)\n", GetLastError());
        return 1;
    }

    return rv;
}

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

    SaveBitmapFromClipboard();

    // release
    DeleteDC(hdc);
    DeleteObject(hbmp);
    ReleaseDC(NULL, hdcScreen);
}
#endif
