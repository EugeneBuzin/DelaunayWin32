# DelaunayWin32
C ++ Win 32 application that triangulates an original point cloud.
Below, dear readers, everything is in Russian - in the language of Leo Tolstoy and Ivan Turgenev.
So.

Описание программы

Общие сведения
Обзначение и наименование программы

Предлагаемая здесь вашему вниманию программа (точнее её проект и исходный текст) имеет наименование Delaunayin32.

При разработке своей программы я часто обращался к приложению triangle.c https://github.com/libigl/triangle/blob/master/triangle.c  Джонотана Ричарда Шевчука https://people.eecs.berkeley.edu/~jrs/ в качестве образца и основы.

Для функционирования программы необходимо следующее программное обеспечение:
- Операционная система Windows 10 (например, я использую Windows 10 Pro),
- Среда разработки программ Microsoft Visual Studio 2017 для выполнения компиляции программы (я использую Microsoft Visual Studio 2017 Community).

Программа является оконным приложением Win32 и написана на языке C++.

Функциональное назначение
Программа предназначена для выполнения:
- триангуляции исходного облака точек, заданного в входном файле с расширением .node, и рисования, в своём главном окне, полученного изображения триангуляционной сетки, состоящего из рёбер, соединяющих эти точки или, как ещё можно сказать - вершины триангуляционной сетки,
- записи результатов триангуляции в файлы, имеющие следующие расширения:
  - node - выходной файл вершин триангуляционной сетки, порядок следования вершин в выходном файле .node может отличаться от такового в входном файле .node из-за того, что в процессе триангуляцииможет быть изменён программой,
  - ele - выходной файл треугольников, получившихся при триангуляции,
  - edge - выходной файл рёбер, получившейся триангуляционной сетки,
  - neigh - выходной файл треугольников-соседей каждого получившегося при триангуляции треугольника.
  
На моём компьютере, путь по умолчанию для входных файлов следующий:

C:\Users\Public\Documents\Triangulator\Input

В программе он определяется следующим образом:

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
 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
 
 Если папка C:\Users\Public\Documents\Triangulator\Input не содержит входной файл, то он будет сгенерирован программой. См. функцию DelaunayInitialization::setOperatingParameters в файле DelaunayInitialization.cpp.

На моём компьютере, путь по умолчанию для выходных файлов следующий:

C:\Users\Public\Documents\Triangulator\Output

В программе он определяется следующим образом:

case WM_CREATE:
{
 . . . . . . . . . . . . . . . .
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

Как видно из строки кода case WM_CREATE: в снипетах исходного кода, приведённых выше, путь по умолчанию к входным и выходным данным определяется при создании окна приложения во время выполнения функции CreateWindowW.

Описание логической структуры

Ниже приводится описание структуры программы.

Главным файлом программы является файл DelaunayWin32.cpp. Он содержит, в частности, функцию wWinMain, являющуюся точкой входа в программу, функцию DrawTriMesh, выполняющую начальное рисование триангуляционной сетки, функцию RedrawTriMesh, выполняющую перерисовку триангуляционной сетки при изменении размера окна программы
или при перетаскивании изображения триангуляционной сетки по клиентской области главного окна программы. В файле DelaunayWin32.cpp также определены несколько служебных функций: функция SetRequiredDimensions выполняет определение ширины и высоты растрового изображения и высоты кнопочной панели управления, функция SetScale вычисляет и возвращает значение коэффициента масштабирования для рисования триангуляционной сетки, функция OnPaint обрабатывает сообщение WM_PAINT.

Функция wWinMain выполняет начальную инициализацию (в том числе графической библиотеки GDI+), вызывает функции: регистрации главного окна программы - MyRegisterClass, инициализации (вывода на экран) главного окна программы - InitInstance, создания кнопочной панели управления - CreateToolbar, создания панели состояния программы - CreateStatusbar и создания индикатора выполнения CreateProgressbar(). Кроме этого, в файле DelaunayWin32.cpp определена такая важная функция приложения Win32, как оконная процедура WndProc, которая, фактически, представляет собой основной функциональный узел программы. Ниже, я приведу перечень тех частей этой процедуры, которые считаю наиболее важными в работе программы. Некоторые я опишу очень кратко. На некоторых остановлюсь подробнее. 
Обработчик сообщения WM_COMMAND обрабатывает нажатия пунктов меню и кнопок на панели управления. Для этого он, в свою очередь, содержит ряд соответствующих обработчиков сообщений, среди которых есть следующие (опять же я здесь не буду упоминать все):
IDM_INPUT - обрабатывает нажатие пользователем кнопки выбора входного файла. Выводит на экран диалог для выбора входного файла.
IDM_OUTPUT - обрабатывает нажатие пользователем кнопки выбора папки, в которую будут записываться выходные файлы с результатами триангуляции. Выводит на экран диалог для выбора папки.
IDM_TRIANGULATE - это обработчик основного сообщения. Он обрабатывает нажатие пользователем кнопки запуска триангуляции. В самом начале обработки сообщения IDM_TRIANGULATE выполняется очистка клиентской области главного окна приложения, очистка буфера ребер триангуляционной сетки redrawnEdgesBuffer, сброс флага готовности триангуляционной сетки к отображению на экране f_TriMeshIsReadyToDisplay, сброс флага завершения масштабирования изображения триангуляционной сетки f_Zoomed и обнуление и сброс поправочных коэффициентов. Ниже приводится часть исходного текста, в которой выполняются эти операции.

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
        // Сбросить поправочный коэффициент масштабирования,
	// используемый при "перетаскивании" изображения
	scaleFactor = 1.0;
	// Сбросить коэффициент масштабирования изображения при повороте колёсика мыши.
	nScale = 1.0;
. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .	

После выполнения приведённой выше последовательности операторов запускается параллельно выполняемая задача asyncTask. В ней создаётся источник токенов отмены, объявленный ранее в строке 57 программы,

static cancellation_token_source cancelTriSource;

и из него берётся токен отмены. Затем, объявляются буфер для данных триангуляционной сетки (получаемых при выполнении триангуляции) и буфер для настроечных параметров приложения. Ниже приводится часть исходного текста, в которой выполняются эти операции. 

asyncTask = create_task([hWnd]()
{
	// Получить токен отмены для задачи triangulationTask.
	cancelTriSource = cancellation_token_source();
	cancellation_token token = cancelTriSource.get_token();
	// Буфер для данных триангуляционной сетки, которые будут получены в процессе триангуляции.
	Mesh triMesh;
	// Настроечные параметры для приложения
	Configuration b;
 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .	

Здесь hWnd это идентификатор главного окна программы, с которым будет взаимодействовать код, работающий в задаче, и, выполняющий триангуляцию. Именно по этой причине этот идентификатор и передаётся задаче. 
Переменная cancelTriSource объявлена в области видимости файла DelaunayWin32.cpp в строке 57. Это сделано из-за того, что доступ к ней, внутри оконной процедуры WndProc, осуществляется ещё из обработчика сообщения IDM_STOP - останова и отмены выполнения триангуляции. А объявлять cancelTriSource внутри WndProc мне не хотелось, что бы "не засорять" оконную процедуру локальными переменными.
Затем внутри задачи asyncTask создаётся и запускается ещё одна параллельно-выполняемая задача triangulationTask, которая непосредственно выполняет триангуляцию. После запуска triangulationTask, задача asyncTask ожидает завершения её выполнения, см. строку 1067 исходного текста файла DelaunayWin32.cpp. Ниже приводится данная строчка исходного текста:

triangulationTask.get();

Задача asyncTask продолжит своё выполнение и завершится после того, как по завершению задачи triangulationTask выполнится задача-продолжение этой последней. Эта задача-продолжение (см. её начало в строке 1031 в исходном тексте файла DelaunayWin32.cpp) выполняет деинициализацию программы путём вызова определённой в программе функции TriDeinit, изменяет состояние кнопок на панели управления и устанавливает флаг очистки f_ClearClientArea клиентской области главного окна программы, подготавливая тем самым программу к следующим запускам триангуляции. (Функция TriDeinit объявлена в классе DelaunayDeinitializition, см файл DelaunayDeinitializition.h, и определяется в файле DelaunayDeinitializition.cpp в строках 15 - 20.)

Во время выполнения задачи triangulationTask, в ней объявляются объектные переменные, типы которых определяются в программе в виде классов. Внутри этих типов определены функции, инициализирующие выполнение триангуляции, выполняющие триангуляцию, и, записывающие результаты выполнения триангуляции на жёсткий диск компьютера. Выполнение процесса триангуляции показывается в программе с помощью индикатора выполнения (progress bar). Взаимодействие между функциями выполнения триангуляции, определёнными в объектных типах и оконной процедурой WinProc, управляющей, в частности, видом индикатора выполнения, происходит посредством посылки сообщений из функций выполнения триангуляции в главное окно приложения. Для посылки сообщений используется функция PostMessage из Win32 API. Изменение состояния индикатора выполнения происходит в обработчике сообщения WM_UPDATEPROGRESS.
Ниже описывается порядок выполнения триангуляции в задаче triangulationTask:
- Инициализация используемых переменных,
- Чтение исходных данных (координат точек, которые будут вершинами триангуляционной сетки) из исходного файла,
- Выделение памяти для треугольников, которые будут созданы,
- Запуск и выполнение триангуляции,
- Рисование триангуляционной сетки по результатам триангуляции,
- Запись результатов триангуляции в файлы:
  - запись вершин, отсортированных в процессе триангуляции,
  - запись треугольников, полученных в процессе триангуляции,
  - запись рёбер этих треугольников,
  - запись треугольников-соседей каждого треугольника, который был получен в процессе триангуляции.

В функциях программы, которые содержат циклы, выполняется проверка токена отмены. Т.е., проверяется отменил ли пользователь выполнение триангуляции или нет. Это делается по приведённой ниже схеме:

if (token.is_canceled())
{
    // Если состояние токена отмены canceled, то выполнить отмену задачи PPL,
    // в которой выполняется функция, содержащая эту проверку.
    concurrency::cancel_current_task(); 
}
else
{
    // Выполнять код функции.
    . . . . . . . . . . . . . . . . . . . . . . .
}   

Ниже, выполнение триангуляции описывается более подробно.

Инициализация используемых переменных, чтение исходных данных, выделение памяти для треугольников

Инициализация начинается с вызова функции DelaunayInitialization::TriInit, (мм. её определение начиная с строки 40 файла DelaunayInitialization.cpp). Сначала в ней выполняется вызовы функции DelaunayInitialization::setPoolToZero для обнуление пулов триангуляции см. функцию. Затем вызывается DelaunayInitialization::setOperatingParameters для установки рабочих параметров программы и создания входного файла по умолчанию (последнее выполняется, если файл по умолчанию не существует на компьтере пользователя). Следующим шагом, вызывается функция DelaunayHelper::DelaunayHelperInit для инициализации вспомогательных переменных из файла DelaunayHelper.cpp. После этого выполняется чтение данных из исходного файла .node (см. функцию ReadNodes, определение которой начинается в строке 57 файла DelaunayInitialization.cpp). В ней проверяется существование входного файла

infile = _wfopen(nodefilename.c_str(), L"r");

(см. строку 82 в файле DelaunayInitialization.cpp). Здесь nodefilename это буфер для имени входного файла. Входным файлом может быть либо файл .node по умолчанию, либо файл .node, выбранный пользователем. Если файл открывается, то выполняется чтение его содержимого. Иначе, выводится сообщение об ошибке и, и после того, как пользователь зарывает диалог с этим сообщением, осуществляется выход из программы. После того, как исходные данные были прочитаны, файл закрывается. Затем, вызывается функция DelaunayInitialization::InitializeTriSubpools (её определение см. в строках 273 - 307). Эта функция выполняет рассчёт размера структуры данных для треугольника и инициализацию его пула памяти.

Запуск и выполнение триангуляции

После выполнения инициализации и чтения исходных данных, в программе определяется объектная переменная delaunayTri класса DelaunayTriangulation и вызывается функция DelaunayTriangulation::StartTriangulation, являющаяся членом класса DelaunayTriangulation. Определение этой функции см. в файле DelaunayTriangulation.cpp. Внутри DelaunayTriangulation::StartTriangulation определяется объектная переменная divideAndConquerTriangulation класса DivideAndConquer. Затем, относительно неё, выполняется вызов функции DivideAndConquer::DivconqDelaunay, которая выполняет триангуляцию Делоне по алгоритму "Разделяй и властвуй". Внутри функции DivideAndConquer::DivconqDelaunay выполняются следующие операции:
- Создание и наполнение массива вершин, предназначенных для сортировки,
- Сортировка вершин (точек, координаты которых были прочитаны из входного файла при инициализации приложения,
- Удаление дублирующихся вершин (дубликатов точек, прочитанных из входного файла),
- Рассечение имеющегося множества вершин на отдельные части - "рассечения" и сортировка вершин внутри "рассечений" (полученные "рассечения", в свою очередь, могут рассекаться на
  более мелкие "рассечения"),

Затем, вызывается функция DivideAndConquer::divconqRecurse, которая собственно и выполняет триангуляцию Делоне. Определение этой функции см. в файле DivideAndConquer.cpp в строках 421 - 579. По выполнению триангуляции происходит удаление технологических треугольников (которые, в программе, носят название призрачных). Это вспомогательные треугольники, которые создавались в процессе триангуляции, но которые не нужны в полученной триангуляционной сетке. Эту операцию выполняет функция DivideAndConquer::removeGhosts, определённая в строках 586 - 676 файла DivideAndConquer.cpp с учётом находящегося в ней закоментированного исходного текста.

Визуально, выполнение триангуляции контролируется индикатором выполнения в главном окне программы в правой части панели состояния программы. Данные для изменения внешнего вида индикатора выполнения передаются в главного окно программы из функций, выполняемых в задачие triangulationTask, путём вызовов функции PostMessage Win32 API.
