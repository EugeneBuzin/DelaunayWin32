#pragma once

#include "DelaunayHelper.h"

using namespace concurrency;
using namespace DelaunayHelper;
using namespace DelaunayHelper::Triangles;
using namespace UsualHelper;

// Выполняет запись в файлы результатов триангуляции Делоне.
class DelaunayWriteToFile
{
public:
	DelaunayWriteToFile();
	~DelaunayWriteToFile();

	// Записывает отсортированные при триангуляции вершины в выходной файл *.node.
	void WriteNodes(HWND hWnd, Mesh* m, Configuration* b, wstring nodeFileName, cancellation_token token);
	// Записывает полученные при триангуляции треугольники в выходной файл *.ele.
	void WriteElements(HWND hWnd, Mesh* m, Configuration* b, wstring eleFileName, cancellation_token token);
	// Записывает рёбра полученных при триангуляции треугольников в выходной файл *.edge.
	void WriteEdges(HWND hWnd, Mesh* m, Configuration* b, wstring edgeFileName, cancellation_token token);
	// Записывает треугольников-соседей для каждого треугольника в триангуляционной сетке
	// в выходной файл *.neigh.
	void WriteNeighbors(HWND hWnd, Mesh* m, Configuration* b, wstring neighborFileName, cancellation_token token);
private:
	// Указатель на текст информационного сообщения.
	///*const*/ wchar_t* pInfoMessage;
	// Проверяет файл на существование.
	bool fileExists(const wchar_t* fileName);
	// Завершает запись в файл.
	void finishFile(FILE* outFile, wstring fileName, std::wstring nameOfRecordable);
};

