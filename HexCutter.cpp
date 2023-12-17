#include <windows.h>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

using namespace std;

typedef struct ImgInfo {
    vector<COLORREF>* pixels;
    int width;
    int height;
};

ImgInfo* readPixelColorsFromFile(const std::string& filename) {
    ifstream file(filename, ios::binary);
    vector<COLORREF>* pixelColors = new vector<COLORREF>();

    if (file.is_open()) {
        char* header = new char[54];
        file.read(header, 54);

        int width = *(int*)&header[18];
        int height = *(int*)&header[22];

        int dataSize = width * height * 3;

        char* data = new char[dataSize];

        file.read(data, dataSize);

        COLORREF backgroundColor = RGB((unsigned char)data[2], (unsigned char)data[1], (unsigned char)data[0]);

        for (int i = 0; i < dataSize; i += 3) {
            COLORREF color = RGB((unsigned char)data[i + 2], (unsigned char)data[i + 1], (unsigned char)data[i]);
            pixelColors->push_back(color);
        }

        delete[] data;
        delete[] header;

        file.close();

        for (COLORREF& color : *pixelColors) {
            if (color == backgroundColor) {
                color = RGB(255, 255, 255);
            }
        }

        ImgInfo* info = (ImgInfo*)malloc(sizeof(ImgInfo));
        info->pixels = pixelColors;
        info->height = height;
        info->width = width;

        return info;
    }
}


void drawShape(const ImgInfo* info, HDC hdcWindow, HWND hwnd) {
    auto pictWidth = info->width;
    auto pictHeight = info->height;
    auto pixelColors = info->pixels;

    RECT windowRect;
    GetClientRect(hwnd, &windowRect);
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    for (int x = 0; x < pictWidth; ++x) {
        for (int y = 0; y < pictHeight; ++y) {
            SetPixel(hdcWindow, pictWidth - x + windowHeight / 15, y + windowHeight - pictHeight - windowHeight / 15, pixelColors->at(pixelColors->size() - (1 + pictWidth * y + x)));
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZING: { InvalidateRect(hwnd, NULL, TRUE); }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        string filename = "D:\\proj\\WINAPI-VC--\\8.bmp";
        auto info = readPixelColorsFromFile(filename);

        int height = LOWORD(lParam);
        int width = HIWORD(lParam);
        PaintRgn(hdc, CreateRectRgn(0, 0, width, height));
        drawShape(info, hdc, hwnd);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"My Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"BMP Image drawer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}