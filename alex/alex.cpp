#include "framework.h"
#include "alex.h"
#include <CommCtrl.h>
#include <sstream>
#include <string>
#include <windows.h>

#define MAX_LOADSTRING 100
#define ID_MENU_LIST 101
#define ID_ADD_BUTTON 102
#define ID_INPUT_NAME 103
#define ID_INPUT_PRICE 104
#define ID_INPUT_PHONE 105
#define ID_ORDER_BUTTON 106
#define ID_VIEW_REPORT 107
#define ID_ADD_CLIENT_BUTTON 108
#define ID_INPUT_CLIENT_NAME 109
#define ID_INPUT_CLIENT_PHONE 110
#define ID_CLIENT_LIST 111
#define ID_ORDER_LIST 112  // Новый ID для списка заказов

// Идентификаторы для меток (пояснений)
#define ID_LABEL_NAME 111
#define ID_LABEL_PRICE 112
#define ID_LABEL_CLIENT_NAME 113
#define ID_LABEL_CLIENT_PHONE 114



// Глобальные переменные:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

HWND hMenuList, hAddButton, hOrderButton, hInputName, hInputPrice, hInputPhone,
hAddClientButton, hClientList, hInputClientName, hInputClientPhone, hOrderList; // Добавлен новый элемент для списка заказов

// Структуры данных:
struct MenuItem {
    wchar_t name[50];
    int price;
};

struct Client {
    wchar_t name[50];
    wchar_t phone[20];
    int totalSpent;
    int discount;
};

struct Order {
    Client* client;
    MenuItem* items[10];
    int itemCount;
    int totalPrice;
    SYSTEMTIME orderTime;
};

MenuItem menu[10];
Client clients[10];
Order orders[10];
int menuCount = 0;
int clientCount = 0;
int orderCount = 0;

// Прототипы функций:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
void AddColumn();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void AddMenuItem(HWND hWnd);
void AddClientFromUI(HWND hWnd);
void CreateOrder(HWND hWnd);
void UpdateClientList();
void UpdateOrderList();  // Функция обновления списка заказов

// Функции добавления данных:
void AddClient(Client& client, const wchar_t* name, const wchar_t* phone) {
    wcscpy_s(client.name, name);
    wcscpy_s(client.phone, phone);
    client.totalSpent = 0;
    client.discount = 0;
}

void AddOrder(Order& order, Client* client, MenuItem* items[], int itemCount) {
    order.client = client;
    order.itemCount = itemCount;
    int total = 0;
    for (int i = 0; i < itemCount; i++) {
        total += items[i]->price;
    }
    order.totalPrice = total - (total * client->discount / 100);
    GetLocalTime(&order.orderTime);
    client->totalSpent += order.totalPrice;
}
void AddMenuItem(HWND hWnd) {
    wchar_t name[50];
    wchar_t priceText[10];
    GetWindowText(hInputName, name, 50);
    GetWindowText(hInputPrice, priceText, 10);

    if (wcslen(name) == 0 || wcslen(priceText) == 0) {
        MessageBox(hWnd, L"Заполните все поля!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    int price = _wtoi(priceText);
    if (price <= 0) {
        MessageBox(hWnd, L"Цена должна быть больше 0!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    if (menuCount >= 10) {
        MessageBox(hWnd, L"Превышен лимит элементов меню!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    wcscpy_s(menu[menuCount].name, name);
    menu[menuCount].price = price;
    menuCount++;

    // Обновление списка меню после добавления нового блюда
    ListView_DeleteAllItems(hMenuList); // Очистка старых элементов

    LVITEM lvItem = {};
    lvItem.mask = LVIF_TEXT;
    for (int i = 0; i < menuCount; i++) {
        lvItem.iItem = i;
        lvItem.pszText = menu[i].name;
        ListView_InsertItem(hMenuList, &lvItem);  // Вставка элемента

        std::wstringstream ws;
        ws << menu[i].price;
        std::wstring priceStr = ws.str();
        ListView_SetItemText(hMenuList, i, 1, const_cast<LPWSTR>(priceStr.c_str())); // Установка текста в колонку 1
    }

    SetWindowText(hInputName, L"");  // Очистить поле ввода
    SetWindowText(hInputPrice, L"");
}



void AddClientFromUI(HWND hWnd) {
    wchar_t name[50];
    wchar_t phone[20];
    GetWindowText(hInputClientName, name, 50);  // Получаем имя клиента
    GetWindowText(hInputClientPhone, phone, 20);  // Получаем телефон клиента

    // Проверка на пустые поля
    if (wcslen(name) == 0 || wcslen(phone) == 0) {
        MessageBox(hWnd, L"Заполните все поля!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Создаем нового клиента и добавляем его в список клиентов
    if (clientCount >= 10) {
        MessageBox(hWnd, L"Превышен лимит клиентов!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    Client newClient;
    wcscpy_s(newClient.name, name);
    wcscpy_s(newClient.phone, phone);
    newClient.totalSpent = 0;
    newClient.discount = 0;
    clients[clientCount++] = newClient;

    // Обновляем список клиентов
    UpdateClientList();
    SetWindowText(hInputClientName, L"");
    SetWindowText(hInputClientPhone, L"");
    MessageBox(hWnd, L"Клиент добавлен.", L"Успех", MB_OK | MB_ICONINFORMATION);
    
}

void UpdateClientList() {
    ListView_DeleteAllItems(hClientList);
    LVITEM lvItem = {};
    lvItem.mask = LVIF_TEXT;
    for (int i = 0; i < clientCount; i++) {
        lvItem.iItem = i;
        lvItem.pszText = clients[i].name;
        ListView_InsertItem(hClientList, &lvItem);

        std::wstringstream ws;
        ws << clients[i].phone;
        std::wstring phoneStr = ws.str();
        ListView_SetItemText(hClientList, i, 1, const_cast<LPWSTR>(phoneStr.c_str()));
    }
}

void UpdateOrderList() {
    ListView_DeleteAllItems(hOrderList);
    LVITEM lvItem = {};
    lvItem.mask = LVIF_TEXT;

    for (int i = 0; i < orderCount; i++) {
        lvItem.iItem = i;
        lvItem.pszText = orders[i].client->name;
        ListView_InsertItem(hOrderList, &lvItem);

        std::wstringstream ws;
        ws << orders[i].totalPrice;
        std::wstring priceStr = ws.str();
        ListView_SetItemText(hOrderList, i, 1, const_cast<LPWSTR>(priceStr.c_str()));
    }
}

void CreateOrder(HWND hWnd) {
    int selectedItemCount = ListView_GetSelectedCount(hMenuList);
    if (selectedItemCount == 0) {
        MessageBox(hWnd, L"Выберите хотя бы одно блюдо!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Собираем список выбранных блюд для заказа
    MenuItem* selectedItems[10];
    int itemCount = 0;
    for (int i = 0; i < menuCount; i++) {
        if (ListView_GetItemState(hMenuList, i, LVIS_SELECTED) == LVIS_SELECTED) {
            selectedItems[itemCount++] = &menu[i];  // Добавляем выбранное блюдо
        }
    }

    // Выбираем клиента для заказа
    Client* client = nullptr;
    for (int i = 0; i < clientCount; i++) {
        if (ListView_GetItemState(hClientList, i, LVIS_SELECTED) == LVIS_SELECTED) {
            client = &clients[i];  // Находим выбранного клиента
            break;
        }
    }

    if (!client) {
        MessageBox(hWnd, L"Выберите клиента!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Создаем заказ
    if (orderCount < 10) {
        Order newOrder;
        AddOrder(newOrder, client, selectedItems, itemCount);
        orders[orderCount++] = newOrder;

        // Обновляем список заказов
        UpdateOrderList();

        MessageBox(hWnd, L"Заказ успешно создан.", L"Успех", MB_OK | MB_ICONINFORMATION);
    }
}

// Реализация оконного класса и основной функции:

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;
    LoadString(hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInst, IDC_ALEX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, hInstance, NULL);
    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}







void AddColumn()
{
    LV_COLUMN lvColumn = {};
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    // Колонки для меню
    wchar_t menuName[] = L"Название";  // Используем временную строку
    lvColumn.pszText = menuName;  // Присваиваем LPWSTR
    lvColumn.cx = 150;
    ListView_InsertColumn(hMenuList, 0, &lvColumn);

    wchar_t menuPrice[] = L"Цена";  // Используем временную строку
    lvColumn.pszText = menuPrice;  // Присваиваем LPWSTR
    lvColumn.cx = 100;
    ListView_InsertColumn(hMenuList, 1, &lvColumn);

    // Колонки для клиентов
    wchar_t clientName[] = L"Имя";  // Используем временную строку
    lvColumn.pszText = clientName;  // Присваиваем LPWSTR
    lvColumn.cx = 150;
    ListView_InsertColumn(hClientList, 0, &lvColumn);

    wchar_t clientPhone[] = L"Телефон";  // Используем временную строку
    lvColumn.pszText = clientPhone;  // Присваиваем LPWSTR
    lvColumn.cx = 150;
    ListView_InsertColumn(hClientList, 1, &lvColumn);

    // Колонки для заказов
    wchar_t orderClient[] = L"Клиент";  // Используем временную строку
    lvColumn.pszText = orderClient;  // Присваиваем LPWSTR
    lvColumn.cx = 150;
    ListView_InsertColumn(hOrderList, 0, &lvColumn);

    wchar_t orderPrice[] = L"Цена";  // Используем временную строку
    lvColumn.pszText = orderPrice;  // Присваиваем LPWSTR
    lvColumn.cx = 100;
    ListView_InsertColumn(hOrderList, 1, &lvColumn);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
   

    // Инициализация всех строковых переменных
    wchar_t nameColumn[] = L"Название";
    wchar_t priceColumn[] = L"Цена";
    wchar_t clientColumn[] = L"Имя";
    wchar_t phoneColumn[] = L"Телефон";
    wchar_t orderClientColumn[] = L"Клиент";
    wchar_t orderSumColumn[] = L"Сумма";



    // Добавление колонок в ListView
    

    static HFONT hFont = NULL; // Инициализация шрифта
    
    switch (message) {


    case WM_CREATE:
    
        // Инициализация шрифта
        hFont = CreateFont(
            18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial"
        );

        // Создание ListView для отображения меню, клиентов и заказов
        hMenuList = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
            10, 10, 400, 200, hWnd, (HMENU)ID_MENU_LIST, hInst, nullptr);
        hClientList = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
            10, 220, 400, 200, hWnd, (HMENU)ID_CLIENT_LIST, hInst, nullptr);
        hOrderList = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS,
            10, 430, 400, 200, hWnd, (HMENU)ID_ORDER_LIST, hInst, nullptr);
        ListView_SetExtendedListViewStyle(hMenuList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        ListView_SetExtendedListViewStyle(hClientList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        ListView_SetExtendedListViewStyle(hOrderList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        
        AddColumn();

        // Поля ввода и кнопки
        hInputName = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 420, 70, 200, 25, hWnd, (HMENU)ID_INPUT_NAME, hInst, nullptr);
        hInputPrice = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 420, 120, 200, 25, hWnd, (HMENU)ID_INPUT_PRICE, hInst, nullptr);
        hInputClientName = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 420, 220, 200, 25, hWnd, (HMENU)ID_INPUT_CLIENT_NAME, hInst, nullptr);
        hInputClientPhone = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 420, 270, 200, 25, hWnd, (HMENU)ID_INPUT_CLIENT_PHONE, hInst, nullptr);

        CreateWindow(L"STATIC", L"Введите название блюда", WS_CHILD | WS_VISIBLE, 420, 50, 200, 20, hWnd, (HMENU)ID_LABEL_NAME, hInst, nullptr);
        CreateWindow(L"STATIC", L"Введите цену", WS_CHILD | WS_VISIBLE, 420, 100, 200, 20, hWnd, (HMENU)ID_LABEL_PRICE, hInst, nullptr);
        CreateWindow(L"STATIC", L"Введите имя клиента", WS_CHILD | WS_VISIBLE, 420, 200, 200, 20, hWnd, (HMENU)ID_LABEL_CLIENT_NAME, hInst, nullptr);
        CreateWindow(L"STATIC", L"Введите телефон клиента", WS_CHILD | WS_VISIBLE, 420, 250, 200, 20, hWnd, (HMENU)ID_LABEL_CLIENT_PHONE, hInst, nullptr);

        hAddButton = CreateWindow(L"BUTTON", L"Добавить блюдо", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 630, 90, 150, 40, hWnd, (HMENU)ID_ADD_BUTTON, hInst, nullptr);
        hAddClientButton = CreateWindow(L"BUTTON", L"Добавить клиента", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 630, 220, 150, 40, hWnd, (HMENU)ID_ADD_CLIENT_BUTTON, hInst, nullptr);
        hOrderButton = CreateWindow(L"BUTTON", L"Создать заказ", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 420, 320, 200, 40, hWnd, (HMENU)ID_ORDER_BUTTON, hInst, nullptr);

        // Установка шрифта для всех элементов
        if (hFont) {
            SendMessage(hAddButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hAddClientButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hOrderButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hInputName, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hInputPrice, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hInputClientName, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hInputClientPhone, WM_SETFONT, (WPARAM)hFont, TRUE);
        }
        break;
        break;
        

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case ID_ADD_BUTTON:
            AddMenuItem(hWnd);
            break;
        case ID_ADD_CLIENT_BUTTON:
            AddClientFromUI(hWnd);
            break;
        case ID_ORDER_BUTTON:
            CreateOrder(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_SIZE:
        // Вызываем функцию для перерисовки окна ListView, если нужно
        InvalidateRect(hWnd, NULL, TRUE);
        break;


    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    InitInstance(hInstance, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
//Я ЮЗАЮ ЧАТ ГОПОТУ ДЛЯ ТОГО ЧТОБЫ ЭТО НАПИСАТЬ, ТАК ОТ ЧЕГО ЖЕ ВЫ С ЭТИМ НЕ СПРАВИЛИСЬ, ТОВАРИЩИ?) 
// //ВСЕГО 8 ЧАСОВ РАБОТЫ У МЕНЯ ЭТО ЗАНЯЛО 
// 
// /////////////////////////////////////////////////////
// //450 СТРОЧЕК ПОЧТИ, В СОВЕРШЕННО НЕ ЗНАКОМОЙ БИБЛИОТЕКЕ(Я ПИШУ НА DART\FLUTTER ТАКУЮ СОФТИНУ)[ТОЛЬКО ПРИ ИО НЕ УПОМИНАЙТЕ ЭТО, А ТО НАС ВСЕХ ПОВЕСЯТ]
// //ВОТ ТЕПЕРЬ ДУМООООЙТЕ //ЖДУ СВОЁ СПАСИБО НА КАСПИ +77471142005 =)