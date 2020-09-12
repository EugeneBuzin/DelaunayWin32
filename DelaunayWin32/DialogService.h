#pragma once

#include <string>
#include "DialogEventHandler.h"

// Создатель диалоговых окон.
class DialogService
{
public:
	DialogService();
	~DialogService();

	// Создаёт интерфейс обработчика событий диалога для работы с файлами.
	HRESULT DialogEventHandler_CreateInstance(REFIID riid, void** ppv);
	// Создаёт диалоговое окно для выбора файла.
	HRESULT CreateDialogToSelectFile(HWND hWnd);
	// Создаёт диалоговое окно для выбора папки.
	HRESULT CreateDialogToSelectFolder(HWND hWnd);
	// Возвращает путь к файлу с входными данными для триангулции.
	std::wstring GetInputDataFilePath() { return filePath; }
	// Возвращает путь к папке, содержщей файлы с результатами триангуляции.
	std::wstring GetOutputFilesFolderPath() { return folderPath; }
private:
	// Путь к входному файлу с облаком точек для триангуляции.
	std::wstring filePath;
	// Путь к папке, содержащей выходные файлы с результатами триангуляции.
	std::wstring folderPath;
};

