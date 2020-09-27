#pragma once

#include <iostream>
#include "DelaunayStructures.h"

// Сообщение для установки индикатора выполнения в исходное состояние.
const UINT WM_RESETPROGRESS = WM_USER + 1;
// Сообщение для изменения индикатора выполнения.
const UINT WM_UPDATEPROGRESS = WM_USER + 2;
// Сообщение для вывода текстового сообщения в панели состояния приложения.
const UINT WM_WRITESTATUSTEXT = WM_USER + 3;

using namespace DelaunayStructures;

// Содержит вспомогательные подставляемые функции, относящиеся к триангуляции Делоне.
namespace DelaunayHelper
{
	// Массивы для ускорения некоторых примитивов, работающих с триангуляционной сеткой.
	constexpr int plus1mod3[3] = { 1, 2, 0 };
	constexpr int minus1mod3[3] = { 2, 0, 1 };
	// Используется для разбиения чисел с плавающей точкой на два значения
	// с половинной точностью для выполнения точного умножения.
	static double splitter;

	// Содержит подставляемые функции, относящиеся к выполнению триангуляции Делоне.
	namespace AdaptiveArith
	{
		// Вычисляет ошибку округления вычисленной суммы двух чисел с плавающей точкой.
		inline double FastTwoSumTail(double a, double b, double x)
		{
			return b - (x - a);
		}

		// Вычисляет приближённое значение суммы двух чисел с плавающей точкой.
		inline void FastTwoSum(double a, double b, double& x, double& y)
		{
			x = (double)(a + b);
			y = FastTwoSumTail(a, b, x);
		}

		// Подсчитывает ошибку округления суммы двух чисел с плавающей точкой
		// и записывает значение этой ошибки в параметр y.
		inline double TwoSumTail(double a, double b, double x)
		{
			// - фактическое значение второго слагаемого
			double bvirt = (double)(x - a);
			// - фактическое значение первого слагаемого
			double avirt = x - bvirt;
			// - ошибка округления второго слагаемого
			double bround = b - bvirt;
			// - ошибка округления первого слагаемого
			double around = a - avirt;
			// - ошибка округления суммы
			return around + bround;
		}

		// Cуммирует (a + b) и затем определят ошибку округления.
		inline void TwoSum(double a, double b, double& x, double& y)
		{
			x = (double)(a + b);
			y = TwoSumTail(a, b, x);
		}

		inline void TwoTwoSum(double a1, double a0, double b1, double b0, double& x3, double& x2, double& x1, double& x0)
		{
			double _i = 0, _j = 0, _0 = 0;
			TwoSum(a0, b0, _i, x0);
			TwoSum(a1, _i, _j, _0);
			TwoSum(_0, b1, _i, x1);
			TwoSum(_j, _i, x3, x2);
		}

		// Разбивает переданный сомножитель,  представляющий собой  значение
		// с плавающей точкой, на две  неперекрывающиеся  между собой  части
		// с половинной точностью каждая для последующего выполнения точного
		// умножения.
		inline void Split(double a, double& aHi, double& aLo)
		{
			double c = (double)(splitter * a);
			double aBig = (double)(c - a);
			aHi = c - aBig;
			aLo = a - aHi;
		}

		// Подсчитывает ошибку округления при умножении двух чисел с плавающей точкой.
		inline double TwoProductTail(double a, double b, double x)
		{
			double aHi, aLo, bHi, bLo, err1, err2, err3;
			Split(a, aHi, aLo);
			Split(b, bHi, bLo);
			err1 = x - (aHi * bHi);
			err2 = err1 - (aLo * bHi);
			err3 = err2 - (aHi * bLo);
			return (aLo * bLo) - err3;
		}

		// Выполняет умножение двух чисел с плавающей точкой.
		inline void TwoProduct(double a, double b, double& x, double& y)
		{
			x = (double)(a * b);
			y = TwoProductTail(a, b, x);
		}

		// Выполняет умножение двух чисел с плавающей точкой. Работает аналогично TWO_PRODUCT (),
		// где один из входов уже был разбит на две части. Предотвращает избыточное разбиение.
		inline void TwoProductPreSplit(double a, double b, double bhi, double blo, double& x, double& y)
		{
			x = (double)(a * b);
			double ahi, alo;
			Split(a, ahi, alo);
			auto err1 = x - (ahi * bhi);
			auto err2 = err1 - (alo * bhi);
			auto err3 = err2 - (ahi * blo);
			y = (alo * blo) - err3;
		}

		// Вычисляет ошибку округления, возникающую при вычитании одного числа с плавающей точкой
		// из другого числа с плавающей точкой.
		inline double TwoDiffTail(double a, double b, double x)
		{
			double bVirt = (double)(a - x);
			double aVirt = x + bVirt;
			double bRound = bVirt - b;
			double aRound = a - aVirt;
			return aRound + bRound;
		}

		// Вычитает одно число с плавающей точкой из другого числа с плавающей точкой.
		inline void TwoDiff(double a, double b, double& x, double& y)
		{
			x = (double)(a - b);
			y = TwoDiffTail(a, b, x);
		}

		// Подсчитывает разность двух чисел с плавающей точкой.
		inline void TwoTwoDiff(double a1, double a0, double b1, double b0, double& x3, double& x2, double& x1, double& x0)
		{
			double _i = 0, _j = 0, _0 = 0;
			TwoDiff(a0, b0, _i, x0);
			TwoSum(a1, _i, _j, _0);
			TwoDiff(_0, b1, _i, x1);
			TwoSum(_j, _i, x3, x2);
		}

		// Возвращает абсолютное значение числа.
		inline double AbsoluteValue(double a)
		{
			return ((a) >= 0.0 ? (a) : -(a));
		}

		// Вычисляет ошибку округления вычисленной площади.
		inline double SquareTail(double a, double x)
		{
			double aHi = 0, aLo = 0, err1 = 0, err3 = 0;
			Split(a, aHi, aLo);
			err1 = x - (aHi * aHi);
			err3 = err1 - ((aHi + aHi) * aLo);
			return (aLo * aLo) - err3;
		}

		// Вычисляет значение площади.
		inline void Square(double a, double& x, double& y)
		{
			x = (double)(a * a);
			y = SquareTail(a, x);

		}
	}

	// Содержит подставляемые функции, предназначенные для работы с треугольниками.
	namespace Triangles
	{
		/*** Рёбра треугольника ***/

		// Находит предыдущее (по часовой стрелке) ребро треугольника.
		inline void FindPreviouseLeftEdge(OTriangle oTri1, OTriangle& oTri2)
		{
			(oTri2).Tri = (oTri1).Tri;
			(oTri2).Orient = minus1mod3[(oTri1).Orient];
		}

		// Находит следующее ребро треугольника по направлению против часовой стрелке.
		inline void FindNextLeftEdge(OTriangle oTri1, OTriangle& oTri2)
		{
			(oTri2).Tri = (oTri1).Tri;
			(oTri2).Orient = plus1mod3[(oTri1).Orient];
		}

		// Находит предыдущее ребро треугольника по направлению по часовой стрелке.
		inline int FindSelfPreviouseLeftEdge(int triOrientation)
		{
			return minus1mod3[triOrientation];
		}

		// Находит следующее ребро треугольника по направлению против часовой стрелки.
		inline int FindSelfNextLeftEdge(int orientation)
		{
			return plus1mod3[orientation];
		}


		/*** Треугольники ***/

		// Устанавливает тип вершины.
		inline void SetVertexType(Vertex& vx, int value, int vertexMarkIndex)
		{
			((int *)(vx))[vertexMarkIndex] = value;
		}

		// Возвращает тип вершины треугольника.
		inline int GetVertexType(Vertex vx, int vertexMarkIndex)
		{
			return ((int *)(vx))[vertexMarkIndex + 1];
		}

		// Устанавливает маркер вершины.
		inline void SetVertexMark(Vertex& vx, int value, int vertexMarkIndex)
		{
			((int *)(vx))[vertexMarkIndex] = value;
		}

		// Возвращает маркер вершины.
		inline int GetVertexMark(Vertex vx, int vertexMarkIndex)
		{
			return ((int *)(vx))[vertexMarkIndex];
		}

		// Возвращает начальную вершину у треугольника.
		inline Vertex GetOrignVertex(OTriangle otri)
		{
			return (Vertex)(otri).Tri[plus1mod3[(otri).Orient] + 3];
		}

		// Возвращает конечную вершину у треугольника.
		inline Vertex GetDestinationVertex(OTriangle otri)
		{
			return (Vertex)(otri).Tri[minus1mod3[(otri).Orient] + 3];
		}

		// Возвращает апекс у треугольника.
		inline Vertex GetApex(OTriangle oTri)
		{
			return (Vertex)(oTri).Tri[(oTri).Orient + 3];
		}

		// Устанавливает атрибут треугольника.
		inline void SetTriAttribute(OTriangle& otri, int attnum, double value, int triAttribIndex)
		{
			((double*)(otri).Tri)[triAttribIndex + (attnum)] = value;
		}

		// Возвращает атрибут треугольника.
		inline double GetTriAttribute(OTriangle otri, int attnum, int elemAttribIndex)
		{
			return ((double*)(otri).Tri)[elemAttribIndex + (attnum)];
		}

		// Устанавливает ограничение максимальной площади треугольника.
		inline void SetAreaBound(OTriangle& otri, double value, int areaBoundIndex)
		{
			((double*)(otri).Tri)[areaBoundIndex] = value;
		}

		// Преобразует указатель в ориентированный треугольник. Ориентация извлекается
		// из двух младших значащих бит указателя
		inline void PointerToOtri(Triangle ptr, OTriangle& otri)
		{
			(otri).Orient = (int)((unsigned long long) (ptr) & (unsigned long long) 3l);
			(otri).Tri = (Triangle *)((unsigned long long) (ptr) ^ (unsigned long long) (otri).Orient);
		}

		// Сжимает ориентированный треугольник в отдельный указатель.
		// Основан на предположении,  что все треугольники выровнены 
		// по четырехбайтовым границам, поэтому два младших значащих
		// бита (otri).Tri равны нулю.
		inline Triangle OtriToPointer(OTriangle otri)
		{
			return (Triangle)((unsigned long long) (otri).Tri | (unsigned long long) (otri).Orient);
		}

		// Находит примыкающий треугольник на том же ребре. Обратите внимание, что направление
		// ребра  обязательно  меняется  на противоположное, потому что  дескриптор, указанный 
		// ориентированным треугольником, направлен против часовой стрелки вокруг треугольника.
		inline void FindAbutingTriOnSameEdge(OTriangle otri1, OTriangle& otri2)
		{
			Triangle ptr = (otri1).Tri[(otri1).Orient];
			PointerToOtri(ptr, otri2);
		}

		// Находит заданный в аргументе треугольник.
		inline void FindSimilarTri(OTriangle& otri)
		{
			Triangle ptr = (otri).Tri[(otri).Orient];
			PointerToOtri(ptr, otri);
		}

		// Копирует ориентированный треугольник.
		inline void CopyOtri(OTriangle otri1, OTriangle& otri2)
		{
			(otri2).Tri = (otri1).Tri;
			(otri2).Orient = (otri1).Orient;
		}

		// Устанавливает исходный треугольник (???начальную вершину???, ???исходный треугольник???)
		inline void SetOriginTri(OTriangle& otri, Vertex vertexptr)
		{
			(otri).Tri[plus1mod3[(otri).Orient] + 3] = (Triangle)vertexptr;
		}

		// Устанавливает целевой треугольник.
		inline void SetDestinationTri(OTriangle& otri, Vertex vertexptr)
		{
			(otri).Tri[minus1mod3[(otri).Orient] + 3] = (Triangle)vertexptr;
		}

		// Устанавливает треугольник по переданному апексу.
		inline void SetApexTri(OTriangle& otri, Vertex vertexptr)
		{
			(otri).Tri[(otri).Orient + 3] = (Triangle)vertexptr;
		}

		// Связывает вместе два треугольника.
		inline void BondTwoTriangles(OTriangle& otri1, OTriangle& otri2)
		{
			(otri1).Tri[(otri1).Orient] = OtriToPointer(otri2);
			(otri2).Tri[(otri2).Orient] = OtriToPointer(otri1);
		}

		// "Растворяет" связь (с одной стороны). Следует обратить внимание на то, что другой
		// треугольник  все еще будет "думать", что он связан с этим  треугольником. Однако 
		// обычно, другой треугольник полностью удаляется или соединяется с каким-либо другим
		// треугольником, поэтому это не имеет значения.
		inline void Dissolve(OTriangle& otri, Triangle* dummyTriangle)
		{
			(otri).Tri[(otri).Orient] = (Triangle)dummyTriangle;
		}

		// Проверяет ориентированные треугольники на их эквивалентность.
		inline bool AreTwoOrientedTrianglesEqual(OTriangle oTri1, OTriangle  oTri2)
		{
			return (((oTri1).Tri == (oTri2).Tri) && ((oTri1).Orient == (oTri2).Orient));
		}

		// Освобождает память, выделенную под треугольник.
		inline void KillTriangle(Triangle* tria)
		{
			(tria)[1] = nullptr;
			(tria)[3] = nullptr;
		}

		// Проверяет, была ли забрана назад память, выделенная под треугольник.
		inline bool IsTriangleKilled(Triangle* tria)
		{
			return ((tria)[1] == nullptr);
		}
	}


	/*** Функции и макроопределения ***/

	// Идентифицирует удалённую вершину.
	const int DEADVERTEX = -32768;

	// Выполняет начальную инициализацию используемых переменных.
	void DelaunayHelperInit();

	// Запускает обход по блокам.
	void TraversalInit(MemoryPool *pool);

	// Ищет следующий элемент в списке.
	void* Traverse(MemoryPool* pool);

	// Проходит через вершины, пропуская, при этом, "мертвые" вершины.
	Vertex VertexTraverse(Mesh* m);

	// Проходит через треугольники, пропуская, при этом, "мёртвые треугольники".
	Triangle* TriangleTraverse(MemoryPool* pool);

	// Устанавливает тип для вершины.
	inline void SetVertexType(Mesh* m, Vertex vert, const int vertType)
	{
		((int*)(vert))[m->VertexMarkIndex + 1] = vertType;
	}

	// Выделяет память под элемент.
	void* PoolAlloc(MemoryPool* pool);

	// Освобождает память, выделенную под элемент. Освобождённое адресное пространство
	// сохраняется в очереди для его повторного использования в дальнейшем.
	void PoolDealloc(MemoryPool* pool, void* dyingitem);

	// Создаёт новый треугольник с нулевой ориентацией.
	void MakeTriangle(Mesh* m, Configuration* b, OTriangle* newotri);

	// Подсчитывает   приблизительно   расширение  (значение,  состоящее
	// собственно из значения,  полученного  в результате арифметической
	// операции над числами с плавающей точкой, и его ошибки округления).
	double Estimate(int elen, double* e);

	// Суммирует  два  расширения  (значения,  каждое из которых  состоит из значения
	// результата арифметической операции над числами с плавающей точкой и его ошибки
	// округления). Исключает нулевые компоненты из выходного расширения.
	int FastExpansionSumZeroElim(int elen, double* e, int flen, double* f, double* h);

	// Умножает расширение (значение, состоящее собственно из значения, полученного
	// в результате арифметической операции над числами  с плавающей точкой,  и его
	// ошибки  округления)  на скаляр  и исключает  нулевые компоненты из выходного 
	// расширения.
	int ScaleExpansionZeroElim(int elen, double* e, double b, double* h);

	// Выполняет  адаптивную   проверку   на  направление  следования   точек
	// в  треугольнике  против часовой стрелки. Вычисляет  последовательность
	// точных приближений  к определителю и возвращает результат тогда, когда
	// точность знака определителя гарантирована.
	double CounterClockwiseAdapt(Vertex pa, Vertex pb, Vertex pc, double detsum);

	// Выполняет проверку следования вершин треугольника в порядке против
	// часовой стрелки.  Возвращает минор (определитель матрицы 2*2) знак
	// которого  показывает  результат проверки:  положительный - вершины
	// в  треугольнике  следуют  друг за другом  против  часовой стрелки,
	// отрицательный - вершины  в треугольнике  следуют дуг за другом  по
	// часовой стрелке. И, если минор равен нулю, то вершины коллинеарны.
	double CounterClockwise(Mesh* m, Configuration* b, Vertex pa, Vertex pb, Vertex pc);

	// Освобождает память, ранее выделенную под треугольник,
	// и помечает этот треугольник как "мёртвый".
	void TriangleDealloc(Mesh* m, Triangle* dyingtriangle);

	// Выполняет адаптивную проверку  на попадание точки в окружность.
	// Вычисляет последовательность точных приближений  к определителю
	// и возвращает результат тогда, когда точность знака определителя
	// гарантирована.
	double InCircleAdapt(Vertex pa, Vertex pb, Vertex pc, Vertex pd, double permanent);

	// Выполняет проверку на попадание точки в окружность,  описанную вокруг
	// треугольника.  Возвращает  определитель матрицы уравнения окружности,
	// проходящей  через вершины  треугольника. Знак определителя показывает
	// результат проверки: положительный - точка pd лежит внутри окружности,
	// отрицательный - точка pd  лежит вне окружности.  И, если определитель
	// равен нулю,  то точка pd принадлежит окружности  (окружность проходит
	// через точки pa, pb, pc и pd).
	double InCircle(Mesh* m, Configuration* b, Vertex pa, Vertex pb, Vertex pc, Vertex pd);
}

// Данное пространство имён содержит вспомогательные функции общего характера,
// не относящиеся именно к триангуляции Делоне.
namespace UsualHelper
{
	// Определяет, можно ли читать память, на которую ссылается
	// передаваемый (в качестве аргумента) указатель на void.
	bool IsBadReadPointer(void* checkedPointer);

	// Генерирует случайное число.
	unsigned long Randomnation(unsigned int choices);

	// Выделяет память.
	void* TriMalloc(int size);

	// Освобождает указанную область выделенной памяти.
	void TriFree(void* memptr);

	// Выполняет выход из приложения.
	void TriExit(int status);

	// Возвращает текст сообщения об ошибке, код которой было получен
    // вызовом GetLastError(), или пустую строку, если не было ошибки.
	std::wstring GetLastErrorMessage(DWORD errorMessageID);
}
