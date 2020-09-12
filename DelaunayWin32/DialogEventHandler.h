#pragma once

#define STRICT_TYPED_ITEMIDS
#include <objbase.h>      // Для заголовков COM.
#include <shobjidl.h>     // Для интерфейсов IFileDialogEvents и IFileDialogControlEvents.
#include <shlwapi.h>      // Для хранения и извлечения метаданных для элементов оболочки (файлов, папок).
#include <new>            // Для использования std::nothrow.
#include <UIAutomation.h> // Для определения, была или нет выбрана папка для записи результатов.
#include "DelaunayHelper.h"


const COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
	{L"Файл облака точек (*.node)",       L"*.node"}
};

// Индекс типа файла, содержащего облако точек для триангуляции.
#define INDEX_NODEFILE 1

// Реализует обработку событий диалога для работы с файлами.
class DialogEventHandler : public IFileDialogEvents
{
public:
	// Конструктор - cоздаёт экземпляр DialogEventHandler.
	DialogEventHandler();
	
	// Реализация интерфейса IUnknown:
	// - возвращает указатель на интерфейс DialogEventHandler при его запросе клиентом,
	IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);
	// - увеличивает на единицу счётчик обращений к DialogEventHandler при его запросе клиентом,
	IFACEMETHODIMP_(ULONG) AddRef();
	// - уменьшает на единицу счётчик обращений к DialogEventHandler
	//   и вызывает деструктор ~DialogEventHandler, после окончания сеанса работы клиента.
	IFACEMETHODIMP_(ULONG) Release();

	// Реализация интерфейса IFileDialogEvents:
	// При закрытии диалога выбора папки проверяет была или нет выбрана папка.
    // Если папка не была выбрана, а пользователь нажал кнопку "OK", то диалог
    // остаётся на экране. Иначе, диалог закрывается.
	IFACEMETHODIMP OnFileOk(IFileDialog *);
	// Обработчики для остальных семи событий ниже не реализованы в приложении за их ненадобностью.
	IFACEMETHODIMP OnFolderChange(IFileDialog *) { return E_NOTIMPL; };
	IFACEMETHODIMP OnFolderChanging(IFileDialog *, IShellItem *) { return E_NOTIMPL; };
	IFACEMETHODIMP OnSelectionChange(IFileDialog *) { return E_NOTIMPL; };
	IFACEMETHODIMP OnHelp(IFileDialog *) { return E_NOTIMPL; };
	IFACEMETHODIMP OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *) { return E_NOTIMPL; };
	IFACEMETHODIMP OnTypeChange(IFileDialog *pfd) { return E_NOTIMPL; };
	IFACEMETHODIMP OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *) { return E_NOTIMPL; };
private:
	// Деструктор (вызывается из функции Release).
	~DialogEventHandler();
	// Определяет, была ли выбрана папка.
	BOOL IsFolderPathEmpty(HWND hwnd);
	// Счётчик обращений к интерфейсу DialogEventHandler.
	long _cRef;
	IUIAutomation* pClientUIA;
	IUIAutomationElement* pRootElement;

};

