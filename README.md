# DelaunayWin32
C ++ Win 32 application that triangulates an original point cloud.
Below, dear readers, everything is in Russian - in the language of Leo Tolstoy and Ivan Turgenev.
So.

Описание программы

Общие сведения
Обзначение и наименование программы

Предлагаемая здесь вашему вниманию программа (точнее её проект и исходный текст) имеет наименование Delaunayin32. Вкрадце (не описывая прелюдию её создания), я сделал её следующим образом. Взял программу "triangle.c" Джонотана Ричарда Шевчука https://github.com/libigl/triangle/blob/master/triangle.c. Взял из неё ту её часть, которая выполняет триангуляцию Деолоне по алгоритму "Разделяй и властвуй", частично перевёл взятый код с Си на C++, добавив в некоторых местах параллелизм (использование библиотеки PPL), "обернул" то что получилось в оконный интерфейс Win32 API и, в итоге, осмелился считать получившееся приложение (далее по тексту этого документа - программа) своей программой своего собственного создания.

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

Функция wWinMain выполняет начальную инициализацию (в том числе графической библиотеки GDI+), вызывает функции создания главного окна программы, кнопочной панели управления, панели состояния программы и индикатора выполнения. Кроме этого, в файле DelaunayWin32.cpp определена такая важная функция приложения Win32, как оконная процедура WndProc, которая, фактически, представляет собой основной функциональный узел программы. Ниже, я приведу перечень тех частей этой процедуры, которые считаю наиболее важными в работе программы. Некоторые я опишу очень кратко. На некоторых остановлюсь подробнее. 
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

Здесь hWnd это идентификатор главного окна программы, с которым будет взаимодействовать код, работающий в задаче, и, выполняющий триангуляцию. Именно по этой причине этот идентификатор и передаётся задаче. Затем из этой задачи создаётся и запускается ещё одна параллельная задача triangulationTask, которая непосредственно выполняет триангуляцию. В задаче triangulationTask объявляются объектные переменные. Внутри типов этих переменных определены функции, инициализирующие выполнение триангуляции, выполняющие триангуляцию, и, записывающие результаты выполнения триангуляции на жёсткий диск компьютера.
