#pragma once

#include "DivideAndConquer.h"

using namespace concurrency;

// Выполняет триангуляцию Делоне.
class DelaunayTriangulation
{
public:
	DelaunayTriangulation();
	~DelaunayTriangulation();

	// Запускает триангуляцию Делоне.
	long StartTriangulation(HWND hWnd, Mesh* m, Configuration* b, cancellation_token token);
};

