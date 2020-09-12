#include "stdafx.h"
#include <sys/stat.h>

#include <cstring>  // for wcscpy_s, wcscat_s
#include <cstdlib>  // for _countof


#include <errno.h>
#include "DelaunayWriteToFile.h"

#define UNDEADVERTEX -32767

DelaunayWriteToFile::DelaunayWriteToFile()
{
}

DelaunayWriteToFile::~DelaunayWriteToFile()
{
	//delete[] pInfoMessage;
}

// Записывает отсортированные при триангуляции вершины в выходной файл *.node.
void DelaunayWriteToFile::WriteNodes(HWND hWnd, Mesh* m, Configuration* b, wstring nodeFileName, cancellation_token token)
{
	// Проверить, была ли отмена задачи.
	if (token.is_canceled())
	{
		cancel_current_task();
	}
	else
	{
		long outvertices = m->Vertices.CurrentlyAllocatedItems - m->UnshownVertices;

		const wchar_t* outNodeFileName = nodeFileName.c_str();
		// Удалить используемый выходной файл, если он существует.
		if (fileExists(outNodeFileName))
		{
			_wremove(outNodeFileName);
		}
		// Создать и открыть выходной файл *.node, в который будет производится запись вершин.
		FILE* outfile = _wfopen(outNodeFileName, L"w");
		if (outfile == nullptr)
		{
			int err = errno;
			wstring errMsgStr = L"Ошибка:  Невозможно открыть файл '" + nodeFileName
				+ L"' для записи вершин триангуляционной сетки. Приложение будет закрыто.";
			LPCWSTR errMsg = errMsgStr.c_str();
			MessageBox(hWnd, errMsg, L"Триангулятор", MB_ICONERROR | MB_OK);
			TriExit(1);
		}

		wstring message = L"Запись узлов триангуляционной сетки в файл \"" + nodeFileName + L"\"";
		wchar_t* pInfoMessage = _wcsdup(message.c_str());
		PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pInfoMessage, 0L);
		//free(pInfoMessage);
        // Передать количество входных вершин в главное окно для установки индикатора выполнения в исходное состояние.
		const int RANGE = 100;
		int vertices = outvertices;
		int messageCounter = 1;
		if (outvertices > RANGE)
		{
			messageCounter = outvertices / RANGE;
			vertices /= messageCounter;
		}
		PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)vertices);
		int count = 0;
		int i = 0;

		// Количество вершин, количество измерений, количество атрибутов вершины
		// и количество граничных маркеров (ноль или единица).
		fwprintf(outfile, L"%ld  %d  %d  %d\n", outvertices, m->MeshDimension, m->AttributesPerVertex, 1 - b->nobound);

		TraversalInit(&m->Vertices);
		int vertexnumber = b->firstnumber;
		Vertex vertexloop = VertexTraverse(m);

		while (vertexloop != nullptr)
		{
			// Проверить, была ли отмена задачи.
			if (token.is_canceled())
			{
				const wchar_t* pMsgTriIsCancelling = L"Пользователь отменяет триангуляцию...";
				PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pMsgTriIsCancelling, 0L);
				// Завершить работу с файлом по окончанию записи.
				finishFile(outfile, nodeFileName, L"вершин");
				// Подтвердить отмену задачи.
				cancel_current_task();
			}
			else
			{
				if ((GetVertexType(vertexloop, m->VertexMarkIndex) != UNDEADVERTEX))
				{
					// Номер вершины и её координаты X и Y.
					fwprintf(outfile, L"%4d    %.17g  %.17g", vertexnumber, vertexloop[0], vertexloop[1]);
					// Записать аттрибут (если есть).
					for (int i = 0; i < m->AttributesPerVertex; i++)
						fwprintf(outfile, L"  %.17g", vertexloop[i + 2]);
					// Если у вершины нет граничного маркера, то конец строки.
					// Иначе, записать граничный маркер.
					if (b->nobound)
						fwprintf(outfile, L"\n");
					else
						fwprintf(outfile, L"    %d\n", GetVertexMark(vertexloop, m->VertexMarkIndex));

					SetVertexMark(vertexloop, vertexnumber, m->VertexMarkIndex);
					vertexnumber++;
				}
				vertexloop = VertexTraverse(m);

                // Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
				if (i % messageCounter == 0 && count < vertices)
				{
					PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
					count++;
				}
				i++;
			}
		}
		// Завершить работу с файлом по окончанию записи.
		finishFile(outfile, nodeFileName, L"");
		//free(pInfoMessage);
	}
}

// Записывает полученные при триангуляции треугольники в выходной файл *.ele.
void DelaunayWriteToFile::WriteElements(HWND hWnd, Mesh * m, Configuration * b, wstring eleFileName, cancellation_token token)
{
	// Проверить, была ли отмена задачи.
	if (token.is_canceled())
	{
		cancel_current_task();
	}
	else
	{
		const wchar_t* outEleFileName = eleFileName.c_str();

		// Удалить используемый выходной файл, если он существует.
		if (fileExists(outEleFileName))
		{
			_wremove(outEleFileName);
		}
		// Создать и открыть выходной файл *.ele, в который будет производится запись вершин.
		FILE* outFile = _wfopen(outEleFileName, L"w");
		if (outFile == nullptr)
		{
			wstring errMsgStr = L"Ошибка:  Невозможно открыть файл '" + eleFileName
				+ L"' для записи треугольников, составляющих триангуляционную сетку. Приложение будет закрыто.";
			LPCWSTR errMsg = errMsgStr.c_str();
			MessageBox(hWnd, errMsg, L"Триангулятор", MB_ICONERROR | MB_OK);
			TriExit(1);
		}

		wstring message = L"Запись сформированных треугольников в файл \"" + eleFileName + L"\"";
		wchar_t* pInfoMessage = _wcsdup(message.c_str());
		PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pInfoMessage, 0L);
		//free(pInfoMessage);
        // Передать количество входных вершин в главное окно для установки индикатора выполнения в исходное состояние.
		const int RANGE = 100;
		int vertices = m->Triangles.CurrentlyAllocatedItems;
		int messageCounter = 1;
		if (m->Triangles.CurrentlyAllocatedItems > RANGE)
		{
			messageCounter = m->Triangles.CurrentlyAllocatedItems / RANGE;
			vertices /= messageCounter;
		}
		PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)vertices/*(LPARAM)RANGE)*/);
		int count = 0;
		int i = 0;

		// Записать в файл: количество треугольников, количество вершин в каждом треугольнике
		// и количество атрибутов каждого треугольника.
		fwprintf(outFile, L"%ld  %d  %d\n", m->Triangles.CurrentlyAllocatedItems, (b->order + 1) * (b->order + 2) / 2, m->AttributesPerTriangle);

		TraversalInit(&m->Triangles);
		OTriangle triangleloop;
		triangleloop.Tri = TriangleTraverse(&m->Triangles);
		triangleloop.Orient = 0;
		long elementnumber = b->firstnumber;
		Vertex p1, p2, p3, mid1, mid2, mid3;
		while (triangleloop.Tri != nullptr)
		{
			// Проверить, была ли отмена задачи.
			if (token.is_canceled())
			{
				// Завершить работу с файлом по окончанию записи.
				finishFile(outFile, eleFileName, L"треугольников");
				// Подтвердить отмену задачи.
				cancel_current_task();
			}
			else
			{
				p1 = GetOrignVertex(triangleloop);
				p2 = GetDestinationVertex(triangleloop);
				p3 = GetApex(triangleloop);
				// Если не создаются субпараметрические элементы второго порядка, то записать
				// в файл номер треугольника и индексы его трёх вершин.
				if (b->order == 1)
					fwprintf(outFile, L"%4ld    %4d  %4d  %4d", elementnumber, GetVertexMark(p1, m->VertexMarkIndex),
						GetVertexMark(p2, m->VertexMarkIndex), GetVertexMark(p3, m->VertexMarkIndex));
				else
				{
					mid1 = (Vertex)triangleloop.Tri[m->HighOrderIndex + 1];
					mid2 = (Vertex)triangleloop.Tri[m->HighOrderIndex + 2];
					mid3 = (Vertex)triangleloop.Tri[m->HighOrderIndex];
					// Номер треугольника и индексы для шести вершин.
					fwprintf(outFile, L"%4ld    %4d  %4d  %4d  %4d  %4d  %4d", elementnumber,
						GetVertexMark(p1, m->VertexMarkIndex), GetVertexMark(p2, m->VertexMarkIndex),
						GetVertexMark(p3, m->VertexMarkIndex), GetVertexMark(mid1, m->VertexMarkIndex),
						GetVertexMark(mid2, m->VertexMarkIndex), GetVertexMark(mid3, m->VertexMarkIndex));
				}

				for (int i = 0; i < m->AttributesPerTriangle; i++)
					fwprintf(outFile, L"  %.17g", GetTriAttribute(triangleloop, i, m->ElemAttribIndex));

				fwprintf(outFile, L"\n");

				triangleloop.Tri = TriangleTraverse(&m->Triangles);
				elementnumber++;

                // Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
				if (i % messageCounter == 0 && count < vertices)
				{
					PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
					count++;
				}
				i++;
			}
		}
		// Завершить работу с файлом по окончанию записи.
		finishFile(outFile, eleFileName, L"");
		//free(pInfoMessage);
	}
}

// Записывает рёбра полученных при триангуляции треугольников в выходной файл *.edge.
void DelaunayWriteToFile::WriteEdges(HWND hWnd, Mesh* m, Configuration* b, wstring edgeFileName, cancellation_token token)
{
	// Проверить, была ли отмена задачи.
	if (token.is_canceled())
	{
		cancel_current_task();
	}
	else
	{
		const wchar_t* outEdgeFileName = edgeFileName.c_str();

		// Удалить используемый выходной файл, если он существует.
		if (fileExists(outEdgeFileName))
		{
			_wremove(outEdgeFileName);
		}

		FILE* outFile = _wfopen(outEdgeFileName, L"w");
		if (outFile == nullptr)
		{
			wstring errMsgStr = L"Ошибка:  Невозможно открыть файл '" + edgeFileName
				+ L"' для записи рёбер триангуляционной сетки. Приложение будет закрыто.";
			LPCWSTR errMsg = errMsgStr.c_str();
			MessageBox(hWnd, errMsg, L"Триангулятор", MB_ICONERROR | MB_OK);
			TriExit(1);
		}

		wstring message = L"Запись рёбер сформированных треугольников в файл \"" + edgeFileName + L"\"";
		wchar_t* pInfoMessage = _wcsdup(message.c_str());
		PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pInfoMessage, 0L);
		//free(pInfoMessage);
        // Передать количество входных вершин в главное окно для установки индикатора выполнения в исходное состояние.
		const int RANGE = 100;
		int vertices = m->Edges;
		int messageCounter = 1;
		if (m->Edges > RANGE)
		{
			messageCounter = m->Edges / RANGE;
			vertices /= messageCounter;
		}
		PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)vertices);
		int count = 0;
		int i = 0;

		// Количество рёбер и количество граничных маркеров (ноль или единица).
		fwprintf(outFile, L"%ld  %d\n", m->Edges, 1 - b->nobound);

		TraversalInit(&m->Triangles);

		OTriangle triangleloop;
		triangleloop.Tri = TriangleTraverse(&m->Triangles);
		long edgenumber = b->firstnumber;

		OTriangle trisym;
		// Начальная вершина (точка) ребра.
		Vertex p1;
		// Конечная вершина (точка) ребра.
		Vertex p2;
		// Чтобы  перебрать  множество ребер, нужно  перебрать  все треугольники  и посмотреть
		// на три ребра каждого треугольника. Если нет другого треугольника, смежного с ребром,
		// то оперировать ребром. Если есть другой смежный треугольник, то  оперировать ребром,
		// только если текущий треугольник имеет меньший указатель, чем его сосед.Таким образом,
		// каждое ребро рассматривается только один раз.
		while (triangleloop.Tri != nullptr)
		{
			// Проверить, была ли отмена задачи.
			if (token.is_canceled())
			{
				// Завершить работу с файлом по окончанию записи.
				finishFile(outFile, edgeFileName, L"рёбер треугольников");
				// Подтвердить отмену задачи.
				cancel_current_task();
			}
			else
			{
				for (triangleloop.Orient = 0; triangleloop.Orient < 3; triangleloop.Orient++)
				{
					FindAbutingTriOnSameEdge(triangleloop, trisym);
					if ((triangleloop.Tri < trisym.Tri) || (trisym.Tri == m->DummyTri))
					{
						p1 = GetOrignVertex(triangleloop);
						//BEGIN DEBUG:
						// Координаты точки начала ребра.
						//double* dblPtr = ((double*)p1);
						//double x1 = *dblPtr;
						//dblPtr = dblPtr + 1;
						//double y1 = *dblPtr;
						//END DEBUG.
						p2 = GetDestinationVertex(triangleloop);
						//BEGIN DEBUG:
						// Координаты точки конца ребра.
						//dblPtr = ((double*)p2);
						//double x2 = *dblPtr;
						//dblPtr = dblPtr + 1;
						//double y2 = *dblPtr;
						//END DEBUG.

						if (b->nobound)
						{
							//BEGIN DEBUG:
							//int i1 = GET_VERTEX_MARK(p1, m->VertexMarkIndex);
							//int i2 = GET_VERTEX_MARK(p2, m->VertexMarkIndex);
							//END DEBUG.
							// Номер ребра и индексы двух конечных точек ребра.
							fwprintf(outFile, L"%4ld   %d  %d\n", edgenumber, GetVertexMark(p1, m->VertexMarkIndex),
								GetVertexMark(p2, m->VertexMarkIndex));
						}
						else
						{
							// Номер ребра, индексы двух граничных точек ребра и граничный маркер.
							fwprintf(outFile, L"%4ld   %d  %d  %d\n", edgenumber, GetVertexMark(p1, m->VertexMarkIndex),
								GetVertexMark(p2, m->VertexMarkIndex), trisym.Tri == m->DummyTri);
						}
						edgenumber++;
					}
				}
				triangleloop.Tri = TriangleTraverse(&m->Triangles);

                // Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
				if (i % messageCounter == 0 && count < vertices)
				{
					PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
					count++;
				}
				i++;
			}
		}
		// Завершить работу с файлом по окончанию записи.
		finishFile(outFile, edgeFileName, L"");
		//free(pInfoMessage);
	}
}

// Записывает треугольников-соседей для каждого треугольника в триангуляционной сетке
// в выходной файл *.neigh.
void DelaunayWriteToFile::WriteNeighbors(HWND hWnd, Mesh* m, Configuration* b, wstring neighborFileName, cancellation_token token)
{
	// Проверить, была ли отмена задачи.
	if (token.is_canceled())
	{
		cancel_current_task();
	}
	else
	{
		const wchar_t* outNeighborFileName = neighborFileName.c_str();

		// Удалить используемый выходной файл, если он существует.
		if (fileExists(outNeighborFileName))
		{
			_wremove(outNeighborFileName);
		}
		FILE* outFile = _wfopen(outNeighborFileName, L"w");
		if (outFile == nullptr)
		{
			wstring errMsgStr = L"Ошибка:  Невозможно открыть файл '" + neighborFileName
				+ L"' для записи треугольников-соседей. Приложение будет закрыто.";
			LPCWSTR errMsg = errMsgStr.c_str();
			MessageBox(hWnd, errMsg, L"Триангулятор", MB_ICONERROR | MB_OK);
			TriExit(1);
		}

		wstring message = L"Запись треугольников-соседей в файл \"" + neighborFileName + L"\"";
		wchar_t* pInfoMessage = _wcsdup(message.c_str());
		PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pInfoMessage, 0L);
		//free(pInfoMessage);
        // Передать количество входных вершин в главное окно для установки индикатора выполнения в исходное состояние.
		const int RANGE = 100;
		int vertices = m->Triangles.CurrentlyAllocatedItems;
		int messageCounter = 1;
		if (m->Triangles.CurrentlyAllocatedItems > RANGE)
		{
			messageCounter = m->Triangles.CurrentlyAllocatedItems / RANGE;
			vertices /= messageCounter;
		}
		PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)vertices);
		int count = 0;
		int i = 0;

		// Количество треугольников, у каждого треугольника - три соседа.
		fwprintf(outFile, L"%ld  %d\n", m->Triangles.CurrentlyAllocatedItems, 3);

		TraversalInit(&m->Triangles);
		OTriangle triangleloop;
		triangleloop.Tri = TriangleTraverse(/*m*/&m->Triangles);
		triangleloop.Orient = 0;
		long elementnumber = b->firstnumber;

		while (triangleloop.Tri != nullptr)
		{
			// Проверить, была ли отмена задачи.
			if (token.is_canceled())
			{
				cancel_current_task();
			}
			else
			{
				*(int *)(triangleloop.Tri + 6) = (int)elementnumber;
				triangleloop.Tri = TriangleTraverse(&m->Triangles);
				elementnumber++;

                // Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
				if (i % messageCounter == 0 && count < vertices)
				{
					PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
					count++;
				}
				i++;
			}
		}
		*(int *)(m->DummyTri + 6) = -1;

		TraversalInit(&m->Triangles);
		triangleloop.Tri = TriangleTraverse(/*m*/&m->Triangles);
		elementnumber = b->firstnumber;

		OTriangle trisym;
		int neighbor1, neighbor2, neighbor3;

		PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)vertices);
		count = 0; i = 0;

		while (triangleloop.Tri != nullptr)
		{
			// Проверить, была ли отмена задачи.
			if (token.is_canceled())
			{
				// Завершить работу с файлом по окончанию записи.
				finishFile(outFile, neighborFileName, L"треугольников-соседей");
				// Подтвердить отмену задачи.
				cancel_current_task();
			}
			else
			{
				triangleloop.Orient = 1;
				FindAbutingTriOnSameEdge(triangleloop, trisym);
				neighbor1 = *(int *)(trisym.Tri + 6);
				triangleloop.Orient = 2;
				FindAbutingTriOnSameEdge(triangleloop, trisym);
				neighbor2 = *(int *)(trisym.Tri + 6);
				triangleloop.Orient = 0;
				FindAbutingTriOnSameEdge(triangleloop, trisym);
				neighbor3 = *(int *)(trisym.Tri + 6);
				// Номер треугольника и номера трёх треугольников-соседей.
				fwprintf(outFile, L"%4ld    %d  %d  %d\n", elementnumber, neighbor1, neighbor2, neighbor3);

				triangleloop.Tri = TriangleTraverse(/*m*/&m->Triangles);
				elementnumber++;

                // Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
				if (i % messageCounter == 0 && count < vertices)
				{
					PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
					count++;
				}
				i++;
			}
		}
		finishFile(outFile, neighborFileName, L"");
		//free(pInfoMessage);
	}
}

// Проверяет файл на его существование.
bool DelaunayWriteToFile::fileExists(const wchar_t * fileName)
{
	struct _stat buf;
	if (_wstat(fileName, &buf) != -1)
		return true;

	return false;
}

// Завершает работу с файлом по окончанию записи.
void DelaunayWriteToFile::finishFile(FILE* outFile, wstring fileName, std::wstring nameOfRecordable)
{
	fclose(outFile);
}

