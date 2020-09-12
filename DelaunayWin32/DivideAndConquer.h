#pragma once

#include "DelaunayHelper.h"

using namespace concurrency;
using namespace DelaunayHelper;
using namespace DelaunayHelper::Triangles;
using namespace UsualHelper;

// Реализует алгоритм триангуляции Делоне "Разделяй и властвуй".
class DivideAndConquer
{
public:

	// Конструктор.
	DivideAndConquer();
	// Деструктор.
	~DivideAndConquer();
	// Выполняет триангуляцию Делоне по алгоритму "Разделяй и властвуй".
	long DivconqDelaunay(HWND hWnd, Mesh* m, Configuration* b, cancellation_token token);
private:
	// Сортирует массив вершин по координате X, используя координату Y как вторичный ключ.
	void vertexSort(HWND hWnd, Vertex* sortarray, int arraysize);
	// Сортирует вершины в соответствии с алгоритмом "Разделяй и властвуй", используя
	// чередующееся (горизонтальное и вертикальное)  разбиение множества вершин. Если
	// axis = 0, то разбиение выполняется по  координате X, иначе если axis = 1, то -
	// - по координате Y. 
	void alternateAxes(HWND hWnd, Vertex* sortarray, int arraysize, int axis, cancellation_token token);
	// Перетасовывает массив вершин таким образом, что первые «срединные» вершины
	// появляются лексикографически перед остальными вершинами. Делая перетасовку
	// использует координату X  вершины в качестве первичного ключа при  axis = 0
	// и координату Y - при axis = 1.
	void vertexMedian(Vertex* sortarray, int arraysize, int median, int axis);
	// Рекурсивно формирует триангуляцию Делоне по алгоритму "Разделяй и властвуй".
	void divconqRecurse(HWND hWnd, Mesh *m, Configuration *b, Vertex *sortarray, int vertices, int axis, OTriangle *farleft, OTriangle *farright, cancellation_token token);
	// Удаляет призрачные (технологические) треугольники,
	// которые не должны входить в итоговую триангуляцию.
	long removeGhosts(HWND hWnd, Mesh *m, Configuration *b, OTriangle *startghost, cancellation_token token);
	// Объединяет две смежные триангуляции Делоне в одну.
	void mergeHulls(Mesh *m, Configuration *b, OTriangle *farleft, OTriangle *innerleft, OTriangle *innerright, OTriangle *farright, int axis);

	// Указатель на текст информационного сообщения.
	///*const*/ wchar_t* pInfoMessage;
	// Поправочный коэффициент для определения приблизительного количества рекурсий
    // для сортировки исходных вершин.
	const double SORTING_FACTOR;
	// Поправочный коэффициент для определения приблизительного количества рекурсий
    // для разбиения множества вершин по горизонтали и вертикали.
	const double ALTERNATING_FACTOR;
	// Поправочный коэффициент для определения приблизительного количества рекурсий
	// для выполнения триангуляции Делоне по алгоритму "Разделяй и властвуй".
	const double TRIANGULATION_FACTOR;
	// Диапазон значений, в котором изменяется индикатор выполнения (progressbar).
	const int RANGE;
	// Очень приблизительное количество рёбер, ограничивающих выпуклую оболочку.
	// Т.е. рёбер, "очерчивающих" внешнюю форму триангуляционной сетки.
	const int APPROXIMATE_HULL_SIZE;
	// Минимальное количество точек, над которыми можно выполнять триангуляцию.
	//const int MINIMAL_TRIANGULATION;
	// Счётчик рекурсий.
	int recursionCounter;
	// Коэффициент кратности. Используется для разрешения отправки сообщения,
	// уведомляющего, о том, что надо изменить значение индикатора выполнения.
	int multiplicityFactor;
	// Счётчик сообщений на изменение индикатора выполнения,
	// поставленных в очередь на обработку.
	int sentMessageCounter;
	// Фактический диапазон изменения индикатора выполнения.
	int actualProgressBarRange;

};



