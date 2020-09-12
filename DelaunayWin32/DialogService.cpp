#include "stdafx.h"
#include "DialogService.h"


DialogService::DialogService()
{
}


DialogService::~DialogService()
{
}

// Создаёт интерфейс обработчика событий диалога для работы с файлами.
HRESULT DialogService::DialogEventHandler_CreateInstance(REFIID riid, void** ppv)
{
	*ppv = NULL;
	DialogEventHandler *pDialogEventHandler = new (std::nothrow) DialogEventHandler();
	HRESULT hr = pDialogEventHandler ? S_OK : E_OUTOFMEMORY;
	if (SUCCEEDED(hr))
	{
		hr = pDialogEventHandler->QueryInterface(riid, ppv);
		pDialogEventHandler->Release();
	}
	return hr;
}

// Создаёт диалоговое окно для выбора файла.
HRESULT DialogService::CreateDialogToSelectFile(HWND hWnd)
{
	// Создать диалог FileOpenDialog (для выбора входного файла облака точек).
	IFileDialog *pfd = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		// Создать объект обработки событий и подключить его к созданному диалогу.
		IFileDialogEvents *pfde = NULL;
		hr = DialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
		if (SUCCEEDED(hr))
		{
			// Подключить обработчик событий.
			DWORD dwCookie;
			hr = pfd->Advise(pfde, &dwCookie);
			if (SUCCEEDED(hr))
			{
				// Установить параметры для диалогового окна.
				DWORD dwFlags;
				// Перед настройкой, сперва всегда следует получить параметры, чтобы не перекрывать существующие параметры.
				hr = pfd->GetOptions(&dwFlags);
				if (SUCCEEDED(hr))
				{
					// Настроить диалог для показа элементов файловой системы и для выбора файлов.
					hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
					if (SUCCEEDED(hr))
					{
						// Установить типы файлов только для их просмотра. Обращаем внимание, что это одномерный массив.
						hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
						if (SUCCEEDED(hr))
						{
							// Установить индекс для типа файла триангулируемого облака точек.
							hr = pfd->SetFileTypeIndex(INDEX_NODEFILE);
							if (SUCCEEDED(hr))
							{
								// Установите расширение по умолчанию для файла ".node".
								hr = pfd->SetDefaultExtension(L"node");
								if (SUCCEEDED(hr))
								{
									// Вывести диалог на экран.
									hr = pfd->Show(hWnd);
									if (SUCCEEDED(hr))
									{
										// Получить результат, как только пользователь нажмет кнопку «Открыть».
										// Результатом является объект IShellItem.
										IShellItem *psiResult;
										hr = pfd->GetResult(&psiResult);
										if (SUCCEEDED(hr))
										{
											// Получить полный путь к входному файлу .node.
											PWSTR pszFilePath = NULL;
											hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
											if (SUCCEEDED(hr))
											{
												filePath = pszFilePath;
												CoTaskMemFree(static_cast<void*>(pszFilePath));
											}
											psiResult->Release();
										}
									}
								}
							}
						}
					}
				}
				// Отключить обработчик событий.
				pfd->Unadvise(dwCookie);
			}
			pfde->Release();
		}
		pfd->Release();
	}

	return hr;
}

// Создаёт диалоговое окно для выбора папки.
HRESULT DialogService::CreateDialogToSelectFolder(HWND hWnd)
{
	//*
	// Создать диалог FileOpenDialog для выбора выбора папок.
	IFileDialog *pfd;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		// Создать объект обработки событий и подключить его к созданному диалогу.
		IFileDialogEvents *pfde = NULL;
		hr = DialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
		if (SUCCEEDED(hr))
		{
			// Подключить обработчик событий.
			DWORD dwCookie;
			hr = pfd->Advise(pfde, &dwCookie);
			if (SUCCEEDED(hr))
			{
				// Установить параметры для диалогового окна.
				DWORD dwOptions;
				// Перед настройкой, сперва всегда следует получить параметры, чтобы не перекрывать существующие параметры.
				hr = pfd->GetOptions(&dwOptions);
				if (SUCCEEDED(hr))
				{
					// Настроить диалог только для показа и выбора папок.
					hr = pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
					if (SUCCEEDED(hr))
					{
						// Вывести диалог на экран.
						hr = pfd->Show(hWnd);
						if (SUCCEEDED(hr))
						{
							// Получить результат, как только пользователь нажмет кнопку «Выбрать папку».
							// Результатом является объект IShellItem.
							IShellItem *psi;
							hr = pfd->GetResult(&psi);
							if (SUCCEEDED(hr))
							{
								PWSTR pszFolderPath = NULL;
								hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);
								if (SUCCEEDED(hr))
								{
									folderPath = pszFolderPath;
									CoTaskMemFree(static_cast<void*>(pszFolderPath));
								}
								//else
								//{
									//MessageBox(NULL, L"GetIDListName() failed", NULL, NULL);
								//}
							}
							psi->Release();
						}
					}
				}
				// Отключить обработчик событий.
				pfd->Unadvise(dwCookie);
			}
			pfde->Release();
		}
		pfd->Release();
	}

	return hr;
	//*/
	/*
	IFileOpenDialog* pPickFolderDialog = NULL;
	IShellItem* pPickedFolder = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pPickFolderDialog));
	if (SUCCEEDED(hr))
	{
		DWORD dialogOptions;
		hr = pPickFolderDialog->GetOptions(&dialogOptions);
		if (SUCCEEDED(hr))
		{
			hr = pPickFolderDialog->SetOptions(dialogOptions | FOS_PICKFOLDERS);
			if (SUCCEEDED(hr))
			{
				hr = pPickFolderDialog->Show(hWnd);
				if (SUCCEEDED(hr))
				{
					hr = pPickFolderDialog->GetResult(&pPickedFolder);
					if (SUCCEEDED(hr))
					{
						PWSTR pszFolderPath = NULL;
						hr = pPickedFolder->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);
						if (SUCCEEDED(hr))
						{
							CoTaskMemFree(pszFolderPath);
						}
					}
					pPickedFolder->Release();
				}
			}
		}
		pPickFolderDialog->Release();
	}
	return hr;
	//*/
}
