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
Главным файлом программы является файл DelaunayWin32.cpp. Он содержит, в частности, функцию wWinMain, являющуюся точкой входа в приложение. Функция wWinMain выполняет начальную
инициализацию - в частности графической библиотеки GDI+, создаёт главное окно приложения, кнопочную панель управления, панель состояния и индикатор выполнения. Кроме этого, в файле DelaunayWin32.cpp определена такая важная функция приложения Win32, как оконная процедура WndProc, которая, фактически, представляет собой основной функциональный узел программы. Ниже, я приведу перечень тех частей этой процедуры, которые считаю наиболее важными в работе программы. Некоторые я опишу очень кратко. На некоторых остановлюсь подробнее. Обработчик сообщения WM_COMMAND обрабатывает нажатия пунктов меню и кнопок на панели управления. Для этого он, в свою очередь, содержит ряд соответствующих обработчиков сообщений, среди которых есть следующие:

