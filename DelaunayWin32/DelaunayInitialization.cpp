#include "stdafx.h"
#include <filesystem>
#include <fstream>
//#include <locale>
//#include <codecvt>
#include "DelaunayInitialization.h"

// Минимальное значение координаты X для исходных точек, по которым строится триангуляция.
extern double xMin;
// Максимальное значение координаты X для исходных точек, по которым строится триангуляция.
extern double xMax;
// Максимальное значение координаты Y для исходных точек, по которым строится триангуляция. 
extern double yMin;
// Максимальное значение координаты Y для исходных точек, по которым строится триангуляция.
extern double yMax;

// Максимальное количество символов в читаемой из файла строке (включая null).
#define INPUTLINESIZE 1024
// Количество вершин, память под которое может быть выделена за один раз.
#define VERTEXPERBLOCK 4092
// Количество треугольников, память под которое может быть выделена за один раз.
#define TRIPERBLOCK 4092
// Количество подсегментов, память под которое может быть выделена за один раз.
#define SUBSEGPERBLOCK 508
// Определяет, что тип вершин - входные вершины.
#define INPUTVERTEX 0


DelaunayInitialization::DelaunayInitialization()
{
}


DelaunayInitialization::~DelaunayInitialization()
{
	//delete[] pInfoMessage;
}

// Выполняет инициализацию переменных, используемых в триангуляции.
void DelaunayInitialization::TriInit(wstring inFilePath, wstring outFolderPath, Mesh* m, Configuration* b)
{
	// Обнулить пул триангуляции.
	setPoolToZero(&m->Vertices);
	setPoolToZero(&m->Triangles);

	// Не была исключена ни одна вершина.
	m->UnshownVertices = 0;

	// Установить рабочие параметры для программы.
	setOperatingParameters(inFilePath, outFolderPath, b);

	// Инициализировать вспомогательные переменные.
	DelaunayHelper::DelaunayHelperInit();
}

// Выполняет чтение исходного облака точек из файла *.node.
void DelaunayInitialization::ReadNodes(HWND hWnd, Mesh* m, Configuration* b, wstring nodefilename, cancellation_token token)
{
	// Проверить, была ли отмена задачи.
	if (token.is_canceled())
	{
		cancel_current_task();
	}
	else
	{
		// Файл, содержащий входные данные.
		FILE* infile = nullptr;
		// Строка, читаемая из файла.
		wchar_t inputLine[INPUTLINESIZE];
		// Маркеры узла.
		int nodemarkers;
		// Буфер для строки, читаемой из файла входных данных.
		wchar_t* stringptr;

		m->ReadNodeFile = 1;
		wstring infilename = nodefilename;

		if (m->ReadNodeFile)
		{
			// Читать вершины из файла *.node.
			// Открыть файл облака точек для чтения.
			infile = _wfopen(nodefilename.c_str(), L"r");
			// Если не удалось, то сообщить об ошибке и завершить работу приложения.
			if (infile == nullptr)
			{
				wstring errMsgStr = L"Ошибка:  Файл '" + nodefilename + L"' недоступен. Приложение будет закрыто";
				LPCWSTR errMsg = errMsgStr.c_str();
				MessageBox(hWnd, errMsg, L"Триангулятор", MB_ICONERROR | MB_OK);
				TriExit(1);
			}
			// Читать количество вершин, измерений, атрибутов вершин и граничных маркеров:
			stringptr = readLine(inputLine, infile, nodefilename.c_str());
			// Получить количество вершин.
			m->InVertices = (int)wcstol(stringptr, &stringptr, 0);
			// Получить количество измерений триангуляционной сетки.
			stringptr = findField(stringptr);
			if (*stringptr == L'\0')
				m->MeshDimension = 2;
			else
				m->MeshDimension = (int)wcstol(stringptr, &stringptr, 0);

			// Получить количество атрибутов вершин.
			stringptr = findField(stringptr);
			if (*stringptr == L'\0')
				m->AttributesPerVertex = 0;
			else
				m->AttributesPerVertex = (int)wcstol(stringptr, &stringptr, 0);
			// Получить количество граничных маркеров.
			stringptr = findField(stringptr);
			if (*stringptr == L'\0')
				nodemarkers = 0;
			else
				nodemarkers = (int)wcstol(stringptr, &stringptr, 0);
		}

		if (m->InVertices < 3)
		{
			wstring errMsgStr = L"Ошибка:  Должно быть не менее трёх входных вершин.";
			LPCWSTR errMsg = errMsgStr.c_str();
			MessageBox(hWnd, errMsg, L"Триангулятор", MB_ICONERROR | MB_OK);
			TriExit(1);
		}
		if (m->MeshDimension != 2)
		{
			wstring errMsgStr = L"Ошибка:  Приложение работает только с двухмерными триангуляционными сетками. Приложение будет закрыто";
			LPCWSTR errMsg = errMsgStr.c_str();
			MessageBox(hWnd, errMsg, L"Триангулятор", MB_ICONERROR | MB_OK);
			TriExit(1);
		}

		initializeVertexPool(m, b);

		wstring message = L"Чтение исходных данных для триангуляции";
		wchar_t* pInfoMessage = _wcsdup(message.c_str());
		PostMessage(hWnd, WM_WRITESTATUSTEXT, (WPARAM)pInfoMessage, 0L);
		//free(pInfoMessage);
		// Передать количество входных вершин в главное окно для установки индикатора выполнения в исходное состояние.
		const int RANGE = 100;
		int vertices = m->InVertices;
		int messageCounter = 1;
		if (m->InVertices > RANGE)
		{
			messageCounter = m->InVertices / RANGE;
			vertices /= messageCounter;
		}
		PostMessage(hWnd, WM_RESETPROGRESS, 0, (LPARAM)vertices/*(LPARAM)RANGE)*/);

		prevPercent = -1;
		int count = 0;
		// Читать вершины.
		for (int i = 0; i < m->InVertices; i++)
		{
			// Проверить, была ли отмена задачи.
			if (token.is_canceled())
			{
				cancel_current_task();
			}
			else
			{
				// Создать буфер для данных очередной вершины.
				Vertex vertexloop = (Vertex)DelaunayHelper::PoolAlloc(&m->Vertices);
				// Получить описание очередной вершины.
				stringptr = readLine(inputLine, infile, infilename.c_str());
				// Если это первая вершина, то получить её номер и, на основании его, значения.
				// Определить, как, в исходном файле, нумеруются вершины - с нуля (0) или с единицы (1):
				if (i == 0)
				{
					int firstnode = (int)wcstol(stringptr, &stringptr, 0);
					if ((firstnode == 0) || (firstnode == 1))
						b->firstnumber = firstnode;
				}
				// Получить координату X очередной вершины.
				stringptr = findField(stringptr);
				if (*stringptr == L'\0')
				{
					wstring errMsg = L"Ошибка:  Вершина " + std::to_wstring(b->firstnumber + i) +
						L" не имеет координаты X. Приложение будет закрыто.";
					MessageBox(hWnd, errMsg.c_str(), L"Триангулятор", MB_ICONERROR | MB_OK);
					TriExit(1);
				}
				double x = (double)wcstod(stringptr, &stringptr);
				// Получить координату Y очередной вершины
				stringptr = findField(stringptr);
				if (*stringptr == L'\0')
				{
					wstring errMsg = L"Ошибка:  Вершина " + std::to_wstring(b->firstnumber + i) +
						L" не имеет координаты Y. Приложение будет закрыто.";
					MessageBox(hWnd, errMsg.c_str(), L"Триангулятор", MB_ICONERROR | MB_OK);
					TriExit(1);
				}
				double y = (double)wcstod(stringptr, &stringptr);
				vertexloop[0] = x;
				vertexloop[1] = y;
				// Читать атрибуты вершины.
				for (int j = 2; j < 2 + m->AttributesPerVertex; j++)
				{
					stringptr = findField(stringptr);
					if (*stringptr == L'\0')
						vertexloop[j] = 0.0;
					else
						vertexloop[j] = (double)wcstod(stringptr, &stringptr);
				}
				// Читать граничный маркер вершины, если в входном файле было определено их количество.
				if (nodemarkers)
				{
					// Получить маркер.
					stringptr = findField(stringptr);
					// Если он не был получен, то установить вместо него значение ноль (0) для вершины.
					// Иначе, установить значение маркера для вершины.
					if (*stringptr == L'\0')
						SetVertexMark(vertexloop, 0, m->VertexMarkIndex);
					else
					{
						int currentmarker = (int)wcstol(stringptr, &stringptr, 0);
						SetVertexMark(vertexloop, currentmarker, m->VertexMarkIndex);
					}
				}
				else
				{
					// Если в входном файле не было определено количество граничных маркеров,
					// то установить ноль, являющийся значением по умолчанию.
					SetVertexMark(vertexloop, 0, m->VertexMarkIndex);
				}
				// Т.к. вершина читается из файла облака точек (*.node),
				// то установить её тип как "входная вершина".
				SetVertexType(vertexloop, INPUTVERTEX, m->VertexMarkIndex);
				// Определить наименьшее и наибольшее значения для координат x и y.
				if (i == 0)
				{
					// Если это первая вершина, то её не с чем сранивать,
					// т.к. информации по остальным вершинам пока нет, и
					// соответственно наменьшее и наибольшее значение для
					// координат точек в их облаке просто устанавливаются
					// равными её координатам.
					m->X_Min = m->X_Max = x;
					m->Y_Min = m->Y_Max = y;
				}
				else
				{
					// Иначе, если это уже одна из последующих вершин,
					// то тогда наименьшее и наибольшее значения для
					// координат устанавливаются путём сравнения с
					// предыдущими наименьшим и наибольшим значениями.
					xMin = m->X_Min = (x < m->X_Min) ? x : m->X_Min;
					xMax = m->X_Max = (x > m->X_Max) ? x : m->X_Max;
					yMin = m->Y_Min = (y < m->Y_Min) ? y : m->Y_Min;
					yMax = m->Y_Max = (y > m->Y_Max) ? y : m->Y_Max;
				}
				// Передать, в главное окно приложения, сообщение о том, что следует изменить значение индикатора выполнения.
				if (i % messageCounter == 0 && count < vertices)
				{
					PostMessage(hWnd, WM_UPDATEPROGRESS, 0, 0);
					count++;
				}

				//int percent = i * RANGE / vertices;
				//if (percent != prevPercent)
				//{
					//PostMessage(hWnd, WM_UPDATEPROGRESS, (WPARAM)percent, 0);
					//prevPercent = percent;
					//count++;
				//}
			}
		}
		// Закрыть файл после того, как он был весь прочитан.
		if (m->ReadNodeFile)
			fclose(infile);
		free(pInfoMessage);
	}
}

// Рассчитывает размеры структур данных для треугольника и подсегмента
// и инициализирует их пулы памяти.
void DelaunayInitialization::InitializeTriSubpools(Mesh* m, Configuration* b)
{
	int trisize;
	const int HIGH_ORDER_INDEX = 6;

	// Индекс  в каждом треугольнике,  в котором находятся дополнительные узлы (более трех),
	// связанные  с  элементами высшего порядка.  Есть три указателя на другие треугольники,
	// и три указателя на углы.
	m->HighOrderIndex = HIGH_ORDER_INDEX;
	// Количество байт, занимаемое треугольником.
	trisize = ((b->order + 1) * (b->order + 2) / 2 + (m->HighOrderIndex - 3)) * sizeof(Triangle);
	// Индекс в каждом треугольнике, в котором находятся его атрибуты,
	// где индекс измеряется числами double.
	m->ElemAttribIndex = (trisize + sizeof(double) - 1) / sizeof(double);
	// Индекс в каждом треугольнике, в котором найдено максимальное ограничение площади,
	// где индекс измеряется в значениях double. Обратите внимание, что если установлен
	// флаг 'regionattrib', то будет добавлен дополнительный атрибут.
	m->AreaBoundIndex = m->ElemAttribIndex + m->AttributesPerTriangle;
	// Если необходимы атрибуты треугольника или граница области, то увеличить
	// количество байт, занимаемых треугольником.
	trisize = (m->AreaBoundIndex + 1) * sizeof(double);
	// Если  запрашивается  диаграмма Вороного  или  граф  соседей треугольника,
	// то убедиться,  что в каждом треугольнике есть место  для хранения индекса 
	// выраженного целым числом.  Этот целочисленный индекс может занимать то же
	// пространство, что и указатели на подсегменты или атрибуты или ограничение
	// области или дополнительные узлы.
	if ((/*b->voronoi ||*/ b->neighbors) && (trisize < 6 * sizeof(Triangle) + sizeof(int)))
		trisize = 6 * sizeof(Triangle) + sizeof(int);

	// Определив объем памяти треугольника, инициализировать эту память.
	poolInit(&m->Triangles, trisize, TRIPERBLOCK, (2 * m->InVertices - 2) > TRIPERBLOCK ? (2 * m->InVertices - 2) :
		TRIPERBLOCK, 4);
	// Инициализировать треугольник «внешнего пространства».
	dummyInit(m, b, m->Triangles.ItemBytes, 0);
}

// Обнуляет пул триангуляции.
void DelaunayInitialization::setPoolToZero(MemoryPool* pool)
{
	pool->FirstBlock = nullptr;
	pool->CurrentAllocationBlock = nullptr;
	pool->NextFreeMemorySlice = nullptr;
	pool->DeadElementsStack = nullptr;
	pool->CurrentTriangulatedBlock = nullptr;
	pool->NextItemForTriangulation = nullptr;
	pool->AlignBytes = 0;
	pool->ItemBytes = 0;
	pool->ItemsPerBlock = 0;
	pool->ItemsFirstBlock = 0;
	pool->CurrentlyAllocatedItems = 0;
	pool->MaxItems = 0;
	pool->UnallocatedItems = 0;
	pool->PathItemsLeft = 0;
}

// Устанавливает рабочие параметры для приложения.
void DelaunayInitialization::setOperatingParameters(wstring inFilePath, wstring outFolderPath, Configuration* b)
{
	b->noexact = 0;
	b->dwyer = 1;
	b->firstnumber = 1;
	b->order = 1;
	b->neighbors = 1;
	b->nobound = 0;
	// Получить полное имя файла, содержащего входные данные.
	wstring inputDataFileName(inFilePath);
	// Получить полное имя папки, в которую будут записываться результаты триангуляции.
	wstring outputFolderName(outFolderPath);

	// Если тестовый файл с входными данными существует, то пока просто закрыть его.
	if (FILE* file = _wfopen(inputDataFileName.c_str(), L"r"))
		fclose(file);
	else
	{
		// Иначе, если нет файла с входными данными:

		// Создать файл.
		std::wstring::size_type found = inputDataFileName.find_last_of(L"/\\");
		std::wstring folderPath = inputDataFileName.substr(0, found);
		std::filesystem::create_directories(folderPath);
		int error;
		HANDLE hFile = CreateFile(
			inputDataFileName.c_str(),          // имя файла
			GENERIC_WRITE,                      // доступ по записи
			FILE_SHARE_READ | FILE_SHARE_WRITE, // может производиться чтение и запись файла
			NULL,                               // атрибуты надёжности
			CREATE_NEW,                         // создать файл только в том  случае, если он не существует
			FILE_ATTRIBUTE_NORMAL,              // атрибуты файла
			NULL);                              // дескриптор
		error = GetLastError();
		CloseHandle(hFile);
		// Определить строку, содержащую координаты точек для триангуляции.
		/*
		std::wstring fileContents = L"21  2  0  1\n1  0.0  0.0  1\n2  1.0  0.0  1\n3  2.0  0.0  1\n4  3.0  0.0  1\n"
			L"5  4.0  0.0  1\n6  0.0  1.0  1\n7  1.0  1.0  0\n8  2.0  1.0  0\n9  3.0  1.0  0\n"
			L"10  4.0  1.0  1\n11  0.0  2.0  1\n12  1.0  2.0  0\n13  2.0  2.0  0\n14  3.0  2.0  1\n"
			L"15  4.0  2.0  1\n16  0.0  3.0  1\n17  1.0  3.0  0\n18  2.0  3.0  1\n19  0.0  4.0  1\n"
			L"20  1.0  4.0  1\n21  2.0  4.0  1";
		//*/

		//*
		std::wstring fileContents = L"# tritest.node\n#\n# A set of fifteen points in 2D, no attributes, no boundary markers.\n"
				L"15  2  0  0\n# And here are the fifteen points.\n 1      0       0\n 2     -0.416   0.909\n 3     -1.35    0.436\n"
			    L" 4     -1.64   -0.549\n 5     -1.31   -1.51\n 6     -0.532  -2.17\n 7      0.454  -2.41\n 8      1.45   -2.21\n"
				L" 9      2.29   -1.66\n10      2.88   -0.838\n11      3.16    0.131\n12      3.12    1.14\n"
				L"13      2.77    2.08\n14      2.16    2.89\n15      1.36    3.49";
		//*/

		/*
		std::wstring fileContents = L"# tritest.node\n#\n# Набор из пятнадцати точек в двухмерных кординатах,"
			" без атрибутов и без ограничительных маркеров.\n"
			L"15  2  0  0\n# Ниже приводятся координаты пятнадцати точек.\n 1      0       0\n 2     -0.416   0.909\n 3     -1.35    0.436\n"
			L"4     -1.64   -0.549\n 5     -1.31   -1.51\n 6     -0.532  -2.17\n 7      0.454  -2.41\n 8      1.45   -2.21\n"
			L" 9      2.29   -1.66\n10      2.88   -0.838\n11      3.16    0.131\n12      3.12    1.14\n"
			L"13      2.77    2.08\n14      2.16    2.89\n15      1.36    3.49";
		//*/

		// Записать эту строку в созданный файл.
		std::wofstream f(inputDataFileName);
		//
		//std::locale loc(std::locale::classic(), new std::codecvt_utf8<wchar_t>);
		//f.imbue(loc);
		//
		f << fileContents;
		f.close();
	}

	// Создать выходную папку для результатов триангуляции.
	DWORD ftyp = GetFileAttributes(outputFolderName.c_str());
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		std::filesystem::create_directories(outputFolderName);

	const unsigned int FILENAMESIZE = 2048;

	if (inputDataFileName.length() <= FILENAMESIZE)
		b->InNodeFileName = inputDataFileName;

	for (wstring::reverse_iterator i = inputDataFileName.rbegin(); *i != '.'; ++i)
		inputDataFileName.erase(i.base() - 1);
	// Получить индекс последнего символа '\' в пути к файлу.
	std::wstring::size_type bsInx = inputDataFileName.find_last_of(L"/\\");
	// Получить только имя файла с точкой в конце.
	wstring fileNameOnly = inputDataFileName.substr(bsInx + 1);
	// Получить полное имя выходного файла.
	wstring outputFileName = (outputFolderName /*+ L"/\\" */+ fileNameOnly);

	//b->InPolyFileName = inputDataFileName;
	//b->InEleFileName = inputDataFileName;

	int meshnumber = 0;
	for (wstring::iterator it = inputDataFileName.begin(); it != inputDataFileName.end(); ++it)
	{
		if (*it == '.')
		{
			wstring::iterator nx = next(it, 1);
			while (nx != inputDataFileName.end() && (*nx >= '0' && *nx <= '9'))
			{
				meshnumber = meshnumber * 10 + (int)(*nx - '0');
				nx++;
				it = nx;
			}
		}
	}

	if (meshnumber == 0)
	{
		b->OutNodeFileName = outputFileName;
		b->OutEleFileName = outputFileName;
		b->OutEdgeFileName = outputFileName;
		b->OutNeighborFileName = outputFileName;
		b->OutNodeFileName += L"1.node";
		b->OutEleFileName += L"1.ele";
		b->OutEdgeFileName += L"1.edge";
		b->OutNeighborFileName += L"1.neigh";
	}
	else
	{
		++meshnumber;
		// Удалить точку в конце строки.
		inputDataFileName.erase(inputDataFileName.length() - 1);
		for (wstring::reverse_iterator i = inputDataFileName.rbegin(); *i != '.'; ++i)
		{
			if (isdigit(*i))
				inputDataFileName.erase(i.base() - 1);
		}
		b->OutNodeFileName = outputFileName + std::to_wstring(meshnumber);//inputDataFileName + std::to_wstring(meshnumber);
		b->OutEleFileName = b->OutNodeFileName;
		b->OutEdgeFileName = b->OutNodeFileName;
		b->OutNeighborFileName = b->OutNodeFileName;
		b->OutNodeFileName += L".node";
		b->OutEleFileName += L".ele";
		b->OutEdgeFileName += L".edge";
		b->OutNeighborFileName += L".neigh";
	}
	//b->InPolyFileName += L"poly";
	//b->InEleFileName += L"ele";
}

// Читает из файла входных данных очередную строку.
wchar_t* DelaunayInitialization::readLine(wchar_t* string, FILE* infile, const wchar_t* infilename)
{
	// Буфер для искомой строки, содержащей либо метаданные, либо информацию о вершине.
	wchar_t *result;

	// Искать что-нибудь похожее на число (т.е., строку, представляющую данные).
	do
	{
		// Получить очередную строку из файла облака точек.
		result = fgetws(string, INPUTLINESIZE, infile);
		// При неудаче вывести сообщение об ошибке и выйти из приложения.
		if (result == nullptr)
		{
			wstring errMsg = L"Ошибка:  Неожиданно встретился конец файла в файле "
				+ std::wstring(infilename) + L". Приложение будет закрыто.";
			MessageBox(NULL, errMsg.c_str(), L"Триангулятор", MB_ICONERROR | MB_OK);
			TriExit(1);
		}
		// Пропускать в полученной строке всё, что не похоже на число.
		while ((*result != L'\0') && (*result != L'#')
			&& (*result != L'.') && (*result != L'+') && (*result != L'-')
			&& ((*result < L'0') || (*result > L'9')))
		{
			result++;
		}
		// Если это комментарий или конец строки, то читать следующую строку и т.д.
	} while ((*result == L'#') || (*result == L'\0'));

	// Вернуть найденную строку.
	return result;
}

// Ищет начинающееся с числа следующее поле в переданной строке и возвращает подстроку,
// которая начинается с найденного поля и продолжается до конца переданной строки.
wchar_t* DelaunayInitialization::findField(wchar_t * string)
{
	// Значение, получаемое из переданной строки.
	wchar_t *result;

	result = string;
	// Пропустить текущее поле (например, порядковый номер записи)
	// и остановиться при достижении пробела.
	while ((*result != L'\0') && (*result != L'#')
		&& (*result != L' ') && (*result != L'\t'))
	{
		result++;
	}
	// Теперь пропустить пробел и все остальное, что не похоже на число,
	// комментарий или конец строки (например, получить значение координаты).
	while ((*result != L'\0') && (*result != L'#')
		&& (*result != L'.') && (*result != L'+') && (*result != L'-')
		&& ((*result < L'0') || (*result > L'9')))
	{
		result++;
	}
	// Проверить наличие комментария с префиксом '#'.
	if (*result == L'#')
	{
		*result = L'\0';
	}

	// Вернуть найденную подстроку.
	return result;
}

// Подсчитывает размер структуры данных вершины и инициализирует требуемый для неё объём памяти.
void DelaunayInitialization::initializeVertexPool(Mesh * m, Configuration * b)
{
	int vertexsize;

	// За индексом в каждой вершине, в которой найден граничный маркер,
	// следует тип вершины.  Необходимо убедиться, что  маркер  вершины
	// выровнен в памяти по размеру типа int.
	m->VertexMarkIndex = ((m->MeshDimension + m->AttributesPerVertex) * sizeof(double) + sizeof(int) - 1) / sizeof(int);
	vertexsize = (m->VertexMarkIndex + 2) * sizeof(int);

	// Инициализировать пул памяти для вершин треугольников.
	poolInit(&m->Vertices, vertexsize, VERTEXPERBLOCK, m->InVertices > VERTEXPERBLOCK ? m->InVertices : VERTEXPERBLOCK,
		sizeof(double));
}

// Инициализирует треугольник, который заполняет «внешнее пространство», и вездесущий подсегмент.
void DelaunayInitialization::dummyInit(Mesh* m, Configuration* b, int trianglebytes, int subsegbytes)
{
	unsigned long long alignptr = 0;

	// Установите "DummyTri", треугольник, который занимает "внешнее пространство".
	m->DummyTriBase = (Triangle *)TriMalloc(trianglebytes + m->Triangles.AlignBytes);
	// Выровнять "DummyTri" по границе, указанной для выравнивания в памяти треугольников. 
	alignptr = (unsigned long long)m->DummyTriBase;
	m->DummyTri = (Triangle*)(alignptr + (unsigned long long)m->Triangles.AlignBytes -
		(alignptr % (unsigned long long)m->Triangles.AlignBytes));
	// Инициализируйте три смежных треугольника, как «внешнее пространство».
	// В конечном итоге они будут изменены различными операциями связывания,
	// но их величины, на самом деле, не имеют значения, если они могут быть
	// разыменованы.
	m->DummyTri[0] = (Triangle)m->DummyTri;
	m->DummyTri[1] = (Triangle)m->DummyTri;
	m->DummyTri[2] = (Triangle)m->DummyTri;
	// Три нулевые вершины.
	m->DummyTri[3] = nullptr;
	m->DummyTri[4] = nullptr;
	m->DummyTri[5] = nullptr;
}

// Инициализирует пул памяти для размещения в ней элементов.
void DelaunayInitialization::poolInit(MemoryPool * pool, int bytecount, int itemcount, int firstitemcount, int alignment)
{
	// Найти правильное выравнивание, которое должно быть никак не меньше, чем:
	// - параметр 'alignment'
	// - sizeof (int*), чтобы стек элементов, из под которых была отобрана назад
	//   ранее выделенная память, мог поддерживаться без выравниваемого доступа.
	if (alignment > sizeof(void*))
		pool->AlignBytes = alignment;
	else
		pool->AlignBytes = sizeof(void*);
	pool->ItemBytes = ((bytecount - 1) / pool->AlignBytes + 1) * pool->AlignBytes;
	pool->ItemsPerBlock = itemcount;
	if (firstitemcount == 0)
		pool->ItemsFirstBlock = itemcount;
	else
		pool->ItemsFirstBlock = firstitemcount;

	// Выделить блок элементов. Выделяется пространство для элементов 'ItemsFirstBlock'
	// и для одного  указателя  (для указания на следующий блок),  а также пространство
	// для обеспечения выравнивания элементов.
	pool->FirstBlock = (void**)TriMalloc(pool->ItemsFirstBlock * pool->ItemBytes + (int) sizeof(void*) + pool->AlignBytes);
	// Установить указатель на следующий блок в ноль.
	*(pool->FirstBlock) = nullptr;
	poolRestart(pool);
}

// Забирает память  у элементов, под которые  она была выделена  ранее,
// но не возвращает её операционной системе, а делает блоки этой памяти
// готовыми к повторному использованию.
void DelaunayInitialization::poolRestart(MemoryPool * pool)
{
	unsigned long long alignptr;

	pool->CurrentlyAllocatedItems = 0;
	pool->MaxItems = 0;

	//Установить текущий активный блок памяти.
	pool->CurrentAllocationBlock = pool->FirstBlock;
	// Найти в пуле память, выделенную для первого элемента. Увеличить на размер(int*).
	alignptr = (unsigned long long)(pool->CurrentAllocationBlock + 1);
	// Выровнять элемент по границе байт 'AlignBytes'.
	pool->NextFreeMemorySlice = (void*)(alignptr + (unsigned long long)pool->AlignBytes -
		(alignptr % (unsigned long long)pool->AlignBytes));
	// В этом блоке осталось много элементов, из под которых
	// выделенная ранее память была отобрана назад.
	pool->UnallocatedItems = pool->ItemsFirstBlock;
	// Стек элементов, из под которых была отобрана память - пуст.
	pool->DeadElementsStack = nullptr;
}
