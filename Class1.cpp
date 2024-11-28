#include "framework.h"  // Подключение заголовка для работы с Windows API
#include "alex.h"       // Пользовательский заголовок (предположительно, для определения ID и других констант)
#include <CommCtrl.h>   // Для работы с компонентами Windows, такими как ListView
#include <sstream>      // Для работы с потоками строк
#include <string>       // Для работы со строками
#include <windows.h>    // Основной заголовок Windows API

// Максимальная длина строк для заголовков и имен
#define MAX_LOADSTRING 100

// Идентификаторы для элементов управления
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
HINSTANCE hInst;  // Хэндл текущего экземпляра приложения
WCHAR szTitle[MAX_LOADSTRING];  // Заголовок окна
WCHAR szWindowClass[MAX_LOADSTRING];  // Имя класса окна

// Хэндлы элементов управления
HWND hMenuList, hAddButton, hOrderButton, hInputName, hInputPrice, hInputPhone,
hAddClientButton, hClientList, hInputClientName, hInputClientPhone, hOrderList; // Добавлен новый элемент для списка заказов

// Структуры данных для хранения информации о меню, клиентах и заказах
struct MenuItem
{
    wchar_t name[50];  // Название блюда
    int price;         // Цена блюда
};

struct Client
{
    wchar_t name[50];      // Имя клиента
    wchar_t phone[20];     // Телефон клиента
    int totalSpent;        // Общая потраченная сумма
    int discount;          // Скидка клиента в процентах
};

struct Order
{
    Client* client;        // Клиент, сделавший заказ
    MenuItem* items[10];   // Список заказанных блюд
    int itemCount;         // Количество заказанных блюд
    int totalPrice;        // Общая стоимость заказа с учетом скидки
    SYSTEMTIME orderTime;  // Время заказа
};

// Массивы для хранения данных
MenuItem menu[10];  // Массив для блюд меню
Client clients[10];  // Массив для клиентов
Order orders[10];   // Массив для заказов
int menuCount = 0;  // Количество элементов в меню
int clientCount = 0;  // Количество клиентов
int orderCount = 0;   // Количество заказов

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

// Функция для добавления клиента
void AddClient(Client& client, const wchar_t* name, const wchar_t* phone)
{
    wcscpy_s(client.name, name);   // Копирование имени клиента
    wcscpy_s(client.phone, phone); // Копирование номера телефона
    client.totalSpent = 0;         // Изначально клиент ничего не потратил
    client.discount = 0;           // Изначально скидка равна 0
}

// Функция для добавления заказа
void AddOrder(Order& order, Client* client, MenuItem* items[], int itemCount)
{
    order.client = client;         // Привязываем клиента к заказу
    order.itemCount = itemCount;   // Количество блюд в заказе
    int total = 0;
    // Вычисляем стоимость заказа без скидки
    for (int i = 0; i < itemCount; i++)
    {
        total += items[i]->price;
    }
    // Применяем скидку клиента
    order.totalPrice = total - (total * client->discount / 100);
    // Получаем время заказа
    GetLocalTime(&order.orderTime);
    // Обновляем общую сумму, потраченную клиентом
    client->totalSpent += order.totalPrice;
}

// Функция для добавления нового блюда в меню
void AddMenuItem(HWND hWnd)
{
    wchar_t name[50];  // Имя блюда
    wchar_t priceText[10];  // Текстовое поле для цены
    GetWindowText(hInputName, name, 50);  // Получаем текст из поля ввода имени блюда
    GetWindowText(hInputPrice, priceText, 10);  // Получаем текст из поля ввода цены

    // Проверка на пустые поля
    if (wcslen(name) == 0 || wcslen(priceText) == 0)
    {
        MessageBox(hWnd, L"Заполните все поля!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    int price = _wtoi(priceText);  // Преобразуем строку в целое число
    if (price <= 0)
    {  // Если цена не больше 0, выводим ошибку
        MessageBox(hWnd, L"Цена должна быть больше 0!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Проверка на превышение лимита элементов меню
    if (menuCount >= 10)
    {
        MessageBox(hWnd, L"Превышен лимит элементов меню!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    wcscpy_s(menu[menuCount].name, name);  // Сохраняем имя блюда
    menu[menuCount].price = price;  // Сохраняем цену блюда
    menuCount++;  // Увеличиваем количество блюд

    // Обновляем отображение меню в ListView
    ListView_DeleteAllItems(hMenuList);  // Очищаем старые элементы

    LVITEM lvItem = { };
    lvItem.mask = LVIF_TEXT;
    for (int i = 0; i < menuCount; i++)
    {
        lvItem.iItem = i;
        lvItem.pszText = menu[i].name;
        ListView_InsertItem(hMenuList, &lvItem);  // Вставляем новый элемент

        // Устанавливаем цену для блюда
        std::wstringstream ws;
        ws << menu[i].price;
        std::wstring priceStr = ws.str();
        ListView_SetItemText(hMenuList, i, 1, const_cast<LPWSTR>(priceStr.c_str()));  // Устанавливаем текст в колонку 1
    }

    // Очищаем поля ввода после добавления блюда
    SetWindowText(hInputName, L"");
    SetWindowText(hInputPrice, L"");
}

// Функция для добавления клиента из интерфейса
void AddClientFromUI(HWND hWnd)
{
    wchar_t name[50];  // Имя клиента
    wchar_t phone[20];  // Телефон клиента
    GetWindowText(hInputClientName, name, 50);  // Получаем имя клиента
    GetWindowText(hInputClientPhone, phone, 20);  // Получаем номер телефона клиента

    // Проверка на пустые поля
    if (wcslen(name) == 0 || wcslen(phone) == 0)
    {
        MessageBox(hWnd, L"Заполните все поля!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Проверка на превышение лимита клиентов
    if (clientCount >= 10)
    {
        MessageBox(hWnd, L"Превышен лимит клиентов!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    // Создаем нового клиента и добавляем его в список
    Client newClient;
    wcscpy_s(newClient.name, name);
    wcscpy_s(newClient.phone, phone);
    newClient.totalSpent = 0;
    newClient.discount = 0;
    clients[clientCount++] = newClient;

    // Обновляем список клиентов
    UpdateClientList();

    // Очищаем поля ввода после добавления клиента
    SetWindowText(hInputClientName, L"");
    SetWindowText(hInputClientPhone, L"");

    // Уведомляем об успешном добавлении клиента
    MessageBox(hWnd, L"Клиент добавлен.", L"Успех", MB_OK | MB_ICONINFORMATION);
}

// Функция для обновления списка клиентов в интерфейсе
void UpdateClientList()
{
    ListView_DeleteAllItems(hClientList);  // Очищаем старые элементы
    LVITEM lvItem = { };
    lvItem.mask = LVIF_TEXT;
    for (int i = 0; i < clientCount; i++)
    {
        lvItem.iItem = i;
        lvItem.pszText = clients[i].name;
        ListView_InsertItem(hClientList, &lvItem);  // Вставляем клиента

        // Устанавливаем телефон
        ListView_SetItemText(hClientList, i, 1, clients[i].phone);

        // Устанавливаем общую потраченную сумму
        std::wstringstream ws;
        ws << clients[i].totalSpent;
        std::wstring totalSpent = ws.str();
        ListView_SetItemText(hClientList, i, 2, const_cast<LPWSTR>(totalSpent.c_str()));  // Общая сумма
    }
}

// Функция для обновления списка заказов в интерфейсе
void UpdateOrderList()
{
    ListView_DeleteAllItems(hOrderList);  // Очищаем старые элементы
    LVITEM lvItem = { };
    lvItem.mask = LVIF_TEXT;
    for (int i = 0; i < orderCount; i++)
    {
        lvItem.iItem = i;
        lvItem.pszText = orders[i].client->name;  // Имя клиента
        ListView_InsertItem(hOrderList, &lvItem);  // Вставляем заказ

        // Устанавливаем количество блюд
        std::wstringstream ws;
        ws << orders[i].itemCount;
        std::wstring itemCount = ws.str();
        ListView_SetItemText(hOrderList, i, 1, const_cast<LPWSTR>(itemCount.c_str()));

        // Устанавливаем общую стоимость заказа
        ws.str(L"");  // Очищаем строковый поток
        ws << orders[i].totalPrice;
        std::wstring totalPrice = ws.str();
        ListView_SetItemText(hOrderList, i, 2, const_cast<LPWSTR>(totalPrice.c_str()));  // Общая стоимость
    }
}

// Основная функция, в которой будут инициализироваться окна и обработчики
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    // Ваш код инициализации окна
    MyRegisterClass(hInstance);
InitInstance(hInstance, nCmdShow);

// Вход в цикл сообщений для обработки событий
MSG msg;
while (GetMessage(&msg, NULL, 0, 0))
{
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}

return (int)msg.wParam;
}
