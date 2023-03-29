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
    HWND hwnd = ::FindWindow(0, TEXT("Zwift")); //::FindWindow(0,_T("ScreenCapture (Running) - Microsoft
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

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        fprintf(stderr, "Could not initialize tesseract.\n");
        exit(1);
    }
    // Open input image with leptonica library
    Pix *image = pixRead("temp.bmp");
    api->SetImage(image);
    // Set lstm_choice_mode to alternative symbol choices per character, bbox is at word level.
    api->SetVariable("lstm_choice_mode", "2");
    api->Recognize(0);
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
    tesseract::ResultIterator *res_it = api->GetIterator();
    // Get confidence level for alternative symbol choices. Code is based on
    // https://github.com/tesseract-ocr/tesseract/blob/a7a729f6c315e751764b72ea945da961638effc5/src/api/hocrrenderer.cpp#L325-L344
    std::vector<std::vector<std::pair<const char *, float>>> *choiceMap = nullptr;
    if (res_it != 0) {
        do {
            const char *word;
            float conf;
            int x1, y1, x2, y2, tcnt = 1, gcnt = 1, wcnt = 0;
            res_it->BoundingBox(level, &x1, &y1, &x2, &y2);
            choiceMap = res_it->GetBestLSTMSymbolChoices();
            for (auto timestep : *choiceMap) {
                if (timestep.size() > 0) {
                    for (auto &j : timestep) {
                        conf = int(j.second * 100);
                        word = j.first;
                        printf("%d  symbol: '%s';  \tconf: %.2f; BoundingBox: %d,%d,%d,%d;\n", wcnt, word, conf, x1, y1,
                               x2, y2);
                        gcnt++;
                    }
                    tcnt++;
                }
                wcnt++;
                printf("\n");
            }
        } while (res_it->Next(level));
    }
    // Destroy used object and release memory
    api->End();
    pixDestroy(&image);

    // release
    DeleteDC(hdc);
    DeleteObject(hbmp);
    ReleaseDC(NULL, hdcScreen);
}
#endif
