#include "framework.h"
#include "Alexc.h"
#include <CommCtrl.h>

#define MAX_LOADSTRING 100
#define ID_MENU_LIST 101
#define ID_ADD_BUTTON 102
#define ID_INPUT_NAME 103
#define ID_INPUT_PRICE 104

// Глобальные переменные:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

HWND hMenuList;  // Список меню
HWND hInputName; // Поле для ввода имени блюда
HWND hInputPrice; // Поле для ввода цены блюда
HWND hAddButton; // Кнопка добавления

// Прототипы функций:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Функция добавления блюда в меню
void AddMenuItem(HWND hWnd) {
    wchar_t name[50];
    wchar_t priceText[10];
    GetWindowText(hInputName, name, 50);
    GetWindowText(hInputPrice, priceText, 10);

    // Проверка на корректность
    if (wcslen(name) == 0 || wcslen(priceText) == 0) {
        MessageBox(hWnd, L"Заполните все поля!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    int price = _wtoi(priceText);
    if (price <= 0) {
        MessageBox(hWnd, L"Цена должна быть больше 0!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Добавляем элемент в список
    LVITEM lvItem = {};
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = ListView_GetItemCount(hMenuList);
    lvItem.pszText = name;
    ListView_InsertItem(hMenuList, &lvItem);

    ListView_SetItemText(hMenuList, lvItem.iItem, 1, priceText);

    // Очистка полей ввода
    SetWindowText(hInputName, L"");
    SetWindowText(hInputPrice, L"");
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ALEXC, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ALEXC));

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ALEXC));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ALEXC);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 600, 400, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    // Создаем элементы интерфейса
    hMenuList = CreateWindowW(WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT,
        10, 10, 400, 200, hWnd, (HMENU)ID_MENU_LIST, hInstance, nullptr);

    // Настройка колонок списка
    LVCOLUMN lvColumn = {};
    lvColumn.mask = LVCF_WIDTH | LVCF_TEXT;
    lvColumn.cx = 200;
    lvColumn.pszText = L"Блюдо";
    ListView_InsertColumn(hMenuList, 0, &lvColumn);

    lvColumn.cx = 100;
    lvColumn.pszText = L"Цена";
    ListView_InsertColumn(hMenuList, 1, &lvColumn);

    // Поля ввода
    CreateWindowW(L"STATIC", L"Название:", WS_CHILD | WS_VISIBLE, 10, 220, 80, 20, hWnd, nullptr, hInstance, nullptr);
    hInputName = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 100, 220, 150, 20, hWnd, (HMENU)ID_INPUT_NAME, hInstance, nullptr);

    CreateWindowW(L"STATIC", L"Цена:", WS_CHILD | WS_VISIBLE, 10, 250, 80, 20, hWnd, nullptr, hInstance, nullptr);
    hInputPrice = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 100, 250, 150, 20, hWnd, (HMENU)ID_INPUT_PRICE, hInstance, nullptr);

    // Кнопка добавления
    hAddButton = CreateWindowW(L"BUTTON", L"Добавить", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 280, 100, 30, hWnd, (HMENU)ID_ADD_BUTTON, hInstance, nullptr);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_ADD_BUTTON) {
            AddMenuItem(hWnd);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    } break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
