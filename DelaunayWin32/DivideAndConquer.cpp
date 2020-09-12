#include "stdafx.h"
#include "DivideAndConquer.h"

// Минимальное значение координаты X для исходных точек, по которым строится триангуляция.
//extern double xMin;
// Максимальное значение координаты X для исходных точек, по которым строится триангуляция.
//extern double xMax;
// Максимальное значение координаты Y для исходных точек, по которым строится триангуляция. 
//extern double yMin;
// Максимальное значение координаты Y для исходных точек, по которым строится триангуляция.
//extern double yMax;

// Конструктор.
DivideAndConquer::DivideAndConquer()
	:RANGE(100), SORTING_FACTOR(1.28171855247),
	ALTERNATING_FACTOR(0.72052397704), TRIANGULATION_FACTOR(1.027),
	APPROXIMATE_HULL_SIZE(10)/*, MINIMAL_TRIANGULATION(3)*/
{
	// Счётчик рекурсий.
	recursionCounter = 0;
	// Коэффициент кратности. Используется для разрешения отправки сообщения,
	// уведомляющего, о том, что надо изменить значение индикатора выполнения.
	multiplicityFactor = 0;
	// Счётчик сообщений на изменение индикатора выполнения,
	// поставленных в очередь на обработку.
	sentMessageCounter = 0;
	// Фактический диапазон изменения индикатора выполнения.
	actualProgressBarRange = 0;
}

// Деструктор.
DivideAndConquer::~DivideAndConquer()
{
	//delete[] pInfoMessage;
}

// Выполняет триангуляцию Делоне по алгоритму "Разделяй и властвуй".
long DivideAndConquer::DivconqDelaunay(HWND hWnd, Mesh* m, Configuration* b, cancellation_token token)
{
	// Проверить, была ли отмена задачи.
	if (token.is_canceled())
	{
		cancel_current_task();
	}
	else
	{
		// Сортируемые вершины.
		//Vertex *sortedArray;
		OTriangle hullleft, hullright;
		int divider;
		// Счётчик для циклов, выполняющих проход по триангулируемым вершинам.
		int i;

		// Выделить массив указателей на вершины для сортировки.
		Vertex* sortedArray = new Vertex[m->InVertices * int(sizeof(Vertex))];

		// Передать количество входных вершин в главное окно для установки индикатора выполнения в исходное состояние.
		actualProgressBarRange = m->InVertices;
		multiplicityFactor = 1;
		if (m->InVertices > RANGE)
		{
			multiplicityFactor = m->InVertices / RANGE;
			actualProgressBarRange /= multiplicityFactor;
		}
		wstring message = L"Создание массива исходных точек для их сортировки";
		wchar_t* pInfoMessage = _wcsdup(message.c_str());
		PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pInfoMessage, 0L);
		PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)actualProgressBarRange);
		sentMessageCounter = 0;

		// Запустить обход.
		TraversalInit(&m->Vertices);
		for (i = 0; i < m->InVertices; i++)
		{
			// Проверить, была ли отмена задачи.
			if (token.is_canceled())
			{
				cancel_current_task();
			}
			else
			{
				sortedArray[i] = VertexTraverse(m);

				// Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
				if (i % multiplicityFactor == 0 && sentMessageCounter < actualProgressBarRange)
				{
					PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
					sentMessageCounter++;
				}
			}
		}

		// Отсортировать вершины:
		// Передать количество входных вершин в главное окно для установки индикатора выполнения в исходное состояние.
		actualProgressBarRange = (int)(m->InVertices / SORTING_FACTOR);
		multiplicityFactor = 1;
		if (m->InVertices > RANGE)
		{
			multiplicityFactor = m->InVertices / RANGE;
			actualProgressBarRange /= multiplicityFactor;
		}
		message = L"Сортировка массива исходных точек";
		pInfoMessage = _wcsdup(message.c_str());
		PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pInfoMessage, 0L);
		PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)actualProgressBarRange);
		sentMessageCounter = 0;
		recursionCounter = 0;
		// Непосредственно выполнить сортировку.
		vertexSort(hWnd, sortedArray, m->InVertices);

		// Отбросить дублирующие вершины, которые действительно могут испортить алгоритм:
		// Передать количество входных вершин в главное окно для установки индикатора выполнения в исходное состояние.
		actualProgressBarRange = m->InVertices;
		multiplicityFactor = 1;
		if (m->InVertices > RANGE)
		{
			multiplicityFactor = m->InVertices / RANGE;
			actualProgressBarRange /= multiplicityFactor;
		}

		message = L"Удаление дубликатов точек";
		pInfoMessage = _wcsdup(message.c_str());
		PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pInfoMessage, 0L);
		PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)actualProgressBarRange/*(LPARAM)RANGE)*/);
		sentMessageCounter = 0;
		i = 0;
		// Выполнить удаление дублирующих вершин.
		for (int j = 1; j < m->InVertices; j++)
		{
			// Проверить, была ли отмена задачи.
			if (token.is_canceled())
			{
				cancel_current_task();
			}
			else
			{
				if ((sortedArray[i][0] == sortedArray[j][0]) && (sortedArray[i][1] == sortedArray[j][1]))
				{
					// Установить тип для вершины.
					SetVertexType(m, sortedArray[j], UNDEADVERTEX);
					m->UnshownVertices++;
				}
				else
				{
					i++;
					sortedArray[i] = sortedArray[j];
				}

                // Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
				if (i % multiplicityFactor == 0 && sentMessageCounter < actualProgressBarRange)
				{
					PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
					sentMessageCounter++;
				}
			}
		}
		i++;

		// По умолчанию использовать чередующиеся вертикальные и горизонтальные
		// рассечения исходного облака точек, которые обычно улучшают скорость,
		// за исключением небольших, коротких и широких наборов вершин.
		if (b->dwyer)
		{
			// Пересортировать массив вершин для размещения чередующихся разрезов:
			divider = i >> 1;
			if (i - divider >= 2)
			{
				// Передать количество входных вершин в главное окно для установки индикатора выполнения в исходное состояние.
				actualProgressBarRange = (int)(divider / ALTERNATING_FACTOR);
				multiplicityFactor = 1;
				if (divider > RANGE)
				{
					multiplicityFactor = divider / RANGE;
					actualProgressBarRange /= multiplicityFactor;
				}

				message = L"Рассечение множества точек по горизонтали и вертикали";
				pInfoMessage = _wcsdup(message.c_str());
				PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pInfoMessage, 0L);
				PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pInfoMessage, 0L);
				PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)actualProgressBarRange);
				sentMessageCounter = 0;
				recursionCounter = 0;
				// Непосредственно выполнить пересортировку для размещения чередующихся разрезов.
				if (divider >= 2)
					alternateAxes(hWnd, sortedArray, divider, 1, token);
				alternateAxes(hWnd, &sortedArray[divider], i - divider, 1, token);
			}
		}

		// Формировать триангуляцию Делоне:
		// Передать количество входных вершин в главное окно для установки индикатора выполнения в исходное состояние.
		actualProgressBarRange = (int)(i / TRIANGULATION_FACTOR);
		multiplicityFactor = 1;
		if (i > RANGE)
		{
			multiplicityFactor = i / RANGE;
			actualProgressBarRange /= multiplicityFactor;
		}
		message = L"Выполнение триангуляции Делоне";
		pInfoMessage = _wcsdup(message.c_str());
		PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pInfoMessage, 0L);
		PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)actualProgressBarRange);
		sentMessageCounter = 0;
		recursionCounter = 0;
		// Непосредственно формировать триангуляцию Делоне.
		divconqRecurse(hWnd, m, b, sortedArray, i, 0, &hullleft, &hullright, token);

		// Освободить память, выделенную для массива указателей.
		delete[] sortedArray;

		// Удалить призрачные (технологические) треугольники:
		/*
		// Передать в главное окно максимум для индикатора выполнения для его установки в исходное состояние.
		if (m->InVertices == MINIMAL_TRIANGULATION)
		{
			actualProgressBarRange = MINIMAL_TRIANGULATION;
			//multiplicityFactor = 1;
		}
		else
		{
			actualProgressBarRange = APPROXIMATE_HULL_SIZE;
			multiplicityFactor = MINIMAL_TRIANGULATION;
		}
		sentMessageCounter = 0;
		PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)actualProgressBarRange);
		//*/
		// Непосредственно удалить технологические треугольники.
		message = L"Удаление \"технологических\" треугольников";
		pInfoMessage = _wcsdup(message.c_str());
		PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pInfoMessage, 0L);
		free(pInfoMessage);

		return removeGhosts(hWnd, m, b, &hullleft, token);
	}
}

// Сортирует массив вершин по координате X, используя координату Y как вспомогательный ключ.
void DivideAndConquer::vertexSort(HWND hWnd, Vertex* sortarray, int arraysize)
{
	// Вспомогательный буфер, используемый при замене
	// левой вершины на правую, а правой - на левую.
	Vertex temp;

	// При сортировке только двух вершин, вершина с большим  значением 
	// сравниваемой координаты становится первой в массиве. (Т.е., это
	// рекурсивный базовый случай.)
	if (arraysize == 2)
	{
		if ((sortarray[0][0] > sortarray[1][0]) || ((sortarray[0][0] == sortarray[1][0]) && (sortarray[0][1] > sortarray[1][1])))
		{
			temp = sortarray[1];
			sortarray[1] = sortarray[0];
			sortarray[0] = temp;
		}
		return;
	}
	// Выбрать случайные координаты двух точек через которые будет проходить
	// условная ось, разбивающая на две части множество точек, содержащихся
	// в сортируемом массиве.
	int pivot = int(Randomnation((unsigned int)arraysize));
	double pivotx = sortarray[pivot][0];
	double pivoty = sortarray[pivot][1];
	// Разбиение сортируемого массива на две части:
	int left = -1;         // отправная точка для поиска координаты X левой вершины,
	int right = arraysize; // отправная точка для поиска координаты X правой вершины.
	while (left < right) {
		// Искать вершину, чья координата X слишком велика для её нахождения в левой части.
		do
			left++;
		while ((left <= right) && ((sortarray[left][0] < pivotx) || ((sortarray[left][0] == pivotx) &&
			(sortarray[left][1] < pivoty))));
		// Искать вершину, чья координата X слишком мала для её нахождения в правой части.
		do
			right--;
		while ((left <= right) && ((sortarray[right][0] > pivotx) || ((sortarray[right][0] == pivotx) &&
			(sortarray[right][1] > pivoty))));
		if (left < right)
		{
			// Поменять местами левую и правую вершины.
			temp = sortarray[left];
			sortarray[left] = sortarray[right];
			sortarray[right] = temp;
		}
	}

    // Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
	if (recursionCounter % multiplicityFactor == 0 && sentMessageCounter < actualProgressBarRange)
	{
		PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
		sentMessageCounter++;
	}

	// Рекурсивно сортировать левое подмножество.
	if (left > 1)
	{
		recursionCounter++;
		vertexSort(hWnd, sortarray, left);
	}
	// Рекурсивно сортировать правое подмножество.
	if (right < arraysize - 2)
	{
		recursionCounter++;
		vertexSort(hWnd, &sortarray[right + 1], arraysize - right - 1);
	}
}

// Сортирует вершины в соответствии с алгоритмом "Разделяй и властвуй", используя
// чередующееся (горизонтальное и вертикальное)  разбиение множества вершин. Если
// axis = 0, то разбиение выполняется по  координате X, иначе если axis = 1, то -
// - по координате Y.
void DivideAndConquer::alternateAxes(HWND hWnd, Vertex* sortarray, int arraysize, int axis, cancellation_token token)
{
	// Проверить, была ли отмена задачи.
	if (token.is_canceled())
	{
		cancel_current_task();
	}
	else
	{
		int divider = arraysize >> 1;

		// Рекурсивный базовый случай: подмножества из 2 или 3 вершин будут обработаны 
		// особенным для себя образом и их всегда следует сортировать по координате X.
		if (arraysize <= 3)
			axis = 0;
		// Выполнить разбиение по горизонтали или по вертикали.
		vertexMedian(sortarray, arraysize, divider, axis);

		// Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
		if (recursionCounter % multiplicityFactor == 0 && sentMessageCounter < actualProgressBarRange)
		{
			PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
			sentMessageCounter++;
		}

		// Рекурсивное разбиение подмножеств с поперечным (перекрёстным) рассечением.
		if (arraysize - divider >= 2)
		{
			if (divider >= 2)
			{
				recursionCounter++;
				alternateAxes(hWnd, sortarray, divider, 1 - axis, token);
			}
			recursionCounter++;
			alternateAxes(hWnd, &sortarray[divider], arraysize - divider, 1 - axis, token);
		}
	}
}

// Перетасовывает массив вершин таким образом, что первые «срединные» вершины
// появляются лексикографически перед остальными вершинами. Делая перетасовку
// использует координату X  вершины в качестве первичного ключа при  axis = 0
// и координату Y - при axis = 1.
void DivideAndConquer::vertexMedian(Vertex* sortarray, int arraysize, int median, int axis)
{
	int left, right;
	int pivot;
	double pivot1, pivot2;
	Vertex temp;

	// Рекурсивный базовый случай, когда нужно перетасовать всего две вершины.
	if (arraysize == 2)
	{
		if ((sortarray[0][axis] > sortarray[1][axis]) ||
			((sortarray[0][axis] == sortarray[1][axis]) &&
			(sortarray[0][1 - axis] > sortarray[1][1 - axis])))
		{
			temp = sortarray[1];
			sortarray[1] = sortarray[0];
			sortarray[0] = temp;
		}
		return;
	}

	// Выбрать случайную условную ось, разбивающую на две части множество точек,
	// содержащихся в перетасовываемом массиве.
	pivot = (int)Randomnation((unsigned int)arraysize);
	pivot1 = sortarray[pivot][axis];
	pivot2 = sortarray[pivot][1 - axis];
	// Разбиение перетасовываемого массива вершин на два подмножества.
	left = -1;
	right = arraysize;

	while (left < right)
	{
		// Искать вершину, чья координата X слишком велика для её нахождения в левом подмножестве.
		do
			left++;
		while ((left <= right) && ((sortarray[left][axis] < pivot1)
			|| ((sortarray[left][axis] == pivot1) && (sortarray[left][1 - axis] < pivot2))));
		// Искать вершину, чья координата X слишком мала для её нахождения в правом подмножестве.
		do
			right--;
		while ((left <= right) && ((sortarray[right][axis] > pivot1)
			|| ((sortarray[right][axis] == pivot1) && (sortarray[right][1 - axis] > pivot2))));

		if (left < right)
		{
			// Поменять местами левую и правую вершины.
			temp = sortarray[left];
			sortarray[left] = sortarray[right];
			sortarray[right] = temp;
		}
	}

	// Удовлетворить истине может только одно из двух приведённых ниже условий:
	if (left > median)
	{
		// Рекурсивно перетасовать левое подмножество.
		vertexMedian(sortarray, left, median, axis);
	}
	if (right < median - 1)
	{
		// Рекурсивно перетасовать правое подмножество.
		vertexMedian(&sortarray[right + 1], arraysize - right - 1, median - right - 1, axis);
	}
}

// Рекурсивно формирует триангуляцию Делоне по алгоритму "Разделяй и властвуй".
void DivideAndConquer::divconqRecurse(HWND hWnd, Mesh * m, Configuration * b, Vertex * sortarray, int vertices, int axis, OTriangle * farleft, OTriangle * farright, cancellation_token token)
{
	// Проверить, была ли отмена задачи.
	if (token.is_canceled())
	{
		cancel_current_task();
	}
	else
	{
		OTriangle midtri, tri1, tri2, tri3;
		OTriangle innerleft, innerright;
		double area;
		int divider;

		// Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
		if (recursionCounter % multiplicityFactor == 0 && sentMessageCounter < actualProgressBarRange)
		{
			PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
			sentMessageCounter++;
		}

		if (vertices == 2)
		{
			// Триангуляция двух вершин,это ребро треугольника. Ребро представлено
			// двумя  граничащими друг с другом треугольниками,  смежными по этому
			// ребру.
			MakeTriangle(m, b, farleft);
			SetOriginTri(*farleft, sortarray[0]);
			SetDestinationTri(*farleft, sortarray[1]);
			// Вершина треугольника намеренно оставлена нулевой.
			MakeTriangle(m, b, farright);
			SetOriginTri(*farright, sortarray[1]);
			SetDestinationTri(*farright, sortarray[0]);
			// Вершина треугольника намеренно оставлена нулевой.
			BondTwoTriangles(*farleft, *farright);
			farleft->Orient = FindSelfPreviouseLeftEdge(farleft->Orient);
			farright->Orient = FindSelfNextLeftEdge(farright->Orient);
			BondTwoTriangles(*farleft, *farright);
			farleft->Orient = FindSelfPreviouseLeftEdge(farleft->Orient);
			farright->Orient = FindSelfNextLeftEdge(farright->Orient);
			BondTwoTriangles(*farleft, *farright);
			// Убедиться, что источником 'farleft' является sortarray [0].
			FindPreviouseLeftEdge(*farright, *farleft);
			return;
		}
		else if (vertices == 3)
		{
			// Триангуляция трех вершин представляет собой либо треугольник
			// (с  тремя  ограничивающими  треугольниками), либо два  ребра 
			// (c четырьмя ограничивающими треугольниками). В любом случае, 
			// четыре треугольника созданы.
			MakeTriangle(m, b, &midtri);
			MakeTriangle(m, b, &tri1);
			MakeTriangle(m, b, &tri2);
			MakeTriangle(m, b, &tri3);
			area = CounterClockwise(m, b, sortarray[0], sortarray[1], sortarray[2]);
			// В случае трёх коллинеарных вершин, триангуляция представляет собой два ребра.
			if (area == 0.0)
			{
				SetOriginTri(midtri, sortarray[0]);
				SetDestinationTri(midtri, sortarray[1]);
				SetOriginTri(tri1, sortarray[1]);
				SetDestinationTri(tri1, sortarray[0]);
				SetOriginTri(tri2, sortarray[2]);
				SetDestinationTri(tri2, sortarray[1]);
				SetOriginTri(tri3, sortarray[1]);
				SetDestinationTri(tri3, sortarray[2]);
				// Все вершины намеренно оставлены нулевыми.
				BondTwoTriangles(midtri, tri1);
				BondTwoTriangles(tri2, tri3);
				midtri.Orient = FindSelfNextLeftEdge(midtri.Orient);
				tri1.Orient = FindSelfPreviouseLeftEdge(tri1.Orient);
				tri2.Orient = FindSelfNextLeftEdge(tri2.Orient);
				tri3.Orient = FindSelfPreviouseLeftEdge(tri3.Orient);
				BondTwoTriangles(midtri, tri3);
				BondTwoTriangles(tri1, tri2);
				midtri.Orient = FindSelfNextLeftEdge(midtri.Orient);
				tri1.Orient = FindSelfPreviouseLeftEdge(tri1.Orient);
				tri2.Orient = FindSelfNextLeftEdge(tri2.Orient);
				tri3.Orient = FindSelfPreviouseLeftEdge(tri3.Orient);
				BondTwoTriangles(midtri, tri1);
				BondTwoTriangles(tri2, tri3);
				// Убедиться, что источником 'farleft 'является sortarray [0].
				CopyOtri(tri1, *farleft);
				// Убедиться, что назначением 'farright 'является sortarray [2].
				CopyOtri(tri2, *farright);
			}
			else
			{
				// Иначе, если три вершины не коллинеарны, то в результате триангуляции
				// получается один ориентированный треугольник.
				SetOriginTri(midtri, sortarray[0]);
				SetDestinationTri(tri1, sortarray[0]);
				SetOriginTri(tri3, sortarray[0]);
				// Вершины ориентированных треугольников tri1, tri2 и tri3 оставлены пустыми.
				if (area > 0.0)
				{
					// Ориентация (следование друг за другом) вершин - против часовой стрелки.
					SetDestinationTri(midtri, sortarray[1]);
					SetOriginTri(tri1, sortarray[1]);
					SetDestinationTri(tri2, sortarray[1]);
					SetApexTri(midtri, sortarray[2]);
					SetOriginTri(tri2, sortarray[2]);
					SetDestinationTri(tri3, sortarray[2]);
				}
				else
				{
					// А здесь ориентация вершин - по часовой стрелке.
					SetDestinationTri(midtri, sortarray[2]);
					SetOriginTri(tri1, sortarray[2]);
					SetDestinationTri(tri2, sortarray[2]);
					SetApexTri(midtri, sortarray[1]);
					SetOriginTri(tri2, sortarray[1]);
					SetDestinationTri(tri3, sortarray[1]);
				}
				// Топология не зависит от порядка следования вершин.
				BondTwoTriangles(midtri, tri1);
				midtri.Orient = FindSelfNextLeftEdge(midtri.Orient);
				BondTwoTriangles(midtri, tri2);
				midtri.Orient = FindSelfNextLeftEdge(midtri.Orient);
				BondTwoTriangles(midtri, tri3);
				tri1.Orient = FindSelfPreviouseLeftEdge(tri1.Orient);
				tri2.Orient = FindSelfNextLeftEdge(tri2.Orient);
				BondTwoTriangles(tri1, tri2);
				tri1.Orient = FindSelfPreviouseLeftEdge(tri1.Orient);
				tri3.Orient = FindSelfPreviouseLeftEdge(tri3.Orient);
				BondTwoTriangles(tri1, tri3);
				tri2.Orient = FindSelfNextLeftEdge(tri2.Orient);
				tri3.Orient = FindSelfPreviouseLeftEdge(tri3.Orient);
				BondTwoTriangles(tri2, tri3);
				// Убедиться, что источником 'farleft 'является sortarray [0].
				CopyOtri(tri1, *farleft);
				// Убедиться, что назначением 'farright 'является sortarray [2].
				if (area > 0.0)
				{
					CopyOtri(tri2, *farright);
				}
				else
				{
					FindNextLeftEdge(*farleft, *farright);
				}
			}

			return;
		}
		else
		{
			// Разбить облако точек пополам
			divider = vertices >> 1;
			// Рекурсивно триангулировать каждую половину.
			recursionCounter++;
			divconqRecurse(hWnd, m, b, sortarray, divider, 1 - axis, farleft, &innerleft, token);
			recursionCounter++;
			divconqRecurse(hWnd, m, b, &sortarray[divider], vertices - divider, 1 - axis, &innerright, farright, token);
			// Слияние двух триангуляций в одну.
			mergeHulls(m, b, farleft, &innerleft, &innerright, farright, axis);
		}
	}
}

// Удаляет "призрачные" (технологические) треугольники. И возвращает количество рёбер,
// ограничивающих выпуклую оболочку (триангуляционную сетку) по её краям.
long DivideAndConquer::removeGhosts(HWND hWnd, Mesh * m, Configuration * b, OTriangle * startghost, cancellation_token token)
{
	// Проверить, была ли отмена задачи.
	if (token.is_canceled())
	{
		cancel_current_task();
	}
	else
	{
		OTriangle searchedge;
		OTriangle dissolveedge;
		// Удаляемый технологический треугольник.
		OTriangle deadtriangle;
		Vertex markorg;
		// Количество рёбер, ограничивающих выпуклую оболочку по её краям.
		long HullSize;

		// Найти ребро на выпуклой поверхности триангуляции, которое может быть взято за стартовую точку.
		FindPreviouseLeftEdge(*startghost, searchedge);
		FindSimilarTri(searchedge);
		m->DummyTri[0] = OtriToPointer(searchedge);
		// Удалить ограничивающий бокс и сосчитать рёбра выпуклой поверхности триангуляции.
		CopyOtri(*startghost, dissolveedge);
		HullSize = 0;
		do
		{
			// Проверить, была ли отмена задачи.
			if (token.is_canceled())
			{
				cancel_current_task();
			}
			else
			{
				//HullSize++;

				//PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)1);

				FindNextLeftEdge(dissolveedge, deadtriangle);
				dissolveedge.Orient = FindSelfPreviouseLeftEdge(dissolveedge.Orient);
				FindSimilarTri(dissolveedge);
				// Установить граничные маркеры всех вершин на  выпуклой поверхности триангуляции.
				// Не упустить случай, когда все входные вершины коллинеарны.
				if (dissolveedge.Tri != m->DummyTri)
				{
					markorg = GetOrignVertex(dissolveedge);
					if (GetVertexMark(markorg, m->VertexMarkIndex) == 0)
						SetVertexMark(markorg, 1, m->VertexMarkIndex);
				}
				// Удалить ограничивающий треугольник из треугольника выпуклой поверхности.
				Dissolve(dissolveedge, m->DummyTri);
				// Найти следующий ограничивающий треугольник.
				FindAbutingTriOnSameEdge(deadtriangle, dissolveedge);
				// Удалить ограничивающий треугольник.
				TriangleDealloc(m, deadtriangle.Tri);

				/*
				// Изменить значение индикатора выполнения.
				if (actualProgressBarRange == MINIMAL_TRIANGULATION)
				{
					if (sentMessageCounter <= actualProgressBarRange)
					{
						PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
						sentMessageCounter++;
					}
				}
				else
				{
					if (HullSize % multiplicityFactor == 0 && sentMessageCounter <= actualProgressBarRange)
					{
						if (sentMessageCounter == actualProgressBarRange)
						{
							sentMessageCounter = 0;
							PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)actualProgressBarRange);
							//
							//continue;
							//
						}
						PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
						sentMessageCounter++;
					}
				}
				//*/

				// Скорректировать счётчик ограничивающих рёбер.
				HullSize++;
			}
		} while (!AreTwoOrientedTrianglesEqual(dissolveedge, *startghost));

		return HullSize;
	}
}

// Объединяет две смежные триангуляции Делоне в одну.
void DivideAndConquer::mergeHulls(Mesh * m, Configuration * b, OTriangle * farleft, OTriangle * innerleft,
	OTriangle * innerright, OTriangle * farright, int axis)
{
	OTriangle leftcand, rightcand;
	OTriangle baseedge;
	OTriangle nextedge;
	OTriangle sidecasing, topcasing, outercasing;
	OTriangle checkedge;
	Vertex innerleftdest;
	Vertex innerrightorg;
	Vertex innerleftapex, innerrightapex;
	Vertex farleftpt, farrightpt;
	Vertex farleftapex, farrightapex;
	Vertex lowerleft, lowerright;
	Vertex upperleft, upperright;
	Vertex nextapex;
	Vertex checkvertex;
	int changemade;
	int badedge;
	int leftfinished, rightfinished;

	innerleftdest = GetDestinationVertex(*innerleft);
	innerleftapex = GetApex(*innerleft);
	innerrightorg = GetOrignVertex(*innerright);
	innerrightapex = GetApex(*innerright);
	// Специальная обработка (трактовка) для горизонтальных рассечений облака точек.
	if (b->dwyer && (axis == 1))
	{
		farleftpt = GetOrignVertex(*farleft);
		farleftapex = GetApex(*farleft);
		farrightpt = GetDestinationVertex(*farright);
		farrightapex = GetApex(*farright);
		// Указатели на экстремальные вершины смещены так,  чтобы указывать
		// на самую верхнюю и самую нижнюю вершины каждой поверхности, а не
		// на самые левые и самые правые вершины.
		while (farleftapex[1] < farleftpt[1])
		{
			farleft->Orient = FindSelfNextLeftEdge(farleft->Orient);
			FindSimilarTri(*farleft);
			farleftpt = farleftapex;
			farleftapex = GetApex(*farleft);
		}
		FindAbutingTriOnSameEdge(*innerleft, checkedge);
		checkvertex = GetApex(checkedge);
		while (checkvertex[1] > innerleftdest[1])
		{
			FindNextLeftEdge(checkedge, *innerleft);
			innerleftapex = innerleftdest;
			innerleftdest = checkvertex;
			FindAbutingTriOnSameEdge(*innerleft, checkedge);
			checkvertex = GetApex(checkedge);
		}
		while (innerrightapex[1] < innerrightorg[1])
		{
			innerright->Orient = FindSelfNextLeftEdge(innerright->Orient);
			FindSimilarTri(*innerright);
			innerrightorg = innerrightapex;
			innerrightapex = GetApex(*innerright);
		}
		FindAbutingTriOnSameEdge(*farright, checkedge);
		checkvertex = GetApex(checkedge);
		while (checkvertex[1] > farrightpt[1])
		{
			FindNextLeftEdge(checkedge, *farright);
			farrightapex = farrightpt;
			farrightpt = checkvertex;
			FindAbutingTriOnSameEdge(*farright, checkedge);
			checkvertex = GetApex(checkedge);
		}
	}
	// Найти касательную к обоим выпуклым поверхностям.
	do
	{
		changemade = 0;
		// Сделать innerleftdest самой нижней вершиной левой выпуклой поверхности.
		if (CounterClockwise(m, b, innerleftdest, innerleftapex, innerrightorg) > 0.0)
		{
			innerleft->Orient = FindSelfPreviouseLeftEdge(innerleft->Orient);
			FindSimilarTri(*innerleft);
			innerleftdest = innerleftapex;
			innerleftapex = GetApex(*innerleft);
			changemade = 1;
		}
		// Сделать innerrightorg самой нижней вершиной правой выпуклой поверхности.
		if (CounterClockwise(m, b, innerrightapex, innerrightorg, innerleftdest) > 0.0)
		{
			innerright->Orient = FindSelfNextLeftEdge(innerright->Orient);
			FindSimilarTri(*innerright);
			innerrightorg = innerrightapex;
			innerrightapex = GetApex(*innerright);
			changemade = 1;
		}
	} while (changemade);

	// Найти двух кандидатов на роль следующего «зуба шестерни».
	FindAbutingTriOnSameEdge(*innerleft, leftcand);
	FindAbutingTriOnSameEdge(*innerright, rightcand);
	// Создать нижний новый ограничительный треугольник.
	MakeTriangle(m, b, &baseedge);
	// Подсоединить его к ограничительным рамкам левой и правой триангуляций.
	BondTwoTriangles(baseedge, *innerleft);
	baseedge.Orient = FindSelfNextLeftEdge(baseedge.Orient);
	BondTwoTriangles(baseedge, *innerright);
	baseedge.Orient = FindSelfNextLeftEdge(baseedge.Orient);
	SetOriginTri(baseedge, innerrightorg);
	SetDestinationTri(baseedge, innerleftdest);
	// Вершина намеренно оставлена нулевой

	// Исправить крайние треугольники, если это необходимо.
	farleftpt = GetOrignVertex(*farleft);
	if (innerleftdest == farleftpt)
	{
		FindNextLeftEdge(baseedge, *farleft);
	}
	farrightpt = GetDestinationVertex(*farright);
	if (innerrightorg == farrightpt)
	{
		FindPreviouseLeftEdge(baseedge, *farright);
	}
	// Вершины текущего связывающего ребра.
	lowerleft = innerleftdest;
	lowerright = innerrightorg;
	// Вершины-кандидаты для связывания.
	upperleft = GetApex(leftcand);
	upperright = GetApex(rightcand);
	// Подниматься по "щели" между двумя триангуляциями, связывая их вместе друг с другом.
	while (1)
	{
		// Достигли  ли  мы  верха? (Это не совсем правильный вопрос,  потому что,
		// даже  если  левая триангуляция  может сечас показаться  законченной, то
		// движение вверх по правой триангуляции может открыть новую вершину левой
		// триангуляции. И наоборот.)
		leftfinished = CounterClockwise(m, b, upperleft, lowerleft, lowerright) <= 0.0;
		rightfinished = CounterClockwise(m, b, upperright, lowerleft, lowerright) <= 0.0;
		if (leftfinished && rightfinished)
		{
			// Создать новый верхний ограничительный треугольник.
			MakeTriangle(m, b, &nextedge);
			SetOriginTri(nextedge, lowerleft);
			SetDestinationTri(nextedge, lowerright);
			// Апекс намеренно оставлен нулевым указателем. Соединить его
			//  с ограничивающими прямоугольниками двух триангуляций.
			BondTwoTriangles(nextedge, baseedge);
			nextedge.Orient = FindSelfNextLeftEdge(nextedge.Orient);
			BondTwoTriangles(nextedge, rightcand);
			nextedge.Orient = FindSelfNextLeftEdge(nextedge.Orient);
			BondTwoTriangles(nextedge, leftcand);
			// Специальная обработка для горизонтального рассечения облака точек.
			if (b->dwyer && (axis == 1))
			{
				farleftpt = GetOrignVertex(*farleft);
				farleftapex = GetApex(*farleft);
				farrightpt = GetDestinationVertex(*farright);
				farrightapex = GetApex(*farright);
				FindAbutingTriOnSameEdge(*farleft, checkedge);
				checkvertex = GetApex(checkedge);
				// Указатели на экстремальные вершины восстанавливаются на крайние левые
				// и крайние правые вершины (а не на крайние верхние и крайние нижние).
				while (checkvertex[0] < farleftpt[0])
				{
					FindPreviouseLeftEdge(checkedge, *farleft);
					farleftapex = farleftpt;
					farleftpt = checkvertex;
					FindAbutingTriOnSameEdge(*farleft, checkedge);
					checkvertex = GetApex(checkedge);
				}
				while (farrightapex[0] > farrightpt[0])
				{
					farright->Orient = FindSelfPreviouseLeftEdge(farright->Orient);
					FindSimilarTri(*farright);
					farrightpt = farrightapex;
					farrightapex = GetApex(*farright);
				}
			}
			return;
		}
		// Рассмотреть исключение ребер из левой триангуляции.
		if (!leftfinished)
		{
			// Какая вершина будет выставлена, если ребро будет удалено?
			FindPreviouseLeftEdge(leftcand, nextedge);
			FindSimilarTri(nextedge);
			nextapex = GetApex(nextedge);
			// Если nextapex представляет собой нулевой указатель, то никакая вершина
			// вершина не будет выставлена. Т.к., в этом случае, триангуляция была бы
			// "съедена" насквозь.
			if (nextapex != nullptr)
			{
				// Проверить, удовлетворяет ли вершина условию Делоне.
				badedge = InCircle(m, b, lowerleft, lowerright, upperleft, nextapex) > 0.0;
				while (badedge)
				{
					// Устраните ребро флипом ребра. В результате у левой триангуляции
					// будет еще один граничный треугольник.
					nextedge.Orient = FindSelfNextLeftEdge(nextedge.Orient);
					FindAbutingTriOnSameEdge(nextedge, topcasing);
					nextedge.Orient = FindSelfNextLeftEdge(nextedge.Orient);
					FindAbutingTriOnSameEdge(nextedge, sidecasing);
					BondTwoTriangles(nextedge, topcasing);
					BondTwoTriangles(leftcand, sidecasing);
					leftcand.Orient = FindSelfNextLeftEdge(leftcand.Orient);
					FindAbutingTriOnSameEdge(leftcand, outercasing);
					nextedge.Orient = FindSelfPreviouseLeftEdge(nextedge.Orient);
					BondTwoTriangles(nextedge, outercasing);
					// Исправьте вершины, чтобы отобразить флип ребра.
					SetOriginTri(leftcand, lowerleft);
					SetDestinationTri(leftcand, nullptr);
					SetApexTri(leftcand, nextapex);
					SetOriginTri(nextedge, nullptr);
					SetDestinationTri(nextedge, upperleft);
					SetApexTri(nextedge, nextapex);
					// Рассмотреть недавно открытую вершину.
					upperleft = nextapex;
					// Какая вершина будет выставлена, если другая вершина будет удалена?
					CopyOtri(sidecasing, nextedge);
					nextapex = GetApex(nextedge);
					if (nextapex != nullptr)
					{
						// Проверить, удовлетворяет ли ребро условию Делоне.
						badedge = InCircle(m, b, lowerleft, lowerright, upperleft, nextapex) > 0.0;
					}
					else
					{
						// Избегать "проедания" триангуляции.
						badedge = 0;
					}
				}
			}
		}
		// Рассмотреть исключение ребер из правой триангуляции.
		if (!rightfinished)
		{
			// Какая вершина будет выставлена, если ребро будет удалено?
			FindNextLeftEdge(rightcand, nextedge);
			FindSimilarTri(nextedge);
			nextapex = GetApex(nextedge);
			// Если nextapex установлен в ноль, то вершина не будет выставлена;
			// триангуляция была бы "съедена" насквозь.
			if (nextapex != nullptr)
			{
				// Проверить, соответствует ли ребро условию Делоне.
				badedge = InCircle(m, b, lowerleft, lowerright, upperright, nextapex) > 0.0;
				while (badedge)
				{
					// Исключить ребро флипом ребра. В результате у правой триангуляции
					// будет еще один граничный треугольник.
					nextedge.Orient = FindSelfPreviouseLeftEdge(nextedge.Orient);
					FindAbutingTriOnSameEdge(nextedge, topcasing);
					nextedge.Orient = FindSelfPreviouseLeftEdge(nextedge.Orient);
					FindAbutingTriOnSameEdge(nextedge, sidecasing);
					BondTwoTriangles(nextedge, topcasing);
					BondTwoTriangles(rightcand, sidecasing);
					rightcand.Orient = FindSelfPreviouseLeftEdge(rightcand.Orient);
					FindAbutingTriOnSameEdge(rightcand, outercasing);
					nextedge.Orient = FindSelfNextLeftEdge(nextedge.Orient);
					BondTwoTriangles(nextedge, outercasing);
					// Исправить вершины, чтобы отобразить флип ребра.
					SetOriginTri(rightcand, nullptr);
					SetDestinationTri(rightcand, lowerright);
					SetApexTri(rightcand, nextapex);
					SetOriginTri(nextedge, upperright);
					SetDestinationTri(nextedge, nullptr);
					SetApexTri(nextedge, nextapex);
					// Рассмотреть недавно открытую вершину.
					upperright = nextapex;
					// Какая вершина будет выставлена, если другое ребро будет удалено?
					CopyOtri(sidecasing, nextedge);
					nextapex = GetApex(nextedge);
					if (nextapex != nullptr)
					{
						// Проверить, соответствует ли ребро условию Делоне.
						badedge = InCircle(m, b, lowerleft, lowerright, upperright, nextapex) > 0.0;
					}
					else
					{
						// Избегать "проедания" прямо через триангуляцию.
						badedge = 0;
					}
				}
			}
		}
		if (leftfinished || (!rightfinished && (InCircle(m, b, upperleft, lowerleft, lowerright, upperright) > 0.0)))
		{
			// Связывать триангуляции друг с другом, добавляя ребро от "нижнего левого" (переменная lowerleft)
			// до "правого верхнего" (переменная upperright).
			BondTwoTriangles(baseedge, rightcand);
			FindPreviouseLeftEdge(rightcand, baseedge);
			SetDestinationTri(baseedge, lowerleft);
			lowerright = upperright;
			FindAbutingTriOnSameEdge(baseedge, rightcand);
			upperright = GetApex(rightcand);
		}
		else
		{
			// Связать триангуляции друг с другом, добавляя ребро от "верхнего левого" (переменная upperleft)
			// до "нижнего правого" (переменная lowerright).
			BondTwoTriangles(baseedge, leftcand);
			FindNextLeftEdge(leftcand, baseedge);
			SetOriginTri(baseedge, lowerright);
			lowerleft = upperleft;
			FindAbutingTriOnSameEdge(baseedge, leftcand);
			upperleft = GetApex(leftcand);
		}
	}
}

