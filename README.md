# DelaunayWin32
C ++ Win 32 application that triangulates an original point cloud.
Below, dear readers, everything is in Russian - in the language of Leo Tolstoy and Ivan Turgenev.
So.

Описание программы

Общие сведения
Обзначение и наименование программы
Предлогаемая здесь вашему вниманию программа (точнее её проект и исходный текст) имеет наименование Delaunayin32. Для её функционирования необходимо следующее программное обеспечение:
- Операционная система Windows 10 (например я - ваш покорный слуга и автор этой программы использую Windows 10 Pro),
- Среда разработки программ Microsoft Visual Studio 2017 для выполнения компиляции программы (я использую Microsoft Visual Studio 2017 Community).
Программа является оконным приложением Win32 и написана на языке C++.

Функциональное назначение
Программа предназначена для выполнения триангуляции исходного облака точек, заданного в входном файле с расширением .node, и рисования, в своём главном окне, полученного
изображения триангуляционной сетки.

Описание логической структуры
Ниже приводится описание структуры программы.
Главным файлом программы является файл DelaunayWin32.cpp. Он содержит, в частности, функцию wWinMain, являющуюся точкой входа в приложение, функцию DrawTriMesh, выполняющую начальное рисование триангуляционной сетки, функцию RedrawTriMesh, выполняющую перерисовку триангуляционной сетки при изменении размера окна приложения
или при перетаскивании её изображения. В файле DelaunayWin32.cpp также определены несколько служебных функций: функция SetRequiredDimensions выполняет определение ширины и высоты растрового изображения и высоты кнопочной панели управления, функция SetScale вычисляет и возвращает значение коэффициента масштабирования для рисования триангуляционной сетки, функция OnPaint обрабатывает сообщение WM_PAINT.
Функция wWinMain выполняет начальную инициализацию (в том числе графической библиотеки GDI+), вызывает функции создания главного окна приложения, кнопочной панели управления, панели состояния приложения и индикатора выполнения. Кроме этого, в файле DelaunayWin32.cpp определена такая важная функция приложения Win32, как оконная процедура WndProc, которая, фактически, представляет собой основной функциональный узел программы. Ниже, я приведу перечень тех частей этой процедуры, которые считаю наиболее важными в работе программы. Некоторые я опишу очень кратко. На некоторых остановлюсь подробнее. 
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

После выполнения приведённой выше последовательности операторов запускается параллельно выполняемая задача asyncTask. В ней  инициализируется токен отмены, объявляются буфер для данных триангуляционной сетки (получаемых при выполнении триангуляции) и буфер для настроечных параметров приложения. Затем из этой задачи создаётся и запускается ещё одна параллельная задача triangulationTask, которая непосредственно выполняет триангуляцию. В задаче triangulationTask объявляются объектные переменные. Внутри типов этих переменных определены функции, инициализирующие выполнение триангуляции, выполняющие триангуляцию, и, записывающие результаты выполнения триангуляции на жёсткий диск компьютера.
