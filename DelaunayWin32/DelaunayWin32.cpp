// DelaunayWin32.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DelaunayWin32.h"

// Минимальное значение координаты X для исходных точек, по которым строится триангуляция.
extern double xMin;
// Максимальное значение координаты X для исходных точек, по которым строится триангуляция.
extern double xMax;
// Максимальное значение координаты Y для исходных точек, по которым строится триангуляция. 
extern double yMin;
// Максимальное значение координаты Y для исходных точек, по которым строится триангуляция.
extern double yMax;

// Представляет  ребро треугольника,  входящее
// в множество нарисованных  в окне приложения
// рёбер, составляющих триангуляционную сетку.
struct EdgeToDraw
{
	EdgeToDraw() {}

	EdgeToDraw(double X_begin, double Y_begin, double X_end, double Y_end)
	{
		Begin.X = (Gdiplus::REAL)X_begin;
		Begin.Y = (Gdiplus::REAL)Y_begin;
		End.X = (Gdiplus::REAL)X_end;
		End.Y = (Gdiplus::REAL)Y_end;
	}
	// Координаты X и Y начальной вершины ребра.
	PointF Begin;
	// Координаты X и Y конечной вершины ребра.
	PointF End;
};

#define MAX_LOADSTRING 100

#define IDM_INPUT 1       // Идентификатор кнопки выбора входного файла.
#define IDM_OUTPUT 2      // Идентификатор кнопки выбора папки для записи выходных файлов.
#define IDM_TRIANGULATE 3 // Идентификатор кнопки запуска триангуляции.
#define IDM_STOP 4        // Идентификатор кнопки останова триангуляции.
#define ID_STATUS 100     // Идентификатор панели состояния приложения.

// Глобальные переменные:
HINSTANCE hInst;                                   // Буфер для дескриптора модуля пользовательского интерфейса,
WCHAR szTitle[MAX_LOADSTRING];                     // Текст заголовка окна приложения,
WCHAR szWindowClass[MAX_LOADSTRING];               // Имя класса главного окна приложения,
HWND g_hWndToolbar;                                // Кнопочная панель управления приложением (toolbar).
HWND g_hWndStatusbar;                              // Панель состояния приложения (statusbar).
HWND g_hWndProgressbar;                            // Индикатор выполнения (progressbar).
WCHAR szInputPath[MAX_PATH];                       // Путь к входному файлу, содержащему исходное множество точек для триангуляции.              
std::wstring inFilePath;                           // Строковое представление пути к входному файлу.
WCHAR szOutputPath[MAX_PATH];                      // Путь к папке, содержащей выходные файлы триангуляции.
std::wstring outFolderPath;                        // Строковое представление пути к папке с выходными файлами триангуляции.
bool isInputClicked = false;                       // Флаг щелчка (нажатия) кнопки выбора входного файла.
bool isOutputClicked = false;                      // Флаг щелчка (нажатия) кнопки выбора выходной папки.
static 	cancellation_token_source cancelTriSource; // Источник токенов отмены для задач триангуляции.
static bool f_TriangulationCanceled = false;       // Флаг принудительной отмены триангуляции пользователем.
concurrent_vector<EdgeToDraw> redrawnEdgesBuffer;  // Буфер рёбер треугольников, из которых составляется
                                                   // рисуемая в окне приложения триангуляционная сетка.
const UINT WM_APP_DRAW_TRIMESH = WM_APP + 0;       // Сообщение для рисования триангуляционной сетки.
task<void> asyncTask;                              // Задача для асинхронного выполнения триангуляции.
const int STATUS_BAR_PARTS = 2;                    // Количество частей, на которые подразделяется панель состояния.
int range;                                         // Диапазон изменения для индикатора выполнения.
wchar_t* pInfoMessage = nullptr;                   // Текст информационного сообщения.
const wchar_t* pMessageAboutDrawing = L"Рисование триангуляционной сетки";
const wchar_t* pMessageAboutRedrawing = L"Перерисовка триангуляционной сетки";
const wchar_t* pMsgTriIsCancelled = L"Процесс триангуляции принудительно отменён пользователем.";
const wchar_t* pMsgTriIsComplete = L"Выполнена триангуляция исходного облака точек.";
const wchar_t* pMsgRedrawComplete = L"Выполнена перерисовка";
bool f_ClearClientArea = false;                    // Флаг очистки клиентской области главного окна приложения.
                                                   // По умолчанию сброшен. Устанавливается после выполнения первой
                                                   // триангуляции (с отменой/без отмены) в текущем сеансе работы
                                                   // с приложением, чтобы в последующих триангуляциях очищать
                                                   // клиентскую область от ранее нарисованных триангуляционных сеток.
bool f_TriMeshIsReadyToDisplay = false;            // Флаг готовности триангуляционной сетки к отображению на экране.
HIMAGELIST g_hImageList = NULL;                    // Список иконок для кнопок панели управления.
float nScale = 1.0;                                // Коэффициент масштабирования изображения в клиентской области окна при повороте колёсика мыши.
bool f_Zooming = false;                            // Флаг выполнения масштабирования изображения в клиентской области окна путём прокрутки колёсика мыши.
bool f_Zoomed = false;                             // Флаг завершения масштабирования изображения в клиентской области окна путём прокрутки колёсика мыши.
// Переменные, относящиеся к "перетаскиванию" изображения.
//RECT g_rcImage;                                  // Прямоугольник, ограничивающий "перетаскиваемое" изображение.
//int g_nImage;                                    // Индекс изображения.
//POINT g_ptHotSpot;                               // Местоположение "горячей точки" изображения (фактически,
                                                   // это координаты курсора мыши внутри изображения).
//BOOL g_fDragging;                                // Флаг выполнения операции "перетаскивания".
//HIMAGELIST himl;                                 // Список изображений.

double xInitialMousePosition = 0.0;                // Координата X курсора мыши в начале операции перетаскивания (в формате double).
double yInitialMousePosition = 0.0;                // Координаты Y курсора мыши в начале операции перетаскивания (в формате double).
double xValueCorrectionFactor = 0.0;               // Поправочный коэффициент для значения координаты X вершины триангуляционной сетки.
double yValueCorrectionFactor = 0.0;               // Поправочный коэффициент для значения координаты Y вершины триангуляционной сетки.
bool f_Drag = false;                               // Флаг выполнения "перетаскивания" изображения (по умолчанию - false).
double scaleFactor = 1.0;                          // Поправочный коэффициент масштабирования (используется при отрисовке изображения при его перетаскивании).


// Предварительные объявления функций, включённых в этот модуль (единицу трансляции):
ATOM                MyRegisterClass(HINSTANCE);               // Регистрирует класс главного окна приложения.
BOOL                InitInstance(HINSTANCE, int);             // Создаёт главное окно приложения.
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);      // Оконная процедура для главного окна приложения.
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);        // Выводит окно "О программе".
VOID CreateToolbar(HWND);                                     // Создаёт кнопочную панель для управления работой приложения.
VOID CreateStatusbar(HWND, int, HINSTANCE, int);              // Создаёт панель состояния приложения.
VOID CreateProgressbar();                                     // Создаёт индикатор выполнения.
void DrawTriMesh(BitmapPtr, Mesh*, HWND, cancellation_token); // Рисует триангуляционную сетку в указанном объекте Bitmap.
void RedrawTriMesh(BitmapPtr, HWND);                          // Перерисовывает триангуляционную сетку при изменении размера окна.
VOID OnPaint(HWND, HDC);                                      // Обрабатывает сообщение WM_PAINT.
// Функции, относящиеся к "перетаскиванию" изображения:
//BEGIN DEBUG:
//BOOL StartDragging(HWND, POINT, HIMAGELIST);                  // Инициирует процесс "перетаскивания".
//BOOL MoveTheImage(POINT);                                     // "Перетаскивает" изображение в указанную точку.
//BOOL StopDragging(HWND, HIMAGELIST, POINT);                   // Завершает операцию "перетаскивания" и рисует изображение в его окончательном месте.
//END DEBUG.
// Определяет ширину и высоту растрового изображения и высоту кнопочной панели управления. 
void SetRequiredDimensions(BitmapPtr, UINT&, UINT&, LONG&);
// Вычисляет и возвращает значение коэффициента масштабирования.
double SetScale(Graphics*, const UINT, UINT, LONG);


// Точка входа в приложение.
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	// Неиспользуемые параметры:
	// - Дескриптор уже запущенной копии приложения - hPrevInstance не используется в Win32
	//   и всегда равен NULL, т.к. в Win32 считается, что текущая копия приложения - hInstance
	//   всегда запускается первой вследствии того, что для каждого приложения выделяется своё
	//   адресное пространство.
	UNREFERENCED_PARAMETER(hPrevInstance);
	// - Командная строка - lpCmdLine не используется в приложении, т.к. оно является оконным.
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Инициализировать GDI+.
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);


	// Инициализировать глобальные строки:
	// - заголовок главного окна,
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	// - название приложения.
	LoadStringW(hInstance, IDC_DELAUNAYWIN32, szWindowClass, MAX_LOADSTRING);
	// Вызвать функцию регистрации главного окна приложения.
	MyRegisterClass(hInstance);

	// Выполнить попытку создания и вывода на экран главного окна приложения:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	// Загрузить таблицу акселераторов ("горячих" клавиш).
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DELAUNAYWIN32));

	// Буфер сообщений.
	MSG msg;

	// Главный цикл обработки сообщений:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// Деинициализировать GDI+.
	GdiplusShutdown(gdiplusToken);

	// По окончанию работы, вернуть значение, передаваемое сообщению WM_QUIT.
	return (int)msg.wParam;
}

// Регистрирует класс главного окна приложения.
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DELAUNAYWIN32));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DELAUNAYWIN32);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

// Сохраняет дескриптор экземпляра модуля пользовательского интерфейса
// в глобальной переменной, а также, создаёт и показывает главное окно
// приложения.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	// Сохранить дескриптор модуля пользовательского интерфейса.
	hInst = hInstance;

	// Создать главное окно приложения.
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	// Визуализировать главное окно приложения.
	ShowWindow(hWnd, nCmdShow);
	// Послать оконной процедуре сообщение WM_PAINT, 
	// если требуется перерисовать содержимое окна.
	UpdateWindow(hWnd);

	return TRUE;
}


// Создаёт панель управления.
VOID CreateToolbar(HWND hWndParent)
{
	// Объявить и инициализировать константы, используемые в функции:
	// - идентификатор списка картинок для кнопок,
	const int ImageListID = 0;
	// - количество кнопок.
	const int numButtons = 4;
	// Размер, в пикселях, каждой картинки для кнопки
	const int bitmapSize = 32;

	const DWORD buttonStyles = BTNS_AUTOSIZE;

	// Создать панель управления приложением, с выводом подсказки для каждой кнопки.
	/*HWND hWndToolbar*/g_hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
		WS_CHILD | TBSTYLE_WRAPABLE | TBSTYLE_TOOLTIPS | TBSTYLE_LIST, 0, 0, 0, 0,
		hWndParent, NULL, hInst, NULL);

	if (/*hWndToolbar*/g_hWndToolbar == NULL)
		return /*NULL*/;

	// Создать список картинок для кнопок.
	g_hImageList = ImageList_Create(bitmapSize, bitmapSize,
		ILC_COLOR16 | ILC_MASK, // Обеспечить прозрачный фон.
		numButtons, 0);

	ImageList_AddIcon(g_hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_SELECT_INPUT_DATA)));
	ImageList_AddIcon(g_hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_SELECT_OUTPUT_FOLDER)));
	ImageList_AddIcon(g_hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_START_TRIANGULATION)));
	ImageList_AddIcon(g_hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_STOP_TRIANGULATION)));

	// Установить список картинок для кнопок.
	SendMessage(/*hWndToolbar*/g_hWndToolbar, TB_SETIMAGELIST,
		(WPARAM)ImageListID,
		(LPARAM)g_hImageList);

	// Инициализировать информацию о кнопках.
	TBBUTTON tbButtons[numButtons] =
	{
		{ MAKELONG(0,  ImageListID), IDM_INPUT,  TBSTATE_ENABLED, buttonStyles, {0}, 0, 0 },
		{ MAKELONG(1, ImageListID), IDM_OUTPUT, TBSTATE_ENABLED, buttonStyles, {0}, 0, 0},
		{ MAKELONG(2, ImageListID), IDM_TRIANGULATE, TBSTATE_ENABLED, buttonStyles, {0}, 0, 0},
		{ MAKELONG(3, ImageListID), IDM_STOP, 0, buttonStyles, {0}, 0, 0}
	};

	// Добавить кнопки.
	SendMessage(/*hWndToolbar*/g_hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	SendMessage(/*hWndToolbar*/g_hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)&tbButtons);

	// Изменить размер панели инструментов, а затем показать её.
	SendMessage(/*hWndToolbar*/g_hWndToolbar, TB_AUTOSIZE, 0, 0);
	ShowWindow(/*hWndToolbar*/g_hWndToolbar, TRUE);

	//return hWndToolbar;
}

// Description: 
//   Создаёт панель состояния приложения и подразделяет её на указанное количесто частей.
// Parameters:
//   hwndParent - родительское окно для панели состояния (в нашем случае - главное окно приложения).
//   idStatus - идентификатор дочернего окна, представляющего панель состояния.
//   hinst - дескриптор экземпляра приложения.
//   cParts - количество частей, на которые подразделяется панель состояния.
//
VOID CreateStatusbar(HWND hwndParent, int idStatus, HINSTANCE hinst, int cParts)
{
	// Клиентская область родительского окна (главного окна приложения).
	RECT rcClient;
	// Дескриптор на блок памяти, выделяемой под хранение массива координат правого края
	// каждой из частей, составляющих панель состояния.
	HLOCAL hloc;
	// Массив для хранения координат правого края каждой из частей,
	// составляющих панель состояния.
	PINT paParts;
	// Порядковый индекс и ширина каждой из частей составляющих панель состояния.
	int i, nWidth;

	// Гарантировать загрузку библиотеки DLL устройств управления общего назначения.
	InitCommonControls();

	// Создать панель состояния.
	g_hWndStatusbar = CreateWindowEx(
		0,                     // без использования расширенных стилей
		STATUSCLASSNAME,       // имя класса панели состояния
		(PCTSTR)NULL,          // при своём первом создании не содержит текста
		SBARS_SIZEGRIP |       // может изменять свой размер
		WS_CHILD | WS_VISIBLE, // создаётся как видимое дочернее окно
		0, 0, 0, 0,            // игнорирует размер и местоположение
		hwndParent,            // дескриптор родительского окна
		(HMENU)idStatus,       // идентификатор панели состояния, как дочернего окна
		hinst,                 // дескриптор экземпляра приложения
		NULL);                 // нет данных создания окна

	// Установить высоту панели состояния приложения.
	SendMessage(g_hWndStatusbar, SB_SETMINHEIGHT, 22, 0);

	// Получить координаты клиентской области родительского окна.
	GetClientRect(hwndParent, &rcClient);

	// Выделить память под массив для хранения координат правого края
	// каждой из частей, на которые подразделяется панель состояния.
	hloc = LocalAlloc(LHND, sizeof(int) * cParts);
	paParts = (PINT)LocalLock(hloc);

	// Рассчитать  координату правого края для каждой из частей,
	// на которые подразделяется панель состояния, и скопировать
	// полученные координаты в массив.
	nWidth = rcClient.right / cParts;
	int rightEdge = nWidth;
	for (i = 0; i < cParts; i++) {
		paParts[i] = rightEdge;
		rightEdge += nWidth;
	}

	// Указать панели состояния создать части, на которые будет подразделяться её окно.
	SendMessage(g_hWndStatusbar, SB_SETPARTS, (WPARAM)cParts, (LPARAM)
		paParts);

	// Освободить память, выделенную под массив и вернуть дескриптор панели состояния.
	LocalUnlock(hloc);
	LocalFree(hloc);
}

// Создаёт индикатор выполнения (progressbar).
VOID CreateProgressbar()
{
	RECT r1;        // Прямоугольная область родительского окна (панели состояния).
	int cyVScroll;  // Высота полосы прокрутки окна (нужна для задания высоты
	                // индикатора выполнения).

	// Гарантировать, что библиотека DLL для создания элементов управления загружена.
	InitCommonControls();

	SendMessage(g_hWndStatusbar, SB_GETRECT, 1, (LPARAM)&r1);

	cyVScroll = GetSystemMetrics(SM_CYVSCROLL);

	// Создать индикатор процесса выполнения (progressbar) в правой половине
	// панели состояния приложения.
	g_hWndProgressbar = CreateWindowEx(
		0,                                  // без использования дополнительных стилей окна
		PROGRESS_CLASS,                     // создаётся индикатор процесса (progresbar) 
		(LPTSTR)NULL,                       // без текстовой информации в окне
		WS_CHILD | WS_VISIBLE | PBS_SMOOTH, // использовать следующие основные стили:
											// - является дочерним окном
											// - является видимым сразу после своего создания
											// - заполнение индикатора будет показываться сплошной полосой
		r1.left + 3,
		r1.top + 3,
		r1.right - r1.left, cyVScroll,
		g_hWndStatusbar,                         // указать дескриптор родительского окна (панели состояния приложения)
		(HMENU)0,                           // меню у окна индикатора выполнения отсутствует 
		hInst,                              // дескриптор экземпляра модуля, ассоциированного с окном
		NULL                                // дополнительных данных не требуется
	);
}

// Рисует триангуляционную сетку в указанном объекте
// растрового изображения (Bitmap).
void DrawTriMesh(BitmapPtr pBitmap, Mesh* m, HWND hWnd, cancellation_token token)
{
	// Проверить, была ли отмена задачи.
	if (token.is_canceled())
	{
		concurrency::cancel_current_task();//  ::cancel_current_task();
	}
	else
	{
		// Если не создан объект растрового изображения,
		// то не выполнять рисования.
		if (pBitmap == nullptr)
			return;

		// Буферы для ширины и высоты растрового изображения, в котором будет рисоваться триангуляционная сетка.
		UINT bmWidth, bmHeight = 0;
		// Буфер для высоты кнопочной панели управления.
		LONG toolbarHeight = 0;

		// Определить размеры растрового изображения, в котором будет выполняться рисование
		// и высоту кнопочной панели управления.
		SetRequiredDimensions(pBitmap, bmWidth, bmHeight, toolbarHeight);
		// Создать объект, поддерживающий возможности рисования в растровом изображении.
		Graphics* bmGraphics = Graphics::FromImage(&(*pBitmap));
		// Получить коэффициент масштабирования.
		double scale = SetScale(bmGraphics, bmWidth, bmHeight, toolbarHeight);
		// Установить, в клиентской области окна, ориентацию оси Y снизу вверх
		bmGraphics->ScaleTransform(1, -1);
		// Выполнить масштабирование.
		bmGraphics->ScaleTransform((Gdiplus::REAL)scale, (Gdiplus::REAL)scale);

		// Сохранить значение масштабного коэффициента для использования при "перетаскивании" изображения.
		scaleFactor = scale;

		// Установить характеристики пера, которым будет выполняться рисование.
		Gdiplus::REAL penWidth = 1 / (Gdiplus::REAL)scale;
		Pen pen(Color(255, 0, 0, 255), penWidth);
		bmGraphics->SetSmoothingMode(SmoothingModeAntiAlias);

		// Взять количество рёбер.
		long edges = m->Edges;
		// Запустить бход по блокам.
		TraversalInit(&m->Triangles);
		// Определить переменную цикла, проходящего по треугольникам.
		OTriangle triangleloop;
		// Текуще-взятый треугольник, рёбра которого будут рисоваться.
		triangleloop.Tri = TriangleTraverse(&m->Triangles);
		// Примыкающий (смежный) треугольник на текуще-взятом ребре текуще-взятого треугольника.
		OTriangle trisym;
		// Начальная вершина (точка) ребра.
		Vertex beginOfEdge;
		// Конечная вершина (точка) ребра.
		Vertex endOfEdge;

		PostMessage(g_hWndStatusbar, SB_SETTEXTW, 0, (LPARAM)pMessageAboutDrawing);
		// Передать количество входных вершин в главное окно для установки индикатора выполнения в исходное состояние.
		const int RANGE = 100;
		int vertices = m->InVertices;
		int messageCounter = 1;
		if (m->InVertices > RANGE)
		{
			messageCounter = m->InVertices / RANGE;
			vertices /= messageCounter;
		}
		PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)vertices);
		int count = 0;
		int i = 0;

		// Чтобы  перебрать  множество ребер, нужно  перебрать  все треугольники  и посмотреть
		// на три ребра каждого треугольника. Если нет другого треугольника, смежного с ребром,
		// то оперировать ребром. Если есть другой смежный треугольник, то  оперировать ребром,
		// только если текущий треугольник имеет меньший указатель, чем его сосед.Таким образом,
		// каждое ребро рассматривается только один раз.
		while (triangleloop.Tri != nullptr)
		{
			// Проверить, была ли отмена задачи.
			if (token.is_canceled())
			{
				concurrency::cancel_current_task();//cancel_current_task();
			}
			else
			{
				for (triangleloop.Orient = 0; triangleloop.Orient < 3; triangleloop.Orient++)
				{
					FindAbutingTriOnSameEdge(triangleloop, trisym);
					if ((triangleloop.Tri < trisym.Tri) || (trisym.Tri == m->DummyTri))
					{
						// Получить точку начала ребра.
						beginOfEdge = GetOrignVertex(triangleloop);
						// Получить её координаты.
						double* pEdgeBounding = ((double*)beginOfEdge);
						double X_begin = *pEdgeBounding;
						pEdgeBounding = pEdgeBounding + 1;
						double Y_begin = *pEdgeBounding;
						// Получить точку конца ребра.
						endOfEdge = GetDestinationVertex(triangleloop);
						// Получить её координаты.
						pEdgeBounding = ((double*)endOfEdge);
						double X_end = *pEdgeBounding;
						pEdgeBounding = pEdgeBounding + 1;
						double Y_end = *pEdgeBounding;
						// Нарисовать очередное ребро триангуляционной сетки.
						bmGraphics->DrawLine(&pen, (REAL)X_begin, (REAL)Y_begin, (REAL)X_end, (REAL)Y_end);
						// Сохранить это ребро в буфере для перерисовки всвязи с изменением размера окна.
						redrawnEdgesBuffer.push_back(EdgeToDraw(X_begin, Y_begin, X_end, Y_end));
					}
				}
				// Взять следующий треугольник.
				triangleloop.Tri = TriangleTraverse(&m->Triangles);

				// Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
				if (i % messageCounter == 0 && count < vertices)
				{
					PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
					count++;
				}
				i++;
			}
		}
		// Установить флаг готовности триангуляционной сетки к отображению на экране.
		f_TriMeshIsReadyToDisplay = true;
		// Добавить растровое изображение триангуляционной сетки в очередь изображений.
		send(m_TriMeshImages, pBitmap);

		// Отправить сообщение в поток UI.
		//PostMessage(hWnd, WM_PAINT, 0, 0);
		// Инициировать перерисовку клиентской области.
		//InvalidateRect(hWnd, NULL, TRUE);
	}
}

// Перерисовывает триангуляционную сетку при изменении размера окна приложения
// и при перетаскивании её изображения.
void RedrawTriMesh(BitmapPtr pBitmap, HWND hWnd)
{
	// Проверить, была ли отменена работа группы задач перерисовки триангуляционной сетки.
	if (redrawingTasks.is_canceling())
		return;

	// Если не создан объект растрового изображения, то не выполнять рисования.
	if (pBitmap == nullptr)
		return;

	// Вывести, в панель состояния, сообщение о перерисовке  триангуляционной сетки.
	PostMessage(g_hWndStatusbar, SB_SETTEXTW, 0, (LPARAM)pMessageAboutRedrawing);
	// Установить индикатор выполнения в исходное состояние.
	const int RANGE = 100;
	int vertices = redrawnEdgesBuffer.size();
	int messageCounter = 1;
	if (redrawnEdgesBuffer.size() > RANGE)
	{
		messageCounter = redrawnEdgesBuffer.size() / RANGE;
		vertices /= messageCounter;
	}
	PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)vertices);
	int count = 0;
	int inx = 0;

	// Буферы для ширины и высоты растрового изображения, в котором будет рисоваться триангуляционная сетка.
	UINT bmWidth, bmHeight = 0;
	// Буфер для высоты кнопочной панели управления.
	LONG toolbarHeight = 0;

	// Определить размеры растрового изображения и высоту кнопочной панели управления.
	SetRequiredDimensions(pBitmap, bmWidth, bmHeight, toolbarHeight);
	// Создать объект, поддерживающий возможности рисования в растровом изображении.
	Graphics* bmGraphics = Graphics::FromImage(&(*pBitmap));
	// Вычислить коэффициент масштабирования.
	double scale = SetScale(bmGraphics, bmWidth, bmHeight, toolbarHeight);
	// Установить, в клиентской области окна, ориентацию оси Y снизу вверх
	bmGraphics->ScaleTransform(1, -1);
	// Выполнить масштабирование.
	bmGraphics->ScaleTransform((Gdiplus::REAL)scale, (Gdiplus::REAL)scale);
	// Установить характеристики пера, которым будет выполняться рисование.
	Gdiplus::REAL penWidth = 1 / ((Gdiplus::REAL)scale * nScale);
	Pen pen(Color(255, 0, 0, 255), penWidth);
	bmGraphics->SetSmoothingMode(SmoothingModeAntiAlias);
	// Перерисовать триангуляционную сетку.
	//parallel_for_each(begin(redrawnEdgesBuffer), end(redrawnEdgesBuffer), [/*&*/bmGraphics, &pen](EdgeToDraw redrawnEdge)
		//{
			//bmGraphics->DrawLine(&pen, redrawnEdge.Begin, redrawnEdge.End);
		//});
	// Объявить коэффициент частоты проверки состояния токена отмены.
	// Значение этого коэффициента будет зависить от количества рёбер
	// в триангуляционной сетке.
	int cancellationFrequencyChecingFactor = 1;
	if (redrawnEdgesBuffer.size() > 50 && redrawnEdgesBuffer.size() <= 500)
		cancellationFrequencyChecingFactor = 10;
	else if (redrawnEdgesBuffer.size() > 500 && redrawnEdgesBuffer.size() <= 1000)
		cancellationFrequencyChecingFactor = 50;
	else if (redrawnEdgesBuffer.size() > 1000)
		cancellationFrequencyChecingFactor = 100;
	// Перерисовать триангуляционную сетку, используя уже имеющиеся рёбра.
	for (auto iter = begin(redrawnEdgesBuffer); iter != end(redrawnEdgesBuffer); iter++)
	{
		if ((inx % cancellationFrequencyChecingFactor) == 0)
		{
			if (redrawingTasks.is_canceling())
				return;
		}
		if (f_Drag)
		{
			(*iter).Begin.X += (float)xValueCorrectionFactor;
			(*iter).Begin.Y += (float)yValueCorrectionFactor;
			(*iter).End.X += (float)xValueCorrectionFactor;
			(*iter).End.Y += (float)yValueCorrectionFactor;
		}
		bmGraphics->DrawLine(&pen, (*iter).Begin, (*iter).End);
		// Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
		if (inx % messageCounter == 0 && count < vertices)
		{
			PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
			count++;
		}
		inx++;
	}
	if (f_Drag)
		f_Drag = false;
	// Добавить растровое изображение триангуляционной сетки в очередь изображений.
	send(m_TriMeshImages, pBitmap);
	// Инициировать перерисовку содержимого клиентской области окна приложения.
	InvalidateRect(hWnd, NULL, TRUE);
}

// Определяет ширину и высоту растрового изображения и высоту кнопочной панели управления. 
void SetRequiredDimensions(BitmapPtr pBitmap, UINT& bmWidth, UINT& bmHeight, LONG& toolbarHeight)
{
	// Получить размер объекта растрового изображения,
    // в котором будет выполняться рисование.
	bmWidth = pBitmap->GetWidth();
	bmHeight = pBitmap->GetHeight();
	// Если высота или ширина растрового изображения
	// равна нулю, то не выполнять рисования.
	if (bmWidth == 0 || bmHeight == 0)
		return;
	// Получить высоту toolbar'а.
	RECT toolbarRect;
	GetWindowRect(g_hWndToolbar, &toolbarRect);
	toolbarHeight = toolbarRect.bottom - toolbarRect.top;
	// Получить высоту statusbar'а
	RECT statusbarRect;
	GetWindowRect(g_hWndStatusbar, &statusbarRect);
	LONG statusbarHeight = statusbarRect.bottom - statusbarRect.top;
	// Скорректировать высоту растрового изображения.
	bmHeight = bmHeight - toolbarHeight - statusbarHeight;
}

// Вычисляет и возвращает значение коэффициента масштабирования.
double SetScale(Graphics* bmGraphics, /*const*/ UINT bmWidth, UINT bmHeight, LONG toolbarHeight)
{
	// Коэффициент масштабирования.
	double scale = 0;
	// Поместить начало координат:
	if (xMin < 0 && xMax > 0 && yMin < 0 && yMax > 0)
	{
		// в центр окна, если используются все квадранты координатой плоскости,
		bmGraphics->TranslateTransform((Gdiplus::REAL)(bmWidth / 2), (Gdiplus::REAL)(bmHeight / 2 + toolbarHeight));
		scale = min(bmWidth / 2, bmHeight / 2) / max(xMax, yMax);
	}
	else if (xMin < 0 && xMax > 0 && yMin >= 0 && yMax > 0)
	{
		// в середину нижней границы окна, если используются 1-й и 2-й квадранты координатной плоскости,
		bmGraphics->TranslateTransform((Gdiplus::REAL)(bmWidth / 2), (Gdiplus::REAL)(bmHeight + toolbarHeight));
		scale = min(bmWidth / 2, bmHeight) / max(xMax, yMax);
	}
	else if (xMin < 0 && xMax <= 0 && yMin < 0 && yMax > 0)
	{
		// в середину правой границы окна, если используются 2-й и 3-й квадранты координатной плоскости,
		bmGraphics->TranslateTransform((Gdiplus::REAL)bmWidth, (Gdiplus::REAL)(bmHeight / 2 + toolbarHeight));
		scale = min(bmWidth, bmHeight / 2) / max(xMax, yMax);
	}
	else if (xMin < 0 && xMax > 0 && yMin < 0 && yMax <= 0)
	{
		// в середину верхней границы окна, если используются 3-й и 4-й квадранты координатной плоскости,
		bmGraphics->TranslateTransform((Gdiplus::REAL)(bmWidth / 2), (Gdiplus::REAL)(0.0 + toolbarHeight));
		scale = min(bmWidth / 2, bmHeight) / max(xMax, yMax);
	}
	else if (xMin >= 0 && xMax > 0 && yMin < 0 && yMax > 0)
	{
		// в середину левой границы окна, если используются 4-й и 1-й квадранты координатной плоскости,
		bmGraphics->TranslateTransform(0.0, (Gdiplus::REAL)(bmHeight / 2 + toolbarHeight));
		scale = min(bmWidth, bmHeight / 2) / max(xMax, yMax);
	}
	else if (xMin >= 0 && xMax > 0 && yMin >= 0 && yMax > 0)
	{
		// в нижний левый угол окна, если используется 1-й квадрант координатной плоскости,
		bmGraphics->TranslateTransform(0.0, (Gdiplus::REAL)(bmHeight + toolbarHeight));
		return min(bmWidth, bmHeight) / max(xMax, yMax);
	}
	else if (xMin < 0 && xMax <= 0 && yMin >= 0 && yMax > 0)
	{
		// в нижний правый угол окна, если используется 2-й квадрант координатной плоскости,
		bmGraphics->TranslateTransform((Gdiplus::REAL)bmWidth, (Gdiplus::REAL)(bmHeight + toolbarHeight));
		scale = min(bmWidth, bmHeight) / max(xMax, yMax);
	}
	else if (xMin < 0 && xMax <= 0 && yMin < 0 && yMax <= 0)
	{
		// в верхний правый угол окна, если используется 3-й квадрант координатной плоскости,
		bmGraphics->TranslateTransform((Gdiplus::REAL)bmWidth, (Gdiplus::REAL)(0.0 + toolbarHeight));
		scale = min(bmWidth, bmHeight) / max(xMax, yMax);
	}
	else if (xMin >= 0 && xMax > 0 && yMin < 0 && yMax <= 0)
	{
		// в верхний левый угол окна, если используется 4-й квадрант координатной плоскости.
		bmGraphics->TranslateTransform(0.0, (Gdiplus::REAL)(0.0 + toolbarHeight));
		scale = min(bmWidth, bmHeight) / max(xMax, yMax);
	}

	return scale;
}

// Обрабатывает сообщение WM_PAIN.
VOID OnPaint(HWND hWnd, HDC hDc)
{
	if (!isnan(xMin) && !isnan(xMax) && !isnan(yMin) && !isnan(yMax))
	{
		// Если буфер обмена (unbounded_buffer) содержит объект Bitmap,
		// содержащий нарисованную триангуляционную сетку, то показать
		// её на экране.
		// Иначе, создать объект Bitmap и инициировать в нём рисование
		// триангуляционной сетки по результатам триангуляции.
		BitmapPtr pBitmap;
		if (try_receive(m_TriMeshImages, pBitmap))
		{
			if (pBitmap != NULL)
			{
				// Если выполняется масштабирование.
				if (f_Zoomed)
				{
					// Показать изображение триангуляционной сетки в изменённом масштабе.
					RECT clientRect;
					GetClientRect(hWnd, &clientRect);
					HDC targetDC = hDc;
					HDC bufferedDC = NULL;
					HRESULT hr = S_OK;
					HPAINTBUFFER pb = BeginBufferedPaint(targetDC, &clientRect, BPBF_TOPDOWNDIB, 0, &bufferedDC);
					Graphics* graphics = new Graphics(bufferedDC);
					Bitmap *bm = pBitmap.get();
					UINT bmWidth = bm->GetWidth();
					UINT bmHeight = bm->GetHeight();
					if (graphics && graphics->GetLastStatus() == Ok)
					{
						graphics->Clear(Color(255, 255, 255, 255));
						REAL nX = (clientRect.right - clientRect.left - bmWidth * nScale) / 2;
						REAL nY = (clientRect.bottom - clientRect.top - bmHeight * nScale) / 2;
						graphics->DrawImage(pBitmap.get(), nX, nY, (REAL)bmWidth * nScale, (REAL)bmHeight * nScale);
						delete graphics;
					}
					hr = EndBufferedPaint(pb, TRUE);
					// Сбросить флаг выполнения масштабирования.
					f_Zooming = false;
				}
				else
				{
					Graphics* graphics = Graphics::FromHDC(hDc);
					graphics->DrawImage(pBitmap.get(), 0, 0);
				}
			}
		}
		else
		{
			if (f_TriMeshIsReadyToDisplay)
			{
				RECT clientRect;
				GetClientRect(hWnd, &clientRect);
				if (clientRect.right != 0 && clientRect.bottom != 0)
				{
					redrawingTasks.run([clientRect, hWnd]()
						{
							RedrawTriMesh(BitmapPtr(new Bitmap(clientRect.right, clientRect.bottom)), hWnd);
							// Сбросить индикатор выполнения в исходное состояние.
							PostMessage(g_hWndProgressbar, PBM_SETPOS, 0, 0);
							// Вывести, в панель состояния, сообщение о выполнении перерисовки.
							PostMessage(g_hWndStatusbar, SB_SETTEXTW, 0, (LPARAM)pMsgRedrawComplete);
						});
				}
			}
		}
	}
}

/*
// Инициирует процесс "перетаскивания". Если щелчок мыши выполнился
// внутри прямоугольника, ограничивающего изображение, то захватывает
// ввод мыши, стирает изображение из клиентской области и вычисляет
// положение горячей точки на изображении.
BOOL StartDragging(HWND hWnd, POINT ptCur, HIMAGELIST himl)
{
	// Если щелчок мыши выполнен не внутри прямоугольника,
	// ограничивающего изображение, то он не будет обработан. 
	if (!PtInRect(&g_rcImage, ptCur))
		return FALSE;

	// Захватить ввод мыши. 
	SetCapture(hWnd);

	// Стереть изображение в клиентской области.
	InvalidateRect(hWnd, &g_rcImage, TRUE);
	UpdateWindow(hWnd);

	// Вычислить и сохранить местоположение горячей точки. 
	g_ptHotSpot.x = ptCur.x - g_rcImage.left;
	g_ptHotSpot.y = ptCur.y - g_rcImage.top;

	// Начать операцию перетаскивания. 
	if (!ImageList_BeginDrag(himl, g_nImage,
		g_ptHotSpot.x, g_ptHotSpot.y))
		return FALSE;

	// Получить прямоугольник гланого окна приложения.
	RECT clientRect, windowRect;
	GetClientRect(hWnd, &clientRect);
	GetWindowRect(hWnd, &windowRect);
	// Получить ширину рамки окна приложения.
	//long g_cxBorder = (windowRect.right - windowRect.left) - clientRect.right;
	int g_cxBorder = GetSystemMetrics(SM_CXBORDER);
	// Получить высоту рамки окна приложения
	//long g_cyBorder = (windowRect.bottom - windowRect.top) - clientRect.bottom;
	int g_cyBorder = GetSystemMetrics(SM_CYBORDER);
	// Получить высоту панели заголовка окна приложения.
	//const UINT dpi = GetDpiForWindow(hWnd);
	//clientRect = { 0 };
	//AdjustWindowRectExForDpi(&clientRect, WS_OVERLAPPEDWINDOW, FALSE, 0, dpi);
	//long g_cyCaption = abs(clientRect.top);
	int g_cyCaption = GetSystemMetrics(SM_CYCAPTION);
	// Получить высоту панели меню в окне приложения.
	int g_cyMenu = GetSystemMetrics(SM_CYMENU);
	// Установить начальное местоположение изображения и сделать его видимым.
	// Поскольку функция ImageList_DragEnter ожидает, что координаты будут
	// относительно верхнего левого угла данного окна, необходимо учитывать
	// ширину границы, строку заголовка и строку меню.
	ImageList_DragEnter(hWnd, ptCur.x + g_cxBorder,
		ptCur.y + g_cyBorder + g_cyCaption + g_cyMenu);

	g_fDragging = TRUE;

	return TRUE;
}

// Перетаскивает изображение в указанную точку. 
BOOL MoveTheImage(POINT ptCur)
{
	// Если "перетаскивание" не выполнилось, то вернуть FALSE.
	if (!ImageList_DragMove(ptCur.x, ptCur.y))
		return FALSE;

	// Иначе, в случае успешного выполнения перетаскивания, вернуть TRUE. 
	return TRUE;
}

// Завершает операцию "перетаскивания" и рисует изображение в его окончательном месте.
BOOL StopDragging(HWND hwnd, HIMAGELIST himl, POINT ptCur)
{
	ImageList_EndDrag();
	ImageList_DragLeave(hwnd);

	g_fDragging = FALSE;

	//DrawTheImage(hwnd, himl, ptCur.x - g_ptHotSpot.x,
		//ptCur.y - g_ptHotSpot.y);
	// "Отпустить" мышь.
	ReleaseCapture();
	return TRUE;
}
//*/

/*
// Возвращает поправку на разрешение экрана.
double GetScaleFactor(HWND hWnd)
{
	double screenScale = 0;
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL);
	if (hMonitor != NULL)
	{
		DEVICE_SCALE_FACTOR nScaleFactor;
		if (GetScaleFactorForMonitor(hMonitor, &nScaleFactor) == S_OK)
		{
			if (nScaleFactor != DEVICE_SCALE_FACTOR_INVALID)
				screenScale = static_cast<double>(static_cast<int>(nScaleFactor)) / 100.0;
		}
	}
	return screenScale
}
//*/

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Выполнить разбор пунктов меню:
		switch (wmId)
		{
		case IDM_ABOUT:       // Вывести на экран окно с информацией о приложении.
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:        // Выполнить выход из приложения.
			if (MessageBox(hWnd, L"Вы действительно хотите выйти из приложения?", L"Триангулятор",
				MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
			{
				DestroyWindow(hWnd);
			}
			break;
		case IDM_INPUT:       // Выбрать файл с входными данными.
		{
			if (!isInputClicked)
			{
				isInputClicked = true;
				DialogService* ptrDialogService = new DialogService();
				ptrDialogService->CreateDialogToSelectFile(hWnd);
				inFilePath = ptrDialogService->GetInputDataFilePath();
				delete ptrDialogService;
				// Если файл не выбран, то запретить кнопку запуска триангуляции.
				if (inFilePath.empty())
					SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_TRIANGULATE, MAKELONG(0, 0));
				else
				{
					// Иначе, если файл выбран, но не выбрана выходная папка, то запретить кнопку запуска триангуляции.
					// Иначе, если выбраны и файл и выходная папка, то разрешить кнопку запуска триангуляции.
					if (outFolderPath.empty())
						SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_TRIANGULATE, MAKELONG(0, 0));
					else
						SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_TRIANGULATE, MAKELONG(1, 0));
				}
				isInputClicked = false;
			}
		}
		break;
		case IDM_OUTPUT:      // Выбрать папку для записи файлов с выходными данными.
		{
			if (!isOutputClicked)
			{
				isOutputClicked = true;
				DialogService* ptrDialogService = new DialogService();
				ptrDialogService->CreateDialogToSelectFolder(hWnd);
				outFolderPath = ptrDialogService->GetOutputFilesFolderPath();
				delete ptrDialogService;
				// Если не выбрана выходная папка, то запретить кнопку запуска триангуляции.
				if (outFolderPath.empty())
					SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_TRIANGULATE, MAKELONG(0, 0));
				else
				{
					// Иначе, если папка выбрана, но не выбран входной файл, то запретить кнопку запуска триангуляции.
					// Иначе, если выбраны и папка и входной файл, то разрешить кнопку запуска триангуляции.
					if (inFilePath.empty())
						SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_TRIANGULATE, MAKELONG(0, 0));
					else
						SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_TRIANGULATE, MAKELONG(1, 0));
				}
				isOutputClicked = false;
			}
		}
		break;
		case IDM_TRIANGULATE: // Запустить триангуляцию:
		{
			// Очистить клиентскую область окна.
			if (f_ClearClientArea)
				InvalidateRect(hWnd, NULL, TRUE);
			// Очистить буфер рисуемых рёбер триангуляционной сетки.
			if (redrawnEdgesBuffer.size() > 0)
				redrawnEdgesBuffer.clear();
			// Сбросить флаг готовности триангуляционной сетки к отображению на экране.
			if (f_TriMeshIsReadyToDisplay)
				f_TriMeshIsReadyToDisplay = false;
			// Сбросить флаг завершения масштабирования изображения.
			if (f_Zoomed)
				f_Zoomed = false;
			// Обнулить поправочный коэффициент для значения координаты X вершины триангуляционной сетки.
			xValueCorrectionFactor = 0;
			// Обнулить поправочный коэффициент для значения координаты Y вершины триангуляционной сетки.
			yValueCorrectionFactor = 0;
			// Обнулить поправочный коэффициент масштабирования,
			// используемый при "перетаскивании" изображения
			scaleFactor = 1.0;
			nScale = 1.0;

			asyncTask = create_task([hWnd]()
				{
					// Получить токен отмены для задачи triangulationTask.
					cancelTriSource = cancellation_token_source();
					cancellation_token token = cancelTriSource.get_token();
					// Буфер для данных триангуляционной сетки, которые будут получены в процессе триангуляции.
					Mesh triMesh;
					// Настроечные параметры для приложения
					Configuration b;
					// Определить задачу для триангуляции.
					auto triangulationTask = create_task([&triMesh, &b, &token, hWnd]()
						{
							// Проверить, была ли отмена задачи.
							if (token.is_canceled())
							{
								concurrency::cancel_current_task();//  cancel_current_task();
							}
							else
							{
								DelaunayInitialization delaunayInit;
								// Выполнить инициализацию используемых при триангуляции переменных.
								delaunayInit.TriInit(inFilePath, outFolderPath, &triMesh, &b);
								// Прочитать исходые данные для триангуляции.
								delaunayInit.ReadNodes(hWnd, &triMesh, &b, b.InNodeFileName, token);
								// Выделить память для треугольников и подсегментов.
								triMesh.AttributesPerTriangle = 0;
								delaunayInit.InitializeTriSubpools(&triMesh, &b);
								// Запустить триангуляцию. По её окончанию, получаем, в частности, количество рёбер,
								// ограничивающих триангуляционную сетку по её краям.
								DelaunayTriangulation delaunayTri;
								triMesh.HullSize = delaunayTri.StartTriangulation(hWnd, &triMesh, &b, token);

								// Подсчитать количество всех рёбер на выпуклой поверхности.
								triMesh.Edges = (3l * triMesh.Triangles.CurrentlyAllocatedItems + triMesh.HullSize) / 2l;

								// Получить данные для рисования триангуляционной сетки и выполнить её скрытое рисование.
								RECT clientRect;
								GetClientRect(hWnd, &clientRect);
								DrawTriMesh(BitmapPtr(new Bitmap(clientRect.right, clientRect.bottom)), &triMesh, hWnd, token);

								// Отправить сообщение о том, что надо показать нарисованную триангуляционную сетку
								// в клиентской области окна приложения.
								PostMessage(hWnd, WM_APP_DRAW_TRIMESH, 0, 0);

								// Запись результатов триангуляции в файлы:
								DelaunayWriteToFile writeToFile;
								// запись вершин, отсортированных в процессе триангуляции, 
								writeToFile.WriteNodes(hWnd, &triMesh, &b, b.OutNodeFileName, token);
								// запись треугольников, полученных в процессе триангуляции,
								writeToFile.WriteElements(hWnd, &triMesh, &b, b.OutEleFileName, token);
								// запись рёбер этих треугольников,
								writeToFile.WriteEdges(hWnd, &triMesh, &b, b.OutEdgeFileName, token);
								// запись треугольников - соседей каждого треугольника,
								// который был получен в процессе триангуляции.
								writeToFile.WriteNeighbors(hWnd, &triMesh, &b, b.OutNeighborFileName, token);
							}
						}, token).then([&triMesh, &b, hWnd](task<void> currentTask)
							{
								// По окночанию триангуляции, выполнить деинициализацию приложения (освободить используемую память):
								DelaunayDeinitializition delaunayDeinit;
								delaunayDeinit.TriDeinit(&triMesh, &b);

								// Если приложение завершило работу без принудительной отмены со стороны пользователя,
								// то вывести сообщение о том, что триангуляция выполнена.
								// Иначе, вывести сообщение о том, что пользователь отменил триангуляцию.
								if (!f_TriangulationCanceled)
								{
									// запретить кнопку останова триангуляции.
									SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_STOP, MAKELONG(0, 0));
									// разрешить кнопку запуска триангуляции,
									SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_TRIANGULATE, MAKELONG(1, 0));
									// разрешить кнопку выбора входного файла,
									SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_INPUT, MAKELONG(1, 0));
									// разрешить кнопку выбора папки для файлов с выходными данными.
									SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_OUTPUT, MAKELONG(1, 0));
									// вывести сообщение о выполнении триангуляции.
									PostMessage(g_hWndStatusbar, SB_SETTEXTW, 0, (LPARAM)pMsgTriIsComplete);
									// сбросить индикатор выполнения в исходное состояние.
									PostMessage(g_hWndProgressbar, PBM_SETPOS, 0, 0);
								}
								else
								{
									f_TriangulationCanceled = false;
									PostMessage(g_hWndStatusbar, SB_SETTEXTW, 0, (LPARAM)pMsgTriIsCancelled);
								}

								// Выполнять очистку клиентской области окна приложения
								// при последующих запусках триангуляции.
								f_ClearClientArea = true;
							});

						// Ожидать окончания триангуляции.
						triangulationTask.get();
				});

			// запретить кнопку запуска триангуляции,
			SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_TRIANGULATE, MAKELONG(0, 0));
			// запретить кнопку выбора входного файла,
			SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_INPUT, MAKELONG(0, 0));
			// запретить кнопку выбора папки для файлов с выходными данными.
			SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_OUTPUT, MAKELONG(0, 0));
			// разрешить кнопку останова триангуляции.
			SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_STOP, MAKELONG(1, 0));
		}
		break;
		case IDM_STOP:        // Остановить триангуляцию:
		{
			// Установить флаг отмены триангуляции.
			f_TriangulationCanceled = true;
			// Отменить триангуляцию.
			cancelTriSource.cancel();
			// Ожидать окончания прерванной триангуляции.
			asyncTask.wait();
			// запретить кнопку останова триангуляции.
			SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_STOP, MAKELONG(0, 0));
			// разрешить кнопку запуска триангуляции,
			SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_TRIANGULATE, MAKELONG(1, 0));
			// разрешить кнопку выбора входного файла,
			SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_INPUT, MAKELONG(1, 0));
			// разрешить кнопку выбора папки для файлов с выходными данными.
			SendMessage(g_hWndToolbar, TB_ENABLEBUTTON, (WPARAM)IDM_OUTPUT, MAKELONG(1, 0));
		}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		OnPaint(hWnd, hdc);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_APP_DRAW_TRIMESH: // Нарисовать созданную триангуляционную сетку.
		InvalidateRect(hWnd, nullptr, TRUE);
		break;
	case WM_RESETPROGRESS:    // Установить индикатор выполнения в исходное состояние.
	{
		// Получить значение диапазона изменения для индикатора выполнения
		// (как результат приёма сообщения).
		range = (int)(lParam);
		// Установить диапазон изменения значения для индикатора выполнения.
		SendMessage(g_hWndProgressbar, PBM_SETRANGE32, 0, range);
		// Установить в единицу шаг изменения значения для индикатора выполнения.
		SendMessage(g_hWndProgressbar, PBM_SETSTEP, (WPARAM)1, 0);
		// Установить в ноль начальное значение для индикатора выплнения.
		SendMessage(g_hWndProgressbar, PBM_SETPOS, 0, 0);
	}
		break;
	case WM_UPDATEPROGRESS:   // Установить текущее значение у индикатора выполнения.
		SendMessage(g_hWndProgressbar, PBM_STEPIT, 0, 0);
		break;
	case WM_WRITESTATUSTEXT: // Показать информационное сообщение, принятое из другого файла,
	{                        // в панели состояния приложения.
		pInfoMessage = (wchar_t*)wParam;
		SendMessage(g_hWndStatusbar, SB_SETTEXTW, 0, (LPARAM)pInfoMessage);
		break;
	}
	case WM_DESTROY:
	{
		// Отменить задачи перерисовки.
		//redrawingTasks.cancel();
		// Ожидать окончания их выполнения всвязи с отменой.
		//redrawingTasks.wait();

		// Установить флаг отмены триангуляции.
		//f_TriangulationCanceled = true;
		// Отменить триангуляцию.
		//cancelTriSource.cancel();
		// Ожидать окончания отменённой триангуляции.
		//asyncTask.wait();

		PostQuitMessage(0);
	}
		break;
	case WM_CREATE:
	{
		// Создать панель управления приложением (toolbar).
		CreateToolbar(hWnd);
		// Создать панель состояния приложения.
		CreateStatusbar(hWnd, (int)ID_STATUS, GetModuleHandle(NULL), STATUS_BAR_PARTS);
		// Создать индикатор выполнения.
		CreateProgressbar();
		// Получить путь (по умолчанию) к входному *.NODE файлу приложения.
		wchar_t* pathToInputFile = nullptr;
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_PublicDocuments, KF_FLAG_DEFAULT, NULL, &pathToInputFile);
		if (SUCCEEDED(hr))
		{
			size_t iPathLength = std::char_traits<wchar_t>::length(pathToInputFile) + 1 + 33;
			hr = PathCchAppend(pathToInputFile, iPathLength, L"\\Triangulator\\Input\\tritest.node");
			if (SUCCEEDED(hr))
			{
				inFilePath = std::wstring(pathToInputFile);
			}
		}
		CoTaskMemFree(static_cast<void*>(pathToInputFile));
		// Получить путь (по умолчанию) к выходной папке приложения.
		wchar_t* pathToOutputFolder = nullptr;
		hr = SHGetKnownFolderPath(FOLDERID_PublicDocuments, KF_FLAG_DEFAULT, NULL, &pathToOutputFolder);
		if (SUCCEEDED(hr))
		{
			size_t oPathLength = std::char_traits<wchar_t>::length(pathToOutputFolder) + 1 + 22;
			hr = PathCchAppend(pathToOutputFolder, oPathLength, L"\\Triangulator\\Output\\");
			if (SUCCEEDED(hr))
			{
				outFolderPath = std::wstring(pathToOutputFolder);
			}
		}
		CoTaskMemFree(static_cast<void*>(pathToOutputFolder));
	}
	break;
	case WM_SIZING:
	{
		// Запретить перерисовку во время, когда непосредственно изменяется
		// размер окна, т.к. визуализировать изображение в клиентской области
		// нужно только после того, как завершится изменение размера окна.
		if (f_TriMeshIsReadyToDisplay)
		{
			redrawingTasks.cancel();
		}
	}
		break;
	case WM_SIZE:
	{
		// Изменить размер кнопочной панели управления приложением.
		SendMessage(g_hWndToolbar, TB_AUTOSIZE, 0, 0);
		// Изменить размер панели состояния приложения.
		int nWidth;
		int nWidths[STATUS_BAR_PARTS];
		nWidth = LOWORD(lParam);
		nWidths[0] = 1 * nWidth / STATUS_BAR_PARTS;
		nWidths[1] = 2 * nWidth / STATUS_BAR_PARTS;
		SendMessage(g_hWndStatusbar, SB_SETPARTS, STATUS_BAR_PARTS, (LPARAM)nWidths);
		SendMessage(g_hWndStatusbar, WM_SIZE, wParam, lParam);
		// Поместить индикатор выполнения в правую половину панели состояния приложения.
		RECT r1;
		SendMessage(g_hWndStatusbar, SB_GETRECT, 1, (LPARAM)&r1);
		MoveWindow(g_hWndProgressbar, r1.left + 3, r1.top + 3, r1.right - r1.left - 6, r1.bottom - r1.top - 6, TRUE);

		// Если ранее было выполнено рисование триангуляционной сетки в фоновом потоке,
		// то вследствии окончания изменения размера окна отменить все задачи рисования,
		// выполняющиеся на текущий момент, и ожидать их завершения.
		if (f_TriMeshIsReadyToDisplay)
		{
			redrawingTasks.cancel();
			redrawingTasks.wait();
		}
	}
		break;
	case WM_MOUSEWHEEL:
	{
		if (!f_Zooming)
		{
			static int nDelta = 0;
			nDelta += GET_WHEEL_DELTA_WPARAM(wParam);
			if (abs(nDelta) >= WHEEL_DELTA)
			{
				if (nDelta > 0)
				{
					nScale *= 1.1f;
				}
				else
				{
					nScale /= 1.1f;
				}
				nDelta = 0;
				////
				if (!f_Zoomed)
					f_Zoomed = true;
				f_Zooming = true;
				////
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
	}
		break;
	case WM_LBUTTONDOWN:
	{
		// Убедиться, что нажата именно левая клавиша мыши.
		if (wParam == MK_LBUTTON)
		{
			/*
			// Получить координаты курсора мыши.
			POINT ptCur;
			ptCur.x = GET_X_LPARAM(lParam);
			ptCur.y = GET_Y_LPARAM(lParam);
			// Инициировать процесс перетаскивания изображения триангуляционной сетки.
			BOOL res = StartDragging(hWnd, ptCur, himl);
			//*/
			if (!redrawnEdgesBuffer.empty() && !isnan(xMin) && !isnan(xMax) && !isnan(yMin) && !isnan(yMax))
			{
				// Получить исходные координаты курсора мыши.
				POINT ptInitialMouseCursorPosition;
				ptInitialMouseCursorPosition.x = GET_X_LPARAM(lParam);
				ptInitialMouseCursorPosition.y = GET_Y_LPARAM(lParam);
				xInitialMousePosition = (double)ptInitialMouseCursorPosition.x;
				yInitialMousePosition = (double)ptInitialMouseCursorPosition.y;
				xInitialMousePosition /= scaleFactor;
				yInitialMousePosition /= scaleFactor;
				// Установить флаг выполнения перетаскивания.
				f_Drag = true;
			}
		}
	}
		break;
	case WM_MOUSEMOVE:
	{
		// Убедиться, что нажата именно левая клавиша мыши.
		if (wParam == MK_LBUTTON)
		{
			/*
			// Получить координаты курсора мыши.
			POINT ptCur;
			ptCur.x = GET_X_LPARAM(lParam);
			ptCur.y = GET_Y_LPARAM(lParam);
			// Выполнять перетаскивание.
			BOOL res = MoveTheImage(ptCur);
			//*/
		}
	}
		break;
	case WM_LBUTTONUP:
	{
		/*
		// Получить координаты курсора мыши.
		POINT ptCur;
		ptCur.x = GET_X_LPARAM(lParam);
		ptCur.y = GET_Y_LPARAM(lParam);
		// Завершить "перетаскивание" и нарисовать изображение в месте,
		// в котором было завершено перетаскивание.
		BOOL res = StopDragging(hWnd, himl, ptCur);
		//*/
		if (!redrawnEdgesBuffer.empty() && !isnan(xMin) && !isnan(xMax) && !isnan(yMin) && !isnan(yMax))
		{
			// Если выполняется перетаскивание изображения:
			if (f_Drag)
			{
				// Получить текущие координаты курсора мыши.
				POINT ptFinalMouseCursorPosition;
				ptFinalMouseCursorPosition.x = GET_X_LPARAM(lParam);
				ptFinalMouseCursorPosition.y = GET_Y_LPARAM(lParam);
				double xFinalMousePosition = (double)ptFinalMouseCursorPosition.x;
				xFinalMousePosition /= scaleFactor;
				double yFinalMousePosition = (double)ptFinalMouseCursorPosition.y;
				yFinalMousePosition /= scaleFactor;
				// Вычислить поправочный коэффициент для значений координат X каждой вершины триангуляционной сетки.
				xValueCorrectionFactor = xFinalMousePosition - xInitialMousePosition;
				// Вычислить поправочный коэффициент для значений координат Y каждой вершины триангуляционной сетки.
				yValueCorrectionFactor = yInitialMousePosition - yFinalMousePosition;
				// Перерисовать изображение.
				InvalidateRect(hWnd, NULL, FALSE/*TRUE*/);
				//f_Drag = false;
			}
		}
	}
		break;
	case WM_NOTIFY:
	{
		// Вывести подсказку для каждой кнопки на тулбаре.
		switch (((LPNMHDR)lParam)->code)
		{
			case TTN_GETDISPINFO:
			{
				LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT)lParam;
				// Установить экземпляр модуля, который содержит строковый ресурс-подсказку.
				lpttt->hinst = hInst;
				// Получить кнопку, для которой требуется вывести подсказку.
				UINT_PTR idButton = lpttt->hdr.idFrom;
				// Определить, что это за кнопка:
				switch (idButton)
				{
				case IDM_INPUT:       // Выбор входного файла.
					lpttt->lpszText = MAKEINTRESOURCE(IDS_PICKINPUTFILE_RU);
					break;
				case IDM_OUTPUT:      // Выбор пути к папке, в которую запишутся выходные файлы.
					lpttt->lpszText = MAKEINTRESOURCE(IDS_PICKOUTFOLDER_RU);
					break;
				case IDM_TRIANGULATE: // Запуск триангуляции.
					lpttt->lpszText = MAKEINTRESOURCE(IDS_STARTTRI_RU);
					break;
				case IDM_STOP:        // Останов триангуляции.
					lpttt->lpszText = MAKEINTRESOURCE(IDS_STOPTRI_RU);
					break;
				}
			}
			break;
		}
	}
	break;
	case WM_CLOSE:
		if (MessageBox(hWnd, L"Вы действительно хотите выйти из приложения?", L"Триангулятор", MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
		{
			DestroyWindow(hWnd);
		}
		break;
	case WM_ACTIVATE:
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Выводит окно "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
