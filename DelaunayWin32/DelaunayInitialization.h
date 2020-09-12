#pragma once

#include "DelaunayHelper.h"

using namespace concurrency;
using namespace DelaunayStructures;
using namespace DelaunayHelper::Triangles;
using namespace UsualHelper;

// Выполняет инициализацию объектов, используемых в триангуляции Делоне.
class DelaunayInitialization
{
public:
	DelaunayInitialization();
	~DelaunayInitialization();
	// Выполняет инициализацию переменных, используемых в триангуляции.
	void TriInit(wstring inFilePath, wstring outFolderPath, Mesh* m, Configuration* b);
	// Выполняет чтение исходного облака точек из файла *.node.
	void ReadNodes(HWND hWnd, Mesh* m, Configuration* b, wstring nodefilename, cancellation_token token);
	// Рассчитывает размеры структур данных для треугольника и подсегмента
	// и инициализирует их пулы памяти.
	void InitializeTriSubpools(struct Mesh* m, struct Configuration* b);
	int prevPercent;
private:
	// Указатель на текст информационного сообщения.
	///*const*/ wchar_t* pInfoMessage;
	// Обнуляет пул триангуляции.
	void setPoolToZero(struct MemoryPool* pool);
	// Устанавливает рабочие параметры для приложения.
	void setOperatingParameters(wstring inFilePath, wstring outFolderPath, Configuration* b);
	// Читает из файла строку, содержащую данные.
	wchar_t* readLine(wchar_t* string, FILE* infile, const wchar_t* infilename);
	// Ищет следующее поле в строке.
	wchar_t* findField(wchar_t* string);
	// Подсчитывает размер структуры данных вершины и инициализирует требуемый для неё объём памяти.
	void initializeVertexPool(struct Mesh* m, struct Configuration* b);
	// Инициализирует треугольник, который заполняет «внешнее пространство» и вездесущий подсегмент.
	void dummyInit(struct Mesh* m, struct Configuration* b, int trianglebytes, int subsegbytes);
	// Инициализирует пул памяти для размещения в ней элементов.
	void poolInit(struct MemoryPool* pool, int bytecount, int itemcount, int firstitemcount, int alignment);
	// Забирает память  у элементов, под которые  она была выделена  ранее,
	// но не возвращает её операционной системе, а делает блоки этой памяти
	// готовыми к повторному использованию.
	void poolRestart(struct MemoryPool* pool);
};

