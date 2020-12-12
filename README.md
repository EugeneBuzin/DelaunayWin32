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

Программа предназначена для выполнения триангуляции Делоне по алгоритму "Разделяй и властвуй".

В алгоритме триангуляции «Разделяй и властвуй» множество исходных точек (вершин) точек разбивается на две как можно более равные части с помощью горизонтальных и вертикальных рассечений множество исходных точек. Алгоритм триангуляции рекурсивно применяется к подчастям, а затем производится слияние (объединение, склеивание) полученных подтриангуляций. Рекурсия прекращается при разбиении всего множества на достаточно маленькие части, которые можно легко протриангулировать каким-нибудь другим простым способом. На практике удобно разбивать всё множество на части по 3 и по 4 точки.

Рекурсивный алгоритм триангуляции «Разделяй и властвуй»:
1. Если число точек N = 3, то построить триангуляцию из 1 тре-
угольника.
2. Иначе, если число точек N = 4 , построить триангуляцию из 2
или 3 треугольников.
3. Иначе, если число точек N = 8, разбить множества точек на
две части по 4 точки, рекурсивно применить алгоритм, а затем "склеить"
триангуляции.
4. Иначе, если число точек N <12 , разбить множества точек на
две части по 3 и N − 3 точки, рекурсивно применить алгоритм, а затем
"склеить" триангуляции.
5. Иначе (число точек N ≥12 ) разбить множества точек на две
части по N/2 и N/2 точки, рекурсивно применить алгоритм, а затем
"склеить" триангуляции. 
6. Конец алгоритма.

При работе программы выполняется:
- триангуляция исходного облака точек, заданного в входном файле с расширением .node, и рисования, в своём главном окне, полученного изображения триангуляционной сетки, состоящего из рёбер, соединяющих эти точки или, как ещё можно сказать - вершины триангуляционной сетки,
- запись результатов триангуляции в файлы, имеющие следующие расширения:
  - node - выходной файл вершин триангуляционной сетки, порядок следования вершин в выходном файле .node может отличаться от такового в входном файле .node из-за того, что в процессе триангуляцииможет быть изменён программой,
  - ele - выходной файл треугольников, получившихся при триангуляции,
  - edge - выходной файл рёбер, получившейся триангуляционной сетки,
  - neigh - выходной файл треугольников-соседей каждого получившегося при триангуляции треугольника.

Ниже, в качестве примера, приводится содержимое входного файла spiral.node.

15  2  0  0
 1      0       0
 2     -0.416   0.909
 3     -1.35    0.436
 4     -1.64   -0.549
 5     -1.31   -1.51
 6     -0.532  -2.17
 7      0.454  -2.41
 8      1.45   -2.21
 9      2.29   -1.66
10      2.88   -0.838
11      3.16    0.131
12      3.12    1.14
13      2.77    2.08
14      2.16    2.89
15      1.36    3.49

Файл может содержать комментарии, помеченные символом #. Первая строка содержит следующую информацию (слева на право):
- количество вершин (исходных точек для триангуляции),
- размерность триангуляции (всегда двухмерная),
- количество атрибутов (не используется),
- количество граничных маркеров (не используется).

Каждая последующая строка содержит: 
- порядковый номер вершины, 
- значение координаты X вершины, 
- значение координаты Y вершины.

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

Ниже приводится информация по выходным данным программы.

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

Примеры выходных файлов программы приводятся ниже.

В качестве примера, ниже приводится полученный в результате триангуляции файл вершин триангуляционной сетки spiral.1.node. Формат выходного файла .node соответствует формату входного файла. Порядок следования вершин может отличаться от такового в входном файле .node, а может и не отличаться.

15  2  0  1
   1    0  0    0
   2    -0.41599999999999998  0.90900000000000003    0
   3    -1.3500000000000001   0.436                  1
   4    -1.6399999999999999   -0.54900000000000004   1
   5    -1.3100000000000001   -1.51                  1
   6    -0.53200000000000003  -2.1699999999999999    1
   7    0.45400000000000001   -2.4100000000000001    1
   8    1.45                  -2.21                  1
   9    2.29                  -1.6599999999999999    1
  10    2.8799999999999999    -0.83799999999999997   1
  11    3.1600000000000001    0.13100000000000001    1
  12    3.1200000000000001    1.1399999999999999     1
  13    2.77                  2.0800000000000001     1
  14    2.1600000000000001    2.8900000000000001     1
  15    1.3600000000000001    3.4900000000000002     1
  
Хотел и хочу убрать третью колонку, состоящую из нулей и единиц, и, представляющую атрибуты. Извиняюсь - некогда.

Файл треугольников (.ele)

В качестве примера, ниже приводится полученный в результате триангуляции файл треугольников spiral.1.ele.

15  3  0
   1       1     5     6
   2       1     6     7
   3       3     4     1
   4       2     1    13
   5       1     2     3
   6       4     5     1
   7       3     2    15
   8       7     8     1
   9       9    10     1
  10      11     1    10
  11      11    12     1
  12       2    14    15
  13      14     2    13
  14      13     1    12
  15       8     9     1
  
  Его первая строка содержит следующую информацию (справа на лево):
  - количество треугольников,
  - количество узлов (вершин) в треугольнике (всегда три),
  - количество атрибутов (не используется).
  
  Каждая последующая строка содержит:
  
  - порядковый номер треугольника,
  - номер первого узла (вершины) треугольника, ему соответствует номер одной из вершин в выходном файле .node
  - номер второго узла (вершины) треугольника, ему соответствует номер одной из вершин в выходном файле .node
  - номер третьего узла (вершины) треугольника, ему соответствует номер одной из вершин в выходном файле .node
  
  Файл рёбер треугольника .edge
  
  В качестве примера, ниже приводится полученный в результате триангуляции файл рёбер треугольников spiral.1.edge. Фактически, это файл рёбер, образующих триангуляционную сетку.
  
  Его первая строка содержит следующую информацию (справа на лево):
  
  - количество треугольников,
  - количество граничных маркеров (не используется, хотя и присутствует в строках ниже).
  
  Каждая последующая строка содержит:
  
  - номер первого узла (вершины), ограничивающего ребро с его начала, ему соответствует номер одной из вершин в выходном файле .node,
  - номер второго узла (вершины), ограничивающего ребро с его конца, ему соответствует номер одной из вершин в выходном файле .node,
  - граничный маркер (хочу убрать - нет времени)
  
  29  1
   1   1  5  0
   2   5  6  1
   3   6  1  0
   4   6  7  1
   5   7  1  0
   6   3  4  1
   7   4  1  0
   8   1  3  0
   9   2  1  0
  10   1  13  0
  11   13  2  0
  12   2  3  0
  13   4  5  1
  14   2  15  0
  15   15  3  1
  16   7  8  1
  17   8  1  0
  18   9  10  1
  19   10  1  0
  20   1  9  0
  21   11  1  0
  22   10  11  1
  23   11  12  1
  24   12  1  0
  25   2  14  0
  26   14  15  1
  27   13  14  1
  28   12  13  1
  29   8  9  1
  
  Файл треугольников-соседей, примыуающих к каждому треугольнику с трёх его сторон
  
  В качестве примера, ниже приводится полученный в результате триангуляции файл треугольников-соседей spiral.1.neigh.
  
  15  3
   1    -1  2  6
   2    -1  8  1
   3    6  5  -1
   4    14  13  5
   5    7  3  4
   6    1  3  -1
   7    12  -1  5
   8    15  2  -1
   9    10  15  -1
  10    9  -1  11
  11    14  10  -1
  12    -1  7  13
  13    4  -1  12
  14    11  -1  4
  15    9  8  -1
  
  Его первая строка содержит следующую информацию (справа на лево):
  
  - количество треугольников в триангуляционной сетке,
  - количество количество треугольников-соседей у каждого треугольника.
  
  Каждая последующая строка содержит следующую информацию (справа на лево):
  - порядковый номер треугольника,
  - порядковый номер первого треугольника-соседа (ему соответствует порядковый номер одного из треугольников в выходном файле .ele),
  - порядковый номер второго треугольника-соседа (ему соответствует порядковый номер одного из треугольников в выходном файле .ele),
  - порядковый номер третьего треугольника-соседа (ему соответствует порядковый номер одного из треугольников в выходном файле .ele)
  
Число (-1) обозначает отсутствие треугольника-соседа.  

ОПИСАНИЕ ЛОГИЧЕСКОЙ СТРУКТУРЫ

НИЖЕ ПРИВОДИТСЯ ОПИСАНИЕ СТРУКТУРЫ ПРОГРАММЫ.

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

После выполнения инициализации и чтения исходных данных, в программе определяется объектная переменная delaunayTri класса DelaunayTriangulation и вызывается функция DelaunayTriangulation::StartTriangulation, являющаяся членом класса DelaunayTriangulation. Определение этой функции см. в файле DelaunayTriangulation.cpp. Внутри DelaunayTriangulation::StartTriangulation определяется объектная переменная divideAndConquerTriangulation класса DivideAndConquer. Затем, относительно неё, выполняется вызов функции DivideAndConquer::DivconqDelaunay, которая выполняет триангуляцию Делоне по алгоритму "Разделяй и властвуй". 

Внутри функции DivideAndConquer::DivconqDelaunay выполняются следующие операции:
- Создание и наполнение массива вершин, предназначенных для сортировки,
- Сортировка вершин (точек, координаты которых были прочитаны из входного файла при инициализации приложения),
- Удаление дублирующихся вершин (дубликатов точек, прочитанных из входного файла),
- Рассечение имеющегося множества вершин на отдельные части - "рассечения" и сортировка вершин внутри "рассечений" (полученные "рассечения", в свою очередь, могут рассекаться на
  более мелкие "рассечения"),

Затем, из функции DivideAndConquer::DivconqDelaunay вызывается функция DivideAndConquer::divconqRecurse, которая собственно и выполняет триангуляцию Делоне. Определение этой функции см. в файле DivideAndConquer.cpp в строках 421 - 579. По выполнению триангуляции происходит удаление технологических треугольников (которые, в программе, носят название призрачных). Это вспомогательные треугольники, которые создавались в процессе триангуляции, но которые не нужны в полученной триангуляционной сетке. Эту операцию выполняет функция DivideAndConquer::removeGhosts, определённая в строках 586 - 676 файла DivideAndConquer.cpp с учётом находящегося в ней закоментированного исходного текста.

Визуально, выполнение операций триангуляции контролируется индикатором выполнения. Он находится в главном окне программы в правой части панели состояния программы и имеет вид полосы зелёного цвета. Её ограничена правой границей панели состояния. Высота - немного поменьше, чем высота панели состояния. Чем больше степень выполнения операции, тем большую длину имеет эта полоса. Данные для изменения внешнего вида индикатора выполнения передаются в главного окно программы из функций, выполняемых в задачие triangulationTask, путём вызовов функции PostMessage из Win32 API.

После того, как выполнение триангуляции завершено и полученная триангуляционная сетка нарисована в клиентской области главного окна приложения, выполняется запись выходных данных программы (результатов триангуляции) на жёсткий диск компьютера пользователя. Как было рассказано выше, выходными данными программы являются:
- вершины, отсортированные в процессе триангуляции (файл с расширением .node), запись на диск выполняется функцией
- треугольники, полученные в процессе триангуляции (файл с расширением .ele), 
- рёбера этих треугольников (файл с расширением .edge), 
- треугольники-соседи каждого треугольника, полученого в процессе триангуляции (файл с расширением .neigh).

Для записи выходных данных на жесткий диск в программе определён класс DelaunayWriteToFile, определённый в файле DelaunayWriteToFile.h, и, содержащий следующие функции, выполняющие запись: 

- функция DelaunayWriteToFile::WriteNodes, определённая в строках 23 - 123 файла DelaunayWriteToFile.cpp, записывает отсортированные при триангуляции вершины в выходной файл *.node.;
- функция DelaunayWriteToFile::WriteElements, определённая в строках 126 - 233 файла DelaunayWriteToFile.cpp, записывает полученные при триангуляции треугольники в выходной файл *.ele;
- функция DelaunayWriteToFile::WriteEdges, определённая в строках 236 - 367 файла DelaunayWriteToFile.cpp, записывает полученные при триангуляции рёбра треугольников в выходной файл *.edge;
- функция DelaunayWriteToFile::WriteNeighbors, определённая в строках 236 - 367 файла DelaunayWriteToFile.cpp, записывает данные по треугольникам-соседям каждого треугольника в выходной файл .neigh;

Напоминаю, что путь по умолчанию к файлам выходных данных на компьютере пользователя это C:\Users\Public\Documents\Triangulator\Output.

Ниже я продолжаю перечень сообщений, обрабатываемых оконной процедурой.

IDM_STOP - обрабатывает останов и отмену триангуляции.

WM_PAINT и WM_APP_DRAW_TRIMESH - основное назначение - выполнять рисование триангуляционной сетки в клиентской области главного окна приложения.

WM_RESETPROGRESS - устаналивает индикатор выполнения в исходное состояние.

WM_UPDATEPROGRESS - обрабатывает изменение состояния индикатора выполнения.

WM_WRITESTATUSTEXT - обрабатывает вывод текстаинформационного собщения в панели состояния приложения.

WM_CREATE - при обработке этого сообщения выполняются следующие действия: 
- создаётся панель управления приложением,
- создаётся панель состояния приложения,
- создаётся индикатор выполнения,
- определяется путь (по умолчанию) к входному .NODE файлу приложения,
- определяется путь по умолчанию к папке, в которую будут записаны выходные файлы приложения,

WM_SIZING - запрещает перерисовку содержимого клиентской области во время, когда непосредственно изменяется размер окна, т.к. визуализировать изображение в клиентской области нужно только после того, как завершится изменение размера окна.

WM_MOUSEWHEEL - обрабатывает прокрутку колёсика мыши для изменения размера рисунка триангуляционной сетки в клинтской части главного окна приложения. Подсчитывает коэффициент масштабирования для изменения размера рисунка. Инициирует перерисовку изображения триангуляционной сетки.

WM_LBUTTONDOWN, WM_MOUSEMOVE обрабатывают перемещение ("перетаскивание") изоражение триангуляционной сетки в клиентско йобласти главного окна приложения.

WM_LBUTTONUP - обрабатывает ситуацию, когда пользователь отпускает левую клавишу мыши, которую он удерживал в нажатом состоянии при перемещении ("перетаскивании") изображения триангуляционной сетки. Инициирует перерисовку изображения триангуляционной сетки, после того, как завершилось его перемещение ("перетаскивание") в клиентской области главного окна приложения.

WM_NOTIFY - обрабатывает вывод подсказок для кнопок, расположенных на панели управления программой.

WM_CLOSE - обрабатывает закрытие окна программы пользователем.

Ниже приводится перечень исходных файлов (.cpp) программы с кратким пояснением к каждому из них.

DelaunayInitialization.cpp - подготовка программы к работе.

DelaunayTriangulation.cpp - запуск процесса триангуляции.

DivideAndConquer.cpp - выполнение триангуляции Делоне по алгоритму "Разделяйи властвуй".

DelaunayWriteToFile.cpp - запись результатов выполнения триангуляции на жесткий диск компьютера пользователя.

DelaunayHelper.cpp - содержит два пространства имён: DelaunayHelper и UsualHelper.  Пространство имён DelaunayHelper содержит, в свою очередь, следующие пространства имён:
- пространство имён AdaptiveArith включающее в себя вспомогательные подставляемые (inline) функции адаптивной арифметики, используемые при выполнении триангуляции Делоне,
- пространство имён Triangles включающее в себя вспомогательные подставляемые (inline) функции для работы с треугольниками, используемые при выполнении триангуляции Делоне.
Кроме этого, пространство имён DelaunayHelper содержит обычные (не inline) функции и переменные, которые используются и вызываются при выполнении триангуляции Делоне.
Пространство имён UsualHelper содержит вспомогательные функции общего характера, они используются при работе программы, но не относятся к триангуляции Делоне. Например, функции управления памятью.

DelaunayDeinitializition.cpp - содержит функции освобождения памяти, используемой приложением.

DialogService.cpp - реализует создание диалоговых окон для выбора файлов и папок в программе.

DialogEventHandler.cpp - реализует обработку событий диалогов.
