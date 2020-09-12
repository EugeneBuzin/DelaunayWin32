#pragma once

#include "DelaunayHelper.h"

using namespace DelaunayHelper;
using namespace UsualHelper;

// Выполняет деинициализацию приложения.
class DelaunayDeinitializition
{
public:
	DelaunayDeinitializition();
	~DelaunayDeinitializition();
	// Выполняет деинициализацию приложения.
	void TriDeinit(struct Mesh* m, struct Configuration* b);
private:
	// Освобождает всю память, занятую пулом, и отдаёт её операционной системе.
	void poolDeinit(struct MemoryPool* pool);
};

