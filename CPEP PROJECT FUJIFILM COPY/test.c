#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
#pragma comment (lib,"Gdiplus.lib")

using namespace Gdiplus;

// Initialize GDI+ once at program start
ULONG_PTR gdiplusToken;
void InitGDIPlus() {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

void ShutdownGDIPlus() {
    GdiplusShutdown(gdiplusToken);
}

// ------------------- Print PNG Directly -------------------
void PrintLogoPNG(HDC hdcPrinter, const char* filename) {
    Bitmap* bmp = new Bitmap(LPCWSTR(filename));
    if (!bmp) return;

    Graphics graphics(hdcPrinter);

    // Get printer width
    int printerWidth = GetDeviceCaps(hdcPrinter, HORZRES);
    int printerHeight = GetDeviceCaps(hdcPrinter, VERTRES);

    // Scale image to fit printer width while keeping aspect ratio
    int imgWidth = bmp->GetWidth();
    int imgHeight = bmp->GetHeight();
    int newHeight = imgHeight * printerWidth / imgWidth;

    graphics.DrawImage(bmp, 0, 0, printerWidth, newHeight);

    delete bmp;
}

// ------------------- Print Text -------------------
void PrintCenteredText(HDC hdcPrinter, int y, const char* text, int fontHeight) {
    HFONT hFont = CreateFont(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdcPrinter, hFont);

    RECT rect = {0, y, GetDeviceCaps(hdcPrinter, HORZRES), y + fontHeight};
    SetBkMode(hdcPrinter, TRANSPARENT);
    SetTextColor(hdcPrinter, RGB(0,0,0));
    DrawText(hdcPrinter, text, -1, &rect, DT_CENTER | DT_SINGLELINE);

    SelectObject(hdcPrinter, hOldFont);
    DeleteObject(hFont);
}

// ------------------- Print Receipt -------------------
void PrintReceipt(const char* logoPath, const char* printerName, const char* barcode) {
    DOCINFO di = { sizeof(DOCINFO), "Receipt Print", NULL };
    HDC hdcPrinter = CreateDC(NULL, printerName, NULL, NULL);
    if (!hdcPrinter) return;

    StartDoc(hdcPrinter, &di);
    StartPage(hdcPrinter);

    // Print logo
    PrintLogoPNG(hdcPrinter, logoPath);

    int offsetY = 150; // space below logo
    PrintCenteredText(hdcPrinter, offsetY, "SHUTER DIGITAL", 24);
    offsetY += 30;
    PrintCenteredText(hdcPrinter, offsetY, "Photo Services", 20);
    offsetY += 25;

    // Print date/time
    SYSTEMTIME st;
    GetLocalTime(&st);
    char datetime[64];
    sprintf(datetime, "%02d/%02d/%04d %02d:%02d:%02d",
            st.wDay, st.wMonth, st.wYear,
            st.wHour, st.wMinute, st.wSecond);
    PrintCenteredText(hdcPrinter, offsetY, datetime, 16);
    offsetY += 25;

    // Print barcode (text-based for simplicity)
    char code[32];
    snprintf(code, sizeof(code), "*%s*", barcode); // Code39 format
    PrintCenteredText(hdcPrinter, offsetY, code, 40);

    EndPage(hdcPrinter);
    EndDoc(hdcPrinter);
    DeleteDC(hdcPrinter);
}

// ------------------- Main -------------------
int main() {
    InitGDIPlus();

    const char* logoPath = "C:\\path\\to\\logo.png"; // PNG file
    const char* printerName = "Your_Printer_Name";
    const char* barcode = "TP-1234";

    PrintReceipt(logoPath, printerName, barcode);

    ShutdownGDIPlus();
    return 0;
}
