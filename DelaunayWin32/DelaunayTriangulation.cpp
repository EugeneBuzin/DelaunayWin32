#include "stdafx.h"
#include "DelaunayTriangulation.h"

DelaunayTriangulation::DelaunayTriangulation()
{
}

DelaunayTriangulation::~DelaunayTriangulation()
{
}

// Запускает триангуляцию Делоне.
long DelaunayTriangulation::StartTriangulation(HWND hWnd, Mesh* m, Configuration* b, cancellation_token token)
{
	DivideAndConquer divideAndConquerTriangulation;
	long hulledges = divideAndConquerTriangulation.DivconqDelaunay(hWnd, m, b, token);

	// Если входные вершины коллинеарны (находятся на одной прямой),
	// то  построить на них треугольники невозможно.  Следовательно,
	// вернуть ноль.
	// Иначе, вернуть количество рёбер на выпуклой поверхности.
	if (m->Triangles.CurrentlyAllocatedItems == 0)
		return 0l;
	else
		return hulledges;
}

