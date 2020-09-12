#include "stdafx.h"
#include <Windows.h>
#include "DelaunayHelper.h"

// Минимальное значение координаты X для исходных точек, по которым строится триангуляция.
double xMin(std::numeric_limits<double>::quiet_NaN());
// Максимальное значение координаты X для исходных точек, по которым строится триангуляция.
double xMax(std::numeric_limits<double>::quiet_NaN());
// Максимальное значение координаты Y для исходных точек, по которым строится триангуляция. 
double yMin(std::numeric_limits<double>::quiet_NaN());
// Максимальное значение координаты Y для исходных точек, по которым строится триангуляция.
double yMax(std::numeric_limits<double>::quiet_NaN());

// Переменные,  используемые для точной арифметики.
// Представляют границы погрешностей  для проверок
// ориентации рёбер и попадания точки в окружность.
double ccwerrboundA;
double ccwerrboundB;
double ccwerrboundC;
double resulterrbound;
double iccerrboundB;
double iccerrboundC;
double iccerrboundA;
double o3derrboundA;
double o3derrboundB;
double o3derrboundC;

using namespace DelaunayHelper;
using namespace DelaunayHelper::AdaptiveArith;
using namespace DelaunayHelper::Triangles;
using namespace UsualHelper;

// Выполняет начальную инициализацию переменных, используемых в вычислениях.
void DelaunayHelper::DelaunayHelperInit()
{
	int every_other = 1;
	double half = 0.5;
	double check = 1.0, lastcheck;
	// Значение наибольшей степени двойки, такое что в арифметике
	// с плавающей точкой 1.0 + epsilon = 1.0.  Предназначено для
	// ограничения относительной  ошибки округления. Используется
	// для анализа ошибки при вычислениях с плавающей точкой.
	// (Помещён сюда, потому что в существующем варианте программы
	//  больше ни где не используется.)
	double epsilon = 1.0;

	splitter = 1.0;

	// Делить epsilon на два до тех пор, пока его значение не станет настолько
	// маленьким, что его добавление к единице, не будет вызывать округления.
	// (Также проверить, равна ли эта сумма предыдущей сумме для машин, которые
	// округляют до какого-то предела вместо использования точного округления.
	// Обратить внимание на то, чтобы эти процедуры будут работать на подобных
	// машинах.
	do
	{
		lastcheck = check;
		epsilon *= half;
		if (every_other)
			splitter *= 2.0;
		every_other = !every_other;
		check = 1.0 + epsilon;
	} while ((check != 1.0) && (check != lastcheck));
	splitter += 1.0;

	// Границы погрешностей для проверок ориентации рёбер и попадания точки в окружность.
	resulterrbound = (3.0 + 8.0 * epsilon) * epsilon;
	ccwerrboundA = (3.0 + 16.0 * epsilon) * epsilon;
	ccwerrboundB = (2.0 + 12.0 * epsilon) * epsilon;
	ccwerrboundC = (9.0 + 64.0 * epsilon) * epsilon * epsilon;
	iccerrboundA = (10.0 + 96.0 * epsilon) * epsilon;
	iccerrboundB = (4.0 + 48.0 * epsilon) * epsilon;
	iccerrboundC = (44.0 + 576.0 * epsilon) * epsilon * epsilon;
	o3derrboundA = (7.0 + 56.0 * epsilon) * epsilon;
	o3derrboundB = (3.0 + 28.0 * epsilon) * epsilon;
	o3derrboundC = (26.0 + 288.0 * epsilon) * epsilon * epsilon;
}

// Запускает обход по блокам.
void DelaunayHelper::TraversalInit(MemoryPool *pool)
{
	unsigned long long alignptr;

	// Начать обход в первом блоке.
	pool->CurrentTriangulatedBlock = pool->FirstBlock;
	// Найти первый элемент в блоке. Увеличить на размер его указателя.
	alignptr = (unsigned long long)(pool->CurrentTriangulatedBlock + 1);
	// Выровнять элемент.
	pool->NextItemForTriangulation = (void *)(alignptr + (unsigned long long)pool->AlignBytes - (alignptr % (unsigned long long)pool->AlignBytes));
	// Установить количество элементов, оставшихся в текущем блоке.
	pool->PathItemsLeft = pool->ItemsFirstBlock;
}

// Ищет следующий элемент в списке.
void* DelaunayHelper::Traverse(MemoryPool* pool)
{
	void* newitem;
	unsigned long long alignptr;

	// Остановиться, исчерпав весь список предметов.
	if (pool->NextItemForTriangulation == pool->NextFreeMemorySlice)
		return nullptr;

	// Проверить, остались ли в текущем блоке неисследованные элементы.
	if (pool->PathItemsLeft == 0)
	{
		// Найти следующий блок.
		pool->CurrentTriangulatedBlock = (void**) *(pool->CurrentTriangulatedBlock);
		// Найти первый элемент в блоке. Инкрементировать на размер (int*).
		alignptr = (unsigned long long)(pool->CurrentTriangulatedBlock + 1);
		// Выровнять по элементу на границе, определённой в 'AlignBytes'
		pool->NextItemForTriangulation = (void*)(alignptr + (unsigned long long)pool->AlignBytes - (alignptr % (unsigned long long)pool->AlignBytes));
		// Установить количество элементов, оставшихся в текущем блоке.
		pool->PathItemsLeft = pool->ItemsPerBlock;
	}

	newitem = pool->NextItemForTriangulation;
	// Найти следующий элемент в блоке.
	pool->NextItemForTriangulation = (void*)((char*)pool->NextItemForTriangulation + pool->ItemBytes);
	pool->PathItemsLeft--;
	return newitem;
}

// Проходит через вершины, пропуская, при этом, "мертвые" вершины.
Vertex DelaunayHelper::VertexTraverse(Mesh* m)
{
	Vertex newVertex;

	do
	{
		newVertex = (Vertex)Traverse(&m->Vertices);
		if (newVertex == nullptr)
			return nullptr;
	} 
	while (GetVertexType(newVertex, m->VertexMarkIndex) == DEADVERTEX); // Пропускать "мёртвые" вершины.

	return newVertex;
}

// Проходит через треугольники, пропуская, при этом, "мёртвые" треугольники".
Triangle* DelaunayHelper::TriangleTraverse(MemoryPool* pool)
{
	Triangle* newTriangle;

	do
	{
		newTriangle = (Triangle *)Traverse(pool);
		if (newTriangle == nullptr)
		{
			return nullptr;
		}
	} 
	while (IsTriangleKilled(newTriangle)); // Пропускать "мёртвые" треугольники (которые были вспомогательными
	                                       // при построении триангуляционной сетки).
	return newTriangle;
}

// Выделяет память под элемент.
void* DelaunayHelper::PoolAlloc(MemoryPool* pool)
{
	// Новый элемент.
	void* newitem;

	// Проверить связанный список "мёртвых" элементов. Если список не пуст, то для выделения
	// памяти использовать память, занимаемую элементом из этого списка, а не выделять новую
	// область памяти.
	if (pool->DeadElementsStack != nullptr)
	{
		newitem = pool->DeadElementsStack;
		pool->DeadElementsStack = *(void**)pool->DeadElementsStack;
	}
	else
	{
		// Проверить, есть ли в текущем блоке свободные оставшиеся элементы.
		if (pool->UnallocatedItems == 0)
		{
			// Проверить, должен ли быть выделен ещё один блок.
			if (*(pool->CurrentAllocationBlock) == nullptr)
			{
				// Выделить память под новый блок элементов, на которые указывает предыдущий блок.
				void** newblock = new void*[pool->ItemsPerBlock * pool->ItemBytes + (int) sizeof(void*) + pool->AlignBytes];
				*(pool->CurrentAllocationBlock) = (void*)newblock;
				// Обнулить указатель на следующий блок.
				*newblock = nullptr;
			}

			// Перейти к новому блоку.
			pool->CurrentAllocationBlock = (void**) *(pool->CurrentAllocationBlock);
			// Найти первый элемент в блоке. Инкрементировать на размер указателя на int.
			unsigned long long alignptr = (unsigned long long)(pool->CurrentAllocationBlock + 1);
			// Выровнять элемент по границе определённой в `AlignBytes'.
			pool->NextFreeMemorySlice = (void*)(alignptr + (unsigned long long)pool->AlignBytes -
				(alignptr % (unsigned long long)pool->AlignBytes));
			// В этом блоке осталось много нераспределённых элементов.
			pool->UnallocatedItems = pool->ItemsPerBlock;
		}

		// Выделить новый элемент.
		newitem = pool->NextFreeMemorySlice;
		// Переместить указатель «NextFreeMemorySlice» к следующему свободному элементу в блоке.
		pool->NextFreeMemorySlice = (void*)((char *)pool->NextFreeMemorySlice + pool->ItemBytes);
		pool->UnallocatedItems--;
		pool->MaxItems++;
	}
	pool->CurrentlyAllocatedItems++;

	// Вернуть элемент, под который была выделена память.
	return newitem;
}

// Освобождает память, выделенную под элемент. Освобождённое адресное пространство
// сохраняется в очереди для его повторного использования в дальнейшем.
void DelaunayHelper::PoolDealloc(MemoryPool* pool, void* dyingitem)
{
	// Поместить в стек только что уничтоженный элемент.
	*((void**)dyingitem) = pool->DeadElementsStack;
	pool->DeadElementsStack = dyingitem;
	// Декриментировать счётчик используемых элементов
	pool->CurrentlyAllocatedItems--;
}

// Создаёт новый треугольник с нулевой ориентацией.
void DelaunayHelper::MakeTriangle(Mesh* m, Configuration* b, OTriangle* newotri)
{
	int i;

	newotri->Tri = (Triangle *)PoolAlloc(&m->Triangles);
	// Инициализировать три смежных треугольника как "внешнее пространство".
	newotri->Tri[0] = (Triangle)m->DummyTri;
	newotri->Tri[1] = (Triangle)m->DummyTri;
	newotri->Tri[2] = (Triangle)m->DummyTri;
	// Три неинициализированные вершины.
	newotri->Tri[3] = nullptr;
	newotri->Tri[4] = nullptr;
	newotri->Tri[5] = nullptr;

	for (i = 0; i < m->AttributesPerTriangle; i++)
		SetTriAttribute(*newotri, i, 0.0, m->ElemAttribIndex);

	SetAreaBound(*newotri, -1.0, m->AreaBoundIndex);

	newotri->Orient = 0;
}

// Подсчитывает   приблизительно   расширение  (значение,  состоящее
// собственно из значения,  полученного  в результате арифметической
// операции над числами с плавающей точкой, и его ошибки округления).
double DelaunayHelper::Estimate(int elen, double* e)
{
	// Сохранить первый элемент расщирения.
	double Q = e[0];

	// Сложить между собой все элементы расширения.
	for (int eindex = 1; eindex < elen; eindex++)
		Q += e[eindex];

	// Вернуть полученное расширение.
	return Q;
}

// Суммирует  два  расширения  (значения,  каждого из которых  состоит из значения
// результата арифметической операции над  числами с плавающей точкой и его ошибки
// округления). Исключает нулевые компоненты из выходного расширения.
int DelaunayHelper::FastExpansionSumZeroElim(int elen, double* e, int flen, double* f, double* h)
{
	// Буфер для очередного компонента первого расширения (первого слагаемого).
	double enow = e[0];
	// Буфер для очередного компонента второго расширения (второго слагаемого).
	double fnow = f[0];
	// Индекс очередного компонента первого расширения.
	int eindex = 0;
	// Индекс очередного компонента второго расширения.
	int findex = 0;
	// Буфер для промежуточной приблизительной суммы.
	double Q;

	// Если первое слагаемое больше второго слагаемого:
	if ((fnow > enow) == (fnow > -enow))
	{
		// Сохранить значение первого компонента первого слагаемого.
		Q = enow;
		// Инкрементировать индекс компонента первого слагаемого и получить по нему следующий компонент.
		enow = e[++eindex];
	}
	else
	{
		// Сохранить значение первого компонента второго слагаемого.
		Q = fnow;
		// Инкрементировать индекс компонента второго слагаемого и получить по нему следующий компонент.
		fnow = f[++findex];
	}

	// Индекс очередного компонента расширения суммы e + f либо ошибки округления.
	int hindex = 0;
	// Очередное значение промежуточной суммы.
	double Qnew;
	// Ошибка округления сложения компонентов расширения.
	double hh;

	// Если индексы не вышли за границы соответствующих им расширений,
	// то выполнять сложение компонентов этих расширений:
	if ((eindex < elen) && (findex < flen))
	{
		// Сравнить слагаемые для их упорядочения перед использованием:
		// Если 2-й компонент 2-го слагаемого больше 2-го компонента 1-го слагаемого:
		if ((fnow > enow) == (fnow > -enow))
		{
			// Получить сумму первых двух компонентов 1-го слагаемого и ошибку округления данной операции.
			/*FAST_TWO_SUM*/FastTwoSum(enow, Q, Qnew, hh);
			// Получить следующий компонент 1-го слагаемого.
			enow = e[++eindex];
		}
		else
		{
			// Получить сумму первых двух компонентов 2-го слагаемого и ошибку округления данной операции.
			/*FAST_TWO_SUM*/FastTwoSum(fnow, Q, Qnew, hh);
			// Получить следующий компонент 2-го слагаемого.
			fnow = f[++findex];
		}
		// Сохранить полученную сумму.
		Q = Qnew;
		// Сохранить полученную ошибку округления если она не ноль.
		if (hh != 0.0)
			h[hindex++] = hh;
		// Если индексы не вышли за границы соответствующих им расширений,
		// то выполнять дальнейшее сложение компонентов этих расширений:
		while ((eindex < elen) && (findex < flen))
		{
			// Сравнить слагаемые для их упорядочиванием перед использованием:
			// Если очередной компонент 2-го слагаемого больше очередного компонента 1-го слагаемого:
			if ((fnow > enow) == (fnow > -enow))
			{
				// Получить сумму двух очередных компонентов 1-го слагаемого и ошибку округления данной операции.
				TwoSum(Q, enow, Qnew, hh);
				// Получить следующий компонент 1-го слагаемого.
				enow = e[++eindex];
			}
			else
			{
				// Получить сумму двух очередных компонентов 2-го слагаемого и ошибку округления данной операции.
				TwoSum(Q, fnow, Qnew, hh);
				// Получить следующий компонент 2-го слагаемого.
				fnow = f[++findex];
			}
			// Сохранить полученную сумму.
			Q = Qnew;
			// Сохранить полученную ошибку округления если она не ноль.
			if (hh != 0.0)
				h[hindex++] = hh;
		}
	}
	/***
	Продолжать суммировать компоненты слагаемых расширений.
	***/
	while (eindex < elen)
	{
		TwoSum(Q, enow, Qnew, hh);
		enow = e[++eindex];
		Q = Qnew;
		if (hh != 0.0)
			h[hindex++] = hh;
	}
	while (findex < flen)
	{
		TwoSum(Q, fnow, Qnew, hh);
		fnow = f[++findex];
		Q = Qnew;
		if (hh != 0.0)
			h[hindex++] = hh;
	}
	// Если получено результирующее расширение-сумма и нет ошибки округления,
	// то сохранить расширение сумму в буфере результата.
	if ((Q != 0.0) || (hindex == 0))
		h[hindex++] = Q;

	// Вернуть индекс буфера результата, под которым было
	// сохранено результирующее расширение-сумма.
	return hindex;
}

// Умножает расширение (значение, состоящее собственно из значения, полученного
// в результате арифметической операции над числами  с плавающей точкой,  и его
// ошибки  округления)  на скаляр  и исключает  нулевые компоненты из выходного 
// расширения.
int DelaunayHelper::ScaleExpansionZeroElim(int elen, double* e, double b, double* h)
{
	// Старшая часть скалярного сомножителя, получаемая при его разбиении.
	double bhi = 0;
	// Младшая часть скалярного сомножителя, получаемая при его разбиении.
	double blo = 0;

	// Разбить скалярный сомножитель на две части с половинной точностью.
	Split(b, bhi, blo);

	// Старшая часть умножаемого компонента расширения, получаемая при его разбиении.
	double ahi = 0;
	// Младшая часть умножаемого компонента расширения, получаемая при его разбиении.
	double alo = 0;
	// Буфер, в который TWO_PRODUCT_PRESPLIT записывает вычисленное произведение.
	double Q;
	// Буфер, в который TWO_PRODUCT_PRESPLIT записывает ошибку округления.
	double hh;

	// Вычислить произведение текущего компонента расширения на скаляр
	// и получить ошибку округления.
	TwoProductPreSplit(e[0], b, bhi, blo, Q, hh);

	int hindex = 0;
	if (hh != 0)
		h[hindex++] = hh;

	// Текущий компонент умножаемого расширения.
	double enow;
	// Промежуточные значения произведений.
	double product0;
	double product1;
	// Сумма промежуточных значений произведений.
	double sum;

	// Умножать каждый компонент расширения на скаляр и суммировать результаты.
	for (int eindex = 1; eindex < elen; eindex++)
	{
		// Получить значение текущего компонента умножаемого расширения.
		enow = e[eindex];
		// Выполнить умножение этого компонента на скалярный сомножитель.
		TwoProductPreSplit(enow, b, bhi, blo, product1, product0);
		// Суммировать результаты.
		TwoSum(Q, product0, sum, hh);
		if (hh != 0)
			h[hindex++] = hh;
		/*FAST_TWO_SUM*/FastTwoSum(product1, sum, Q, hh);
		if (hh != 0)
			h[hindex++] = hh;
	}
	// Сохранить результирующее произведение в буфере.
	if ((Q != 0.0) || (hindex == 0))
		h[hindex++] = Q;

	// Вернуть индекс, под которым результирующее произведение сохранено в буфере.
	return hindex;
}

// Выполняет  адаптивную   проверку   на  направление  следования  вершин
// в  треугольнике  против часовой стрелки. Вычисляет  последовательность
// точных приближений  к определителю и возвращает результат тогда, когда
// точность знака определителя гарантирована.
double DelaunayHelper::CounterClockwiseAdapt(Vertex pa, Vertex pb, Vertex pc, double detsum)
{
	// - старшая часть первого сомножителя, полученная при его разбиении,
	double ahi = 0;
	// - младшая часть первого сомножителя, полученная при его разбиении,
	double alo = 0;
	// - старшая часть второго сомножителя, полученная при его разбиении,
	double bhi = 0;
	// - младшая часть второго сомножителя, полученная при его разбиении.
	double blo = 0;

	// Элементы матрицы 2*2, для которой подсчитывается возвращаемый минор.
	double acx = (double)(pa[0] - pc[0]);
	double bcx = (double)(pb[0] - pc[0]);
	double acy = (double)(pa[1] - pc[1]);
	double bcy = (double)(pb[1] - pc[1]);

	// Произведения диагоналей матрицы, используемые для подсчёта минора.
	double detleft, detright;
	// Ошибки округления произведений диагоналей матрицы.
	double detlefttail = 0, detrighttail = 0;

	double B3 = 0;
	// Значения этого массива будут использоваться при подсчёте минора.
	double B[4];
	// Вычислить минор, используя ошибки округления (detlefttail и detrighttail) подсчёта 
	// произведения каждой из двух диагоналей матрицы 2*2.
	TwoProduct(acx, bcy, detleft, detlefttail);
	TwoProduct(acy, bcx, detright, detrighttail);
	TwoTwoDiff(detleft, detlefttail, detright, detrighttail, B3, B[2], B[1], B[0]);
	B[3] = B3;
	double det = Estimate(4, B);
	// Вычислить границу погрешности.
	double errbound = ccwerrboundB * detsum;
	// Если точность знака минора гарантируется, то вернуть минор.
	if ((det >= errbound) || (-det >= errbound))
		return det;

	// Подсчитать ошибки округления, имеющие место
	// при определении значений элементов матрицы.
	double acxtail, acytail, bcxtail, bcytail;
	acxtail = TwoDiffTail(pa[0], pc[0], acx);
	bcxtail = TwoDiffTail(pb[0], pc[0], bcx);
	acytail = TwoDiffTail(pa[1], pc[1], acy);
	bcytail = TwoDiffTail(pb[1], pc[1], bcy);

	// Если эти ошибки округления оказались нулевые, то вернуть вычисленный выше минор.
	if ((acxtail == 0.0) && (acytail == 0.0) && (bcxtail == 0.0) && (bcytail == 0.0))
		return det;

	// Подсчитать граничное значение ошибки.
	errbound = ccwerrboundC * detsum + resulterrbound * AbsoluteValue(det);
	// Скорректировать минор, используя полученные выше ошибки округления.
	det += (acx * bcytail + bcy * acxtail) - (acy * bcxtail + bcx * acytail);
	// Вернуть минор, если гарантируется точность его знака.
	if ((det >= errbound) || (-det >= errbound))
		return det;

	// Ошибка округления.
	double s0 = 0;
	// Произведение.
	double s1;
	TwoProduct(acxtail, bcy, s1, s0);

	// Ошибка округления.
	double t0 = 0;
	// Произведение.
	double t1;
	// Продолжить уточняющее вычисление определителя с проверкой точности его значения.
	TwoProduct(acytail, bcx, t1, t0);
	double u[4];
	double u3 = 0;
	TwoTwoDiff(s1, s0, t1, t0, u3, u[2], u[1], u[0]);
	u[3] = u3;
	double C1[8];
	int C1length = FastExpansionSumZeroElim(4, B, 4, u, C1);

	TwoProduct(acx, bcytail, s1, s0);
	TwoProduct(acy, bcxtail, t1, t0);
	TwoTwoDiff(s1, s0, t1, t0, u3, u[2], u[1], u[0]);
	u[3] = u3;
	double C2[12];
	int C2length = FastExpansionSumZeroElim(C1length, C1, 4, u, C2);

	TwoProduct(acxtail, bcytail, s1, s0);
	TwoProduct(acytail, bcxtail, t1, t0);
	TwoTwoDiff(s1, s0, t1, t0, u3, u[2], u[1], u[0]);
	u[3] = u3;
	// Точный результат
	double D[16];
	int Dlength = FastExpansionSumZeroElim(C2length, C2, 4, u, D);

	// Вернуть полученный определитель.
	return(D[Dlength - 1]);
}

// Выполняет проверку следования вершин треугольника в порядке против
// часовой стрелки.  Возвращает минор (определитель матрицы 2*2) знак
// которого  показывает  результат проверки:  положительный - вершины
// в  треугольнике  следуют  друг за другом  против  часовой стрелки,
// отрицательный - вершины  в треугольнике  следуют дуг за другом  по
// часовой стрелке. И, если минор равен нулю, то вершины коллинеарны.
double DelaunayHelper::CounterClockwise(Mesh* m, Configuration* b, Vertex pa, Vertex pb, Vertex pc)
{
	// Определить минор:
	double detleft = (pa[0] - pc[0]) * (pb[1] - pc[1]);
	double detright = (pa[1] - pc[1]) * (pb[0] - pc[0]);
	// Вычислить возвращаемый минор (определитель матрицы 2*2), знак которого
	// указывает результат выполнения проверки следования вершин треугольника
	// в направлении против часовой стрелки. Минор > 0, если вершины pa, pb и
	// pc следуют в порядке против часовой стрелки. Минор < 0,  если точки pa
	// pb и pc следуют в порядке по часовой стрелке. Минор = 0, если точки pa
	// pb и pc коллинеарны.
	double det = detleft - detright;

	// Если не используется точная арифметика, то вернуть полученный минор.
	if (b->noexact)
		return det;

	// Сумма определителей.
	double detsum;

	// В зависимости от значений произведениия 1-го и 4-го элементов матрицы
	// и произведения 3-го и 2-го элементов матрицы вернуть полученный минор
	// либо сумму этих произведений. Иначе получить сумму  этих произведений
	// для подсчёта граничного значения ошибки.
	if (detleft > 0.0)
	{
		if (detright <= 0.0)
			return det;
		else
			detsum = detleft + detright;
	}
	else if (detleft < 0.0)
	{
		if (detright >= 0.0)
			return det;
		else
			detsum = -detleft - detright;
	}
	else
		return det;

	// Подсчитать граничное значение ошибки.
	double errbound = ccwerrboundA * detsum;
	// Если точность знака определителя гарантируется, то вернуть определитель.
	if ((det >= errbound) || (-det >= errbound))
		return det;

	// Иначе, вернуть результат адаптивной проверки на направление следования
	// точек pa, pb и pc (представляющих вершины треугольника) против часовой
	// стрелки.
	return CounterClockwiseAdapt(pa, pb, pc, detsum);
}

// Освобождает память, ранее выделенную под треугольник,
// и помечает этот треугольник как "мёртвый".
void DelaunayHelper::TriangleDealloc(Mesh* m, Triangle* dyingtriangle)
{
	// Пометить треугольник как "мертвый". Это позволяет обнаруживать
	// "мертвые" треугольники при обходе списка всех треугольников.
	KillTriangle(dyingtriangle);
	PoolDealloc(&m->Triangles, (void*)dyingtriangle);
}

// Выполняет адаптивную проверку  на попадание точки в окружность.
// Вычисляет последовательность точных приближений  к определителю
// и возвращает результат тогда, когда точность знака определителя
// гарантирована.
double DelaunayHelper::InCircleAdapt(Vertex pa, Vertex pb, Vertex pc, Vertex pd, double permanent)
{
	double bdxcdy1, cdxbdy1, cdxady1, adxcdy1, adxbdy1, bdxady1;
	double bdxcdy0 = 0, cdxbdy0 = 0, cdxady0 = 0, adxcdy0 = 0, adxbdy0 = 0, bdxady0 = 0;
	double bc[4], ca[4], ab[4];
	double bc3 = 0, ca3 = 0, ab3 = 0;
	double axbc[8], axxbc[16], aybc[8], ayybc[16], adet[32];
	double bxca[8], bxxca[16], byca[8], byyca[16], bdet[32];
	double cxab[8], cxxab[16], cyab[8], cyyab[16], cdet[32];
	double abdet[64];
	double fin1[1152], fin2[1152];
	double* finswap;

	double adxtail, bdxtail, cdxtail, adytail, bdytail, cdytail;
	double adxadx1, adyady1, bdxbdx1, bdybdy1, cdxcdx1, cdycdy1;
	double adxadx0, adyady0, bdxbdx0, bdybdy0, cdxcdx0, cdycdy0;
	double aa[4], bb[4], cc[4];
	double aa3 = 0, bb3 = 0, cc3 = 0;
	double ti1, tj1;
	double ti0 = 0, tj0 = 0;
	double u[4], v[4];
	double u3 = 0, v3 = 0;
	double temp8[8], temp16a[16], temp16b[16], temp16c[16];
	double temp32a[32], temp32b[32], temp48[48], temp64[64];
	int temp8len, temp16alen, temp16blen, temp16clen;
	int temp32alen, temp32blen, temp48len, temp64len;
	double axtbb[8], axtcc[8], aytbb[8], aytcc[8];
	int axtbblen, axtcclen, aytbblen, aytcclen;
	double bxtaa[8], bxtcc[8], bytaa[8], bytcc[8];
	int bxtaalen, bxtcclen, bytaalen, bytcclen;
	double cxtaa[8], cxtbb[8], cytaa[8], cytbb[8];
	int cxtaalen, cxtbblen, cytaalen, cytbblen;
	double axtbc[8], aytbc[8], bxtca[8], bytca[8], cxtab[8], cytab[8];
	int axtbclen, aytbclen, bxtcalen, bytcalen, cxtablen, cytablen;
	double axtbct[16], aytbct[16], bxtcat[16], bytcat[16], cxtabt[16], cytabt[16];
	int axtbctlen, aytbctlen, bxtcatlen, bytcatlen, cxtabtlen, cytabtlen;
	double axtbctt[8], aytbctt[8], bxtcatt[8];
	double bytcatt[8], cxtabtt[8], cytabtt[8];
	int axtbcttlen, aytbcttlen, bxtcattlen, bytcattlen, cxtabttlen, cytabttlen;
	double abt[8], bct[8], cat[8];
	int abtlen, bctlen, catlen;
	double abtt[4], bctt[4], catt[4];
	int abttlen, bcttlen, cattlen;
	double abtt3 = 0, bctt3 = 0, catt3 = 0;
	double negate;
	double ahi = 0, alo = 0, bhi = 0, blo = 0;

	// Подсчитать элементы второго столбца матрицы
	double adx = (double)(pa[0] - pd[0]);
	double bdx = (double)(pb[0] - pd[0]);
	double cdx = (double)(pc[0] - pd[0]);
	// Подсчитать элементы третьего столбца матрицы
	double ady = (double)(pa[1] - pd[1]);
	double bdy = (double)(pb[1] - pd[1]);
	double cdy = (double)(pc[1] - pd[1]);

	TwoProduct(bdx, cdy, bdxcdy1, bdxcdy0);
	TwoProduct(cdx, bdy, cdxbdy1, cdxbdy0);
	TwoTwoDiff(bdxcdy1, bdxcdy0, cdxbdy1, cdxbdy0, bc3, bc[2], bc[1], bc[0]);
	bc[3] = bc3;
	int axbclen = ScaleExpansionZeroElim(4, bc, adx, axbc);
	int axxbclen = ScaleExpansionZeroElim(axbclen, axbc, adx, axxbc);
	int aybclen = ScaleExpansionZeroElim(4, bc, ady, aybc);
	int ayybclen = ScaleExpansionZeroElim(aybclen, aybc, ady, ayybc);
	int alen = FastExpansionSumZeroElim(axxbclen, axxbc, ayybclen, ayybc, adet);

	TwoProduct(cdx, ady, cdxady1, cdxady0);
	TwoProduct(adx, cdy, adxcdy1, adxcdy0);
	TwoTwoDiff(cdxady1, cdxady0, adxcdy1, adxcdy0, ca3, ca[2], ca[1], ca[0]);
	ca[3] = ca3;
	int bxcalen = ScaleExpansionZeroElim(4, ca, bdx, bxca);
	int bxxcalen = ScaleExpansionZeroElim(bxcalen, bxca, bdx, bxxca);
	int bycalen = ScaleExpansionZeroElim(4, ca, bdy, byca);
	int byycalen = ScaleExpansionZeroElim(bycalen, byca, bdy, byyca);
	int blen = FastExpansionSumZeroElim(bxxcalen, bxxca, byycalen, byyca, bdet);

	TwoProduct(adx, bdy, adxbdy1, adxbdy0);
	TwoProduct(bdx, ady, bdxady1, bdxady0);
	TwoTwoDiff(adxbdy1, adxbdy0, bdxady1, bdxady0, ab3, ab[2], ab[1], ab[0]);
	ab[3] = ab3;
	int cxablen = ScaleExpansionZeroElim(4, ab, cdx, cxab);
	int cxxablen = ScaleExpansionZeroElim(cxablen, cxab, cdx, cxxab);
	int cyablen = ScaleExpansionZeroElim(4, ab, cdy, cyab);
	int cyyablen = ScaleExpansionZeroElim(cyablen, cyab, cdy, cyyab);
	int clen = FastExpansionSumZeroElim(cxxablen, cxxab, cyyablen, cyyab, cdet);

	int ablen = FastExpansionSumZeroElim(alen, adet, blen, bdet, abdet);
	int finlength = FastExpansionSumZeroElim(ablen, abdet, clen, cdet, fin1);

	double det = Estimate(finlength, fin1);
	double errbound = iccerrboundB * permanent;
	if ((det >= errbound) || (-det >= errbound))
		return det;

	adxtail = TwoDiffTail(pa[0], pd[0], adx);
	adytail = TwoDiffTail(pa[1], pd[1], ady);
	bdxtail = TwoDiffTail(pb[0], pd[0], bdx);
	bdytail = TwoDiffTail(pb[1], pd[1], bdy);
	cdxtail = TwoDiffTail(pc[0], pd[0], cdx);
	cdytail = TwoDiffTail(pc[1], pd[1], cdy);

	// Если ошибки округления нулевые, то вернуть определитель.
	if ((adxtail == 0.0) && (bdxtail == 0.0) && (cdxtail == 0.0) && (adytail == 0.0) && (bdytail == 0.0) && (cdytail == 0.0))
		return det;

	// Подсчитать границу ошибки.
	errbound = iccerrboundC * permanent + resulterrbound * AbsoluteValue(det);
	// Подсчитат определитель.
	det += ((adx * adx + ady * ady) * ((bdx * cdytail + cdy * bdxtail)
		- (bdy * cdxtail + cdx * bdytail))
		+ 2.0 * (adx * adxtail + ady * adytail) * (bdx * cdy - bdy * cdx))
		+ ((bdx * bdx + bdy * bdy) * ((cdx * adytail + ady * cdxtail)
			- (cdy * adxtail + adx * cdytail))
			+ 2.0 * (bdx * bdxtail + bdy * bdytail) * (cdx * ady - cdy * adx))
		+ ((cdx * cdx + cdy * cdy) * ((adx * bdytail + bdy * adxtail)
			- (ady * bdxtail + bdx * adytail))
			+ 2.0 * (cdx * cdxtail + cdy * cdytail) * (adx * bdy - ady * bdx));
	// Если определитель больше чем граница ошибки, то вернуть определитель.
	if ((det >= errbound) || (-det >= errbound))
		return det;

	double* finnow = fin1;
	double* finother = fin2;

	if ((bdxtail != 0.0) || (bdytail != 0.0) || (cdxtail != 0.0) || (cdytail != 0.0))
	{
		Square(adx, adxadx1, adxadx0);
		Square(ady, adyady1, adyady0);
		TwoTwoSum(adxadx1, adxadx0, adyady1, adyady0, aa3, aa[2], aa[1], aa[0]);
		aa[3] = aa3;
	}
	if ((cdxtail != 0.0) || (cdytail != 0.0) || (adxtail != 0.0) || (adytail != 0.0))
	{
		Square(bdx, bdxbdx1, bdxbdx0);
		Square(bdy, bdybdy1, bdybdy0);
		TwoTwoSum(bdxbdx1, bdxbdx0, bdybdy1, bdybdy0, bb3, bb[2], bb[1], bb[0]);
		bb[3] = bb3;
	}
	if ((adxtail != 0.0) || (adytail != 0.0) || (bdxtail != 0.0) || (bdytail != 0.0))
	{
		Square(cdx, cdxcdx1, cdxcdx0);
		Square(cdy, cdycdy1, cdycdy0);
		TwoTwoSum(cdxcdx1, cdxcdx0, cdycdy1, cdycdy0, cc3, cc[2], cc[1], cc[0]);
		cc[3] = cc3;
	}

	if (adxtail != 0.0)
	{
		axtbclen = ScaleExpansionZeroElim(4, bc, adxtail, axtbc);
		temp16alen = ScaleExpansionZeroElim(axtbclen, axtbc, 2.0 * adx, temp16a);

		axtcclen = ScaleExpansionZeroElim(4, cc, adxtail, axtcc);
		temp16blen = ScaleExpansionZeroElim(axtcclen, axtcc, bdy, temp16b);

		axtbblen = ScaleExpansionZeroElim(4, bb, adxtail, axtbb);
		temp16clen = ScaleExpansionZeroElim(axtbblen, axtbb, -cdy, temp16c);

		temp32alen = FastExpansionSumZeroElim(temp16alen, temp16a, temp16blen, temp16b, temp32a);
		temp48len = FastExpansionSumZeroElim(temp16clen, temp16c, temp32alen, temp32a, temp48);
		finlength = FastExpansionSumZeroElim(finlength, finnow, temp48len, temp48, finother);

		finswap = finnow;
		finnow = finother;
		finother = finswap;
	}
	if (adytail != 0.0)
	{
		aytbclen = ScaleExpansionZeroElim(4, bc, adytail, aytbc);
		temp16alen = ScaleExpansionZeroElim(aytbclen, aytbc, 2.0 * ady, temp16a);

		aytbblen = ScaleExpansionZeroElim(4, bb, adytail, aytbb);
		temp16blen = ScaleExpansionZeroElim(aytbblen, aytbb, cdx, temp16b);

		aytcclen = ScaleExpansionZeroElim(4, cc, adytail, aytcc);
		temp16clen = ScaleExpansionZeroElim(aytcclen, aytcc, -bdx, temp16c);

		temp32alen = FastExpansionSumZeroElim(temp16alen, temp16a, temp16blen, temp16b, temp32a);
		temp48len = FastExpansionSumZeroElim(temp16clen, temp16c, temp32alen, temp32a, temp48);
		finlength = FastExpansionSumZeroElim(finlength, finnow, temp48len, temp48, finother);
		finswap = finnow; finnow = finother; finother = finswap;
	}
	if (bdxtail != 0.0)
	{
		bxtcalen = ScaleExpansionZeroElim(4, ca, bdxtail, bxtca);
		temp16alen = ScaleExpansionZeroElim(bxtcalen, bxtca, 2.0 * bdx, temp16a);

		bxtaalen = ScaleExpansionZeroElim(4, aa, bdxtail, bxtaa);
		temp16blen = ScaleExpansionZeroElim(bxtaalen, bxtaa, cdy, temp16b);

		bxtcclen = ScaleExpansionZeroElim(4, cc, bdxtail, bxtcc);
		temp16clen = ScaleExpansionZeroElim(bxtcclen, bxtcc, -ady, temp16c);

		temp32alen = FastExpansionSumZeroElim(temp16alen, temp16a, temp16blen, temp16b, temp32a);
		temp48len = FastExpansionSumZeroElim(temp16clen, temp16c, temp32alen, temp32a, temp48);
		finlength = FastExpansionSumZeroElim(finlength, finnow, temp48len, temp48, finother);
		finswap = finnow; finnow = finother; finother = finswap;
	}
	if (bdytail != 0.0)
	{
		bytcalen = ScaleExpansionZeroElim(4, ca, bdytail, bytca);
		temp16alen = ScaleExpansionZeroElim(bytcalen, bytca, 2.0 * bdy, temp16a);

		bytcclen = ScaleExpansionZeroElim(4, cc, bdytail, bytcc);
		temp16blen = ScaleExpansionZeroElim(bytcclen, bytcc, adx, temp16b);

		bytaalen = ScaleExpansionZeroElim(4, aa, bdytail, bytaa);
		temp16clen = ScaleExpansionZeroElim(bytaalen, bytaa, -cdx, temp16c);

		temp32alen = FastExpansionSumZeroElim(temp16alen, temp16a, temp16blen, temp16b, temp32a);
		temp48len = FastExpansionSumZeroElim(temp16clen, temp16c, temp32alen, temp32a, temp48);
		finlength = FastExpansionSumZeroElim(finlength, finnow, temp48len, temp48, finother);
		finswap = finnow; finnow = finother; finother = finswap;
	}
	if (cdxtail != 0.0)
	{
		cxtablen = ScaleExpansionZeroElim(4, ab, cdxtail, cxtab);
		temp16alen = ScaleExpansionZeroElim(cxtablen, cxtab, 2.0 * cdx, temp16a);

		cxtbblen = ScaleExpansionZeroElim(4, bb, cdxtail, cxtbb);
		temp16blen = ScaleExpansionZeroElim(cxtbblen, cxtbb, ady, temp16b);

		cxtaalen = ScaleExpansionZeroElim(4, aa, cdxtail, cxtaa);
		temp16clen = ScaleExpansionZeroElim(cxtaalen, cxtaa, -bdy, temp16c);

		temp32alen = FastExpansionSumZeroElim(temp16alen, temp16a, temp16blen, temp16b, temp32a);
		temp48len = FastExpansionSumZeroElim(temp16clen, temp16c,
			temp32alen, temp32a, temp48);
		finlength = FastExpansionSumZeroElim(finlength, finnow, temp48len, temp48, finother);
		finswap = finnow; finnow = finother; finother = finswap;
	}
	if (cdytail != 0.0)
	{
		cytablen = ScaleExpansionZeroElim(4, ab, cdytail, cytab);
		temp16alen = ScaleExpansionZeroElim(cytablen, cytab, 2.0 * cdy, temp16a);

		cytaalen = ScaleExpansionZeroElim(4, aa, cdytail, cytaa);
		temp16blen = ScaleExpansionZeroElim(cytaalen, cytaa, bdx, temp16b);

		cytbblen = ScaleExpansionZeroElim(4, bb, cdytail, cytbb);
		temp16clen = ScaleExpansionZeroElim(cytbblen, cytbb, -adx, temp16c);

		temp32alen = FastExpansionSumZeroElim(temp16alen, temp16a, temp16blen, temp16b, temp32a);
		temp48len = FastExpansionSumZeroElim(temp16clen, temp16c, temp32alen, temp32a, temp48);
		finlength = FastExpansionSumZeroElim(finlength, finnow, temp48len, temp48, finother);
		finswap = finnow; finnow = finother; finother = finswap;
	}

	if ((adxtail != 0.0) || (adytail != 0.0))
	{
		if ((bdxtail != 0.0) || (bdytail != 0.0) || (cdxtail != 0.0) || (cdytail != 0.0))
		{
			TwoProduct(bdxtail, cdy, ti1, ti0);
			TwoProduct(bdx, cdytail, tj1, tj0);
			TwoTwoSum(ti1, ti0, tj1, tj0, u3, u[2], u[1], u[0]);
			u[3] = u3;
			negate = -bdy;
			TwoProduct(cdxtail, negate, ti1, ti0);
			negate = -bdytail;
			TwoProduct(cdx, negate, tj1, tj0);
			TwoTwoSum(ti1, ti0, tj1, tj0, v3, v[2], v[1], v[0]);
			v[3] = v3;
			bctlen = FastExpansionSumZeroElim(4, u, 4, v, bct);

			TwoProduct(bdxtail, cdytail, ti1, ti0);
			TwoProduct(cdxtail, bdytail, tj1, tj0);
			TwoTwoDiff(ti1, ti0, tj1, tj0, bctt3, bctt[2], bctt[1], bctt[0]);
			bctt[3] = bctt3;
			bcttlen = 4;
		}
		else
		{
			bct[0] = 0.0;
			bctlen = 1;
			bctt[0] = 0.0;
			bcttlen = 1;
		}

		if (adxtail != 0.0)
		{
			temp16alen = ScaleExpansionZeroElim(axtbclen, axtbc, adxtail, temp16a);
			axtbctlen = ScaleExpansionZeroElim(bctlen, bct, adxtail, axtbct);
			temp32alen = ScaleExpansionZeroElim(axtbctlen, axtbct, 2.0 * adx, temp32a);
			temp48len = FastExpansionSumZeroElim(temp16alen, temp16a, temp32alen, temp32a, temp48);
			finlength = FastExpansionSumZeroElim(finlength, finnow, temp48len, temp48, finother);
			finswap = finnow; finnow = finother; finother = finswap;
			if (bdytail != 0.0)
			{
				temp8len = ScaleExpansionZeroElim(4, cc, adxtail, temp8);
				temp16alen = ScaleExpansionZeroElim(temp8len, temp8, bdytail, temp16a);
				finlength = FastExpansionSumZeroElim(finlength, finnow, temp16alen, temp16a, finother);
				finswap = finnow; finnow = finother; finother = finswap;
			}
			if (cdytail != 0.0)
			{
				temp8len = ScaleExpansionZeroElim(4, bb, -adxtail, temp8);
				temp16alen = ScaleExpansionZeroElim(temp8len, temp8, cdytail, temp16a);
				finlength = FastExpansionSumZeroElim(finlength, finnow, temp16alen, temp16a, finother);
				finswap = finnow; finnow = finother; finother = finswap;
			}

			temp32alen = ScaleExpansionZeroElim(axtbctlen, axtbct, adxtail, temp32a);
			axtbcttlen = ScaleExpansionZeroElim(bcttlen, bctt, adxtail, axtbctt);
			temp16alen = ScaleExpansionZeroElim(axtbcttlen, axtbctt, 2.0 * adx, temp16a);
			temp16blen = ScaleExpansionZeroElim(axtbcttlen, axtbctt, adxtail, temp16b);
			temp32blen = FastExpansionSumZeroElim(temp16alen, temp16a, temp16blen, temp16b, temp32b);
			temp64len = FastExpansionSumZeroElim(temp32alen, temp32a, temp32blen, temp32b, temp64);
			finlength = FastExpansionSumZeroElim(finlength, finnow, temp64len, temp64, finother);
			finswap = finnow; finnow = finother; finother = finswap;
		}
		if (adytail != 0.0)
		{
			temp16alen = ScaleExpansionZeroElim(aytbclen, aytbc, adytail, temp16a);
			aytbctlen = ScaleExpansionZeroElim(bctlen, bct, adytail, aytbct);
			temp32alen = ScaleExpansionZeroElim(aytbctlen, aytbct, 2.0 * ady, temp32a);
			temp48len = FastExpansionSumZeroElim(temp16alen, temp16a, temp32alen, temp32a, temp48);
			finlength = FastExpansionSumZeroElim(finlength, finnow, temp48len, temp48, finother);
			finswap = finnow; finnow = finother; finother = finswap;

			temp32alen = ScaleExpansionZeroElim(aytbctlen, aytbct, adytail, temp32a);
			aytbcttlen = ScaleExpansionZeroElim(bcttlen, bctt, adytail, aytbctt);
			temp16alen = ScaleExpansionZeroElim(aytbcttlen, aytbctt, 2.0 * ady, temp16a);
			temp16blen = ScaleExpansionZeroElim(aytbcttlen, aytbctt, adytail, temp16b);
			temp32blen = FastExpansionSumZeroElim(temp16alen, temp16a, temp16blen, temp16b, temp32b);
			temp64len = FastExpansionSumZeroElim(temp32alen, temp32a, temp32blen, temp32b, temp64);
			finlength = FastExpansionSumZeroElim(finlength, finnow, temp64len, temp64, finother);
			finswap = finnow; finnow = finother; finother = finswap;
		}
	}
	if ((bdxtail != 0.0) || (bdytail != 0.0))
	{
		if ((cdxtail != 0.0) || (cdytail != 0.0) || (adxtail != 0.0) || (adytail != 0.0))
		{
			TwoProduct(cdxtail, ady, ti1, ti0);
			TwoProduct(cdx, adytail, tj1, tj0);
			TwoTwoSum(ti1, ti0, tj1, tj0, u3, u[2], u[1], u[0]);
			u[3] = u3;
			negate = -cdy;
			TwoProduct(adxtail, negate, ti1, ti0);
			negate = -cdytail;
			TwoProduct(adx, negate, tj1, tj0);
			TwoTwoSum(ti1, ti0, tj1, tj0, v3, v[2], v[1], v[0]);
			v[3] = v3;
			catlen = FastExpansionSumZeroElim(4, u, 4, v, cat);

			TwoProduct(cdxtail, adytail, ti1, ti0);
			TwoProduct(adxtail, cdytail, tj1, tj0);
			TwoTwoDiff(ti1, ti0, tj1, tj0, catt3, catt[2], catt[1], catt[0]);
			catt[3] = catt3;
			cattlen = 4;
		}
		else
		{
			cat[0] = 0.0;
			catlen = 1;
			catt[0] = 0.0;
			cattlen = 1;
		}

		if (bdxtail != 0.0)
		{
			temp16alen = ScaleExpansionZeroElim(bxtcalen, bxtca, bdxtail, temp16a);
			bxtcatlen = ScaleExpansionZeroElim(catlen, cat, bdxtail, bxtcat);
			temp32alen = ScaleExpansionZeroElim(bxtcatlen, bxtcat, 2.0 * bdx, temp32a);
			temp48len = FastExpansionSumZeroElim(temp16alen, temp16a, temp32alen, temp32a, temp48);
			finlength = FastExpansionSumZeroElim(finlength, finnow, temp48len, temp48, finother);
			finswap = finnow; finnow = finother; finother = finswap;
			if (cdytail != 0.0)
			{
				temp8len = ScaleExpansionZeroElim(4, aa, bdxtail, temp8);
				temp16alen = ScaleExpansionZeroElim(temp8len, temp8, cdytail, temp16a);
				finlength = FastExpansionSumZeroElim(finlength, finnow, temp16alen, temp16a, finother);
				finswap = finnow; finnow = finother; finother = finswap;
			}
			if (adytail != 0.0)
			{
				temp8len = ScaleExpansionZeroElim(4, cc, -bdxtail, temp8);
				temp16alen = ScaleExpansionZeroElim(temp8len, temp8, adytail, temp16a);
				finlength = FastExpansionSumZeroElim(finlength, finnow, temp16alen, temp16a, finother);
				finswap = finnow; finnow = finother; finother = finswap;
			}

			temp32alen = ScaleExpansionZeroElim(bxtcatlen, bxtcat, bdxtail,
				temp32a);
			bxtcattlen = ScaleExpansionZeroElim(cattlen, catt, bdxtail, bxtcatt);
			temp16alen = ScaleExpansionZeroElim(bxtcattlen, bxtcatt, 2.0 * bdx,
				temp16a);
			temp16blen = ScaleExpansionZeroElim(bxtcattlen, bxtcatt, bdxtail, temp16b);
			temp32blen = FastExpansionSumZeroElim(temp16alen, temp16a, temp16blen, temp16b, temp32b);
			temp64len = FastExpansionSumZeroElim(temp32alen, temp32a, temp32blen, temp32b, temp64);
			finlength = FastExpansionSumZeroElim(finlength, finnow, temp64len, temp64, finother);
			finswap = finnow; finnow = finother; finother = finswap;
		}
		if (bdytail != 0.0)
		{
			temp16alen = ScaleExpansionZeroElim(bytcalen, bytca, bdytail, temp16a);
			bytcatlen = ScaleExpansionZeroElim(catlen, cat, bdytail, bytcat);
			temp32alen = ScaleExpansionZeroElim(bytcatlen, bytcat, 2.0 * bdy, temp32a);
			temp48len = FastExpansionSumZeroElim(temp16alen, temp16a, temp32alen, temp32a, temp48);
			finlength = FastExpansionSumZeroElim(finlength, finnow, temp48len, temp48, finother);
			finswap = finnow; finnow = finother; finother = finswap;

			temp32alen = ScaleExpansionZeroElim(bytcatlen, bytcat, bdytail, temp32a);
			bytcattlen = ScaleExpansionZeroElim(cattlen, catt, bdytail, bytcatt);
			temp16alen = ScaleExpansionZeroElim(bytcattlen, bytcatt, 2.0 * bdy, temp16a);
			temp16blen = ScaleExpansionZeroElim(bytcattlen, bytcatt, bdytail, temp16b);
			temp32blen = FastExpansionSumZeroElim(temp16alen, temp16a, temp16blen, temp16b, temp32b);
			temp64len = FastExpansionSumZeroElim(temp32alen, temp32a, temp32blen, temp32b, temp64);
			finlength = FastExpansionSumZeroElim(finlength, finnow, temp64len, temp64, finother);
			finswap = finnow; finnow = finother; finother = finswap;
		}
	}
	if ((cdxtail != 0.0) || (cdytail != 0.0))
	{
		if ((adxtail != 0.0) || (adytail != 0.0) || (bdxtail != 0.0) || (bdytail != 0.0))
		{
			TwoProduct(adxtail, bdy, ti1, ti0);
			TwoProduct(adx, bdytail, tj1, tj0);
			TwoTwoSum(ti1, ti0, tj1, tj0, u3, u[2], u[1], u[0]);
			u[3] = u3;
			negate = -ady;
			TwoProduct(bdxtail, negate, ti1, ti0);
			negate = -adytail;
			TwoProduct(bdx, negate, tj1, tj0);
			TwoTwoSum(ti1, ti0, tj1, tj0, v3, v[2], v[1], v[0]);
			v[3] = v3;
			abtlen = FastExpansionSumZeroElim(4, u, 4, v, abt);

			TwoProduct(adxtail, bdytail, ti1, ti0);
			TwoProduct(bdxtail, adytail, tj1, tj0);
			TwoTwoDiff(ti1, ti0, tj1, tj0, abtt3, abtt[2], abtt[1], abtt[0]);
			abtt[3] = abtt3;
			abttlen = 4;
		}
		else
		{
			abt[0] = 0.0;
			abtlen = 1;
			abtt[0] = 0.0;
			abttlen = 1;
		}

		if (cdxtail != 0.0)
		{
			temp16alen = ScaleExpansionZeroElim(cxtablen, cxtab, cdxtail, temp16a);
			cxtabtlen = ScaleExpansionZeroElim(abtlen, abt, cdxtail, cxtabt);
			temp32alen = ScaleExpansionZeroElim(cxtabtlen, cxtabt, 2.0 * cdx, temp32a);
			temp48len = FastExpansionSumZeroElim(temp16alen, temp16a, temp32alen, temp32a, temp48);
			finlength = FastExpansionSumZeroElim(finlength, finnow, temp48len, temp48, finother);
			finswap = finnow; finnow = finother; finother = finswap;
			if (adytail != 0.0)
			{
				temp8len = ScaleExpansionZeroElim(4, bb, cdxtail, temp8);
				temp16alen = ScaleExpansionZeroElim(temp8len, temp8, adytail, temp16a);
				finlength = FastExpansionSumZeroElim(finlength, finnow, temp16alen, temp16a, finother);
				finswap = finnow; finnow = finother; finother = finswap;
			}
			if (bdytail != 0.0)
			{
				temp8len = ScaleExpansionZeroElim(4, aa, -cdxtail, temp8);
				temp16alen = ScaleExpansionZeroElim(temp8len, temp8, bdytail, temp16a);
				finlength = FastExpansionSumZeroElim(finlength, finnow, temp16alen, temp16a, finother);
				finswap = finnow; finnow = finother; finother = finswap;
			}

			temp32alen = ScaleExpansionZeroElim(cxtabtlen, cxtabt, cdxtail,
				temp32a);
			cxtabttlen = ScaleExpansionZeroElim(abttlen, abtt, cdxtail, cxtabtt);
			temp16alen = ScaleExpansionZeroElim(cxtabttlen, cxtabtt, 2.0 * cdx, temp16a);
			temp16blen = ScaleExpansionZeroElim(cxtabttlen, cxtabtt, cdxtail, temp16b);
			temp32blen = FastExpansionSumZeroElim(temp16alen, temp16a, temp16blen, temp16b, temp32b);
			temp64len = FastExpansionSumZeroElim(temp32alen, temp32a, temp32blen, temp32b, temp64);
			finlength = FastExpansionSumZeroElim(finlength, finnow, temp64len, temp64, finother);
			finswap = finnow; finnow = finother; finother = finswap;
		}
		if (cdytail != 0.0)
		{
			temp16alen = ScaleExpansionZeroElim(cytablen, cytab, cdytail, temp16a);
			cytabtlen = ScaleExpansionZeroElim(abtlen, abt, cdytail, cytabt);
			temp32alen = ScaleExpansionZeroElim(cytabtlen, cytabt, 2.0 * cdy, temp32a);
			temp48len = FastExpansionSumZeroElim(temp16alen, temp16a, temp32alen, temp32a, temp48);
			finlength = FastExpansionSumZeroElim(finlength, finnow, temp48len, temp48, finother);
			finswap = finnow; finnow = finother; finother = finswap;

			temp32alen = ScaleExpansionZeroElim(cytabtlen, cytabt, cdytail,
				temp32a);
			cytabttlen = ScaleExpansionZeroElim(abttlen, abtt, cdytail, cytabtt);
			temp16alen = ScaleExpansionZeroElim(cytabttlen, cytabtt, 2.0 * cdy, temp16a);
			temp16blen = ScaleExpansionZeroElim(cytabttlen, cytabtt, cdytail, temp16b);
			temp32blen = FastExpansionSumZeroElim(temp16alen, temp16a, temp16blen, temp16b, temp32b);
			temp64len = FastExpansionSumZeroElim(temp32alen, temp32a, temp32blen, temp32b, temp64);
			finlength = FastExpansionSumZeroElim(finlength, finnow, temp64len, temp64, finother);
			finswap = finnow; finnow = finother; finother = finswap;
		}
	}

	// Вернуть полученный определитель.
	return finnow[finlength - 1];
}

// Выполняет проверку на попадание точки в окружность,  описанную вокруг
// треугольника.  Возвращает  определитель матрицы уравнения окружности,
// проходящей  через вершины  треугольника. Знак определителя показывает
// результат проверки: положительный - точка pd лежит внутри окружности,
// отрицательный - точка pd  лежит вне окружности.  И, если определитель
// равен нулю,  то точка pd принадлежит окружности  (окружность проходит
// через точки pa, pb, pc и pd).
double DelaunayHelper::InCircle(Mesh* m, Configuration* b, Vertex pa, Vertex pb, Vertex pc, Vertex pd)
{
	// Полученные через входные параметры исходные данные представляют матрицу
	// (где строка ".pow(2)" обозначает "в квадрате"):

	// | adx.pow(2) + ady.pow(2)   adx   ady |
	// | bdx.pow(2) + bdy.pow(2)   bdx   bdy |
	// | cdx.pow(2) + cdy.pow(2)   cdx   cdy |

	// Раскрыть определитель данной матрицы по её первому столбцу.

	// Первый элемент второго столбца.
	double adx = pa[0] - pd[0];
	// Второй элемент второго столбца.
	double bdx = pb[0] - pd[0];
	// Третий элемент второго столбца.
	double cdx = pc[0] - pd[0];
	// Первый элемент третьего столбца.
	double ady = pa[1] - pd[1];
	// Второй элемент третьего столбца.
	double bdy = pb[1] - pd[1];
	// Третий элемент третьего столбца.
	double cdy = pc[1] - pd[1];

	// Данные для подсчёта первого минора, используемого при раскрытии определителя.
	double bdxcdy = bdx * cdy;
	double cdxbdy = cdx * bdy;
	// Первый элемент первого столбца, по которому раскрывается определитель.
	double alift = adx * adx + ady * ady;

	// Данные для подсчёта второго минора, используемого при раскрытии определителя.
	double cdxady = cdx * ady;
	double adxcdy = adx * cdy;
	// Второй элемент первого столбца, по которому раскрывается определитель.
	double blift = bdx * bdx + bdy * bdy;

	// Данные для подсчёта третьего минора, используемого при раскрытии определителя.
	double adxbdy = adx * bdy;
	double bdxady = bdx * ady;
	// Третий элемент первого столбца, по которому раскрывается определитель.
	double clift = cdx * cdx + cdy * cdy;

	// Вычислить возвращаемый определитель, знак которого указывает результат выполнения проверки.
	// Определитель > 0, если точка pd лежит внутри окружности.  Определитель < 0,  если точка pd
	// лежит вне  окружности. Определитель = 0, если все четыре точки принадлежат окружности.
	// Точки pa, pb и pc должны распологаться против часовой стрелки, иначе знак результата будет
	// обратным.
	double det = alift * (bdxcdy - cdxbdy) + blift * (cdxady - adxcdy) + clift * (adxbdy - bdxady);

	// Если не используется точная арифметика, то вернуть полученный определитель.
	if (b->noexact)
		return det;

	double permanent = (AbsoluteValue(bdxcdy) + AbsoluteValue(cdxbdy)) * alift
		+ (AbsoluteValue(cdxady) + AbsoluteValue(adxcdy)) * blift
		+ (AbsoluteValue(adxbdy) + AbsoluteValue(bdxady)) * clift;
	// Найти границу погрешности для проверки попадания точки в окружность.
	double errbound = iccerrboundA * permanent;

	// Если точность знака определителя гарантируется, то вернуть определитель.
	if ((det > errbound) || (-det > errbound))
		return det;

	// Иначе, вернуть результат адаптивной проверки на попадание точки в окружность.
	return InCircleAdapt(pa, pb, pc, pd, permanent);
}

/*** Определения функций из пространства имён UsualHelper ***/

// Определяет, можно ли читать память, на которую ссылается
// передаваемый (в качестве аргумента) указатель на void.
bool UsualHelper::IsBadReadPointer(void * checkedPointer)
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };
	if (::VirtualQuery(checkedPointer, &mbi, sizeof(mbi)))
	{
		DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
		bool b = !(mbi.Protect & mask);
		// Если страница "сторожевая" или недоступная для чтения,
		// то проверяемый указатель не будет читаться.
		if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
			b = true;

		return b;
	}
	return true;
}

// Генерирует случайное число.
unsigned long UsualHelper::Randomnation(unsigned int choices)
{
	// Установить начальное значение "зерна" для генерации случайного числа в единицу.
	static unsigned long randomseed = 1;
	// Вычислить случайное значение.
	randomseed = (randomseed * 1366l + 150889l) % 714025l;
	// Вернуть полученное случайное значение.
	return randomseed / (714025l / choices + 1);
}

// Выделяет память.
void* UsualHelper::TriMalloc(int size)
{
	void* memptr;

	memptr = malloc((unsigned int)size);
	if (memptr == nullptr)
	{
		MessageBox(NULL, L"Ошибка: Недостаточно памяти.", L"Триангулятор", MB_ICONERROR | MB_OK);
		TriExit(1);
	}
	return(memptr);
}

// Освобождает указанную область выделенной памяти.
void UsualHelper::TriFree(void* memptr)
{
	if (!IsBadReadPointer(memptr))
		free(memptr);
}

// Выполняет выход из приложения.
void UsualHelper::TriExit(int status)
{
	exit(status);
}

// Возвращает текст сообщения об ошибке, код которой было получен
// вызовом GetLastError(), или пустую строку, если не было ошибки.
std::wstring UsualHelper::GetLastErrorMessage(DWORD errorMessageID)
{
	// Если нет ошибки, то вернуть пустую строку.
	if (errorMessageID == 0)
		return std::wstring();

	// Определить буфер для формируемого сообщения об ошибке.
	LPWSTR messageBuffer = nullptr;
	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

	// Получить из буфера сформированное сообщение об ошибке.
	std::wstring message(messageBuffer, size);

	// Освободить память, используемую под буфер.
	LocalFree(messageBuffer);

	// Вернуть полученное сообщение об ошибке.
	return message;
}
