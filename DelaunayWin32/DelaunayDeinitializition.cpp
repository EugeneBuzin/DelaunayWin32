#include "stdafx.h"
#include "DelaunayDeinitializition.h"


DelaunayDeinitializition::DelaunayDeinitializition()
{
}


DelaunayDeinitializition::~DelaunayDeinitializition()
{
}

// Выполняет деинициализацию приложения.
void DelaunayDeinitializition::TriDeinit(Mesh* m, Configuration* b)
{
	poolDeinit(&m->Triangles);
	TriFree((void*)m->DummyTriBase);
	poolDeinit(&m->Vertices);
}

// Освобождает всю память, занятую пулом, и отдаёт её операционной системе.
void DelaunayDeinitializition::poolDeinit(MemoryPool * pool)
{
	if (!IsBadReadPointer(pool->FirstBlock))
	{
		while (pool->FirstBlock != nullptr)
		{
			pool->CurrentAllocationBlock = (void**) *(pool->FirstBlock);
			TriFree((void*)pool->FirstBlock);
			pool->FirstBlock = pool->CurrentAllocationBlock;
		}
	}
}
