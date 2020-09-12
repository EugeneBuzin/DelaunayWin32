#include "stdafx.h"
#include "DialogEventHandler.h"


// Конструктор - cоздаёт экземпляр DialogEventHandler.
DialogEventHandler::DialogEventHandler() : _cRef(1)
{
}

// Деструктор (вызывается из функции Release).
DialogEventHandler::~DialogEventHandler()
{
}

// Возвращает указатель на интерфейс DialogEventHandler при его запросе клиентом.
IFACEMETHODIMP DialogEventHandler::QueryInterface(REFIID riid, void ** ppv)
{
	static const QITAB qit[] = {
		QITABENT(DialogEventHandler, IFileDialogEvents),
	    { 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

// Увеличивает на единицу счётчик обращений к DialogEventHandler при его запросе клиентом.
IFACEMETHODIMP_(ULONG) DialogEventHandler::AddRef()
{
	return InterlockedIncrement(&_cRef);
}

// Уменьшает на единицу счётчик обращений к DialogEventHandler
// и вызывает деструктор ~DialogEventHandler, после окончания сеанса работы клиента.
IFACEMETHODIMP_(ULONG) DialogEventHandler::Release()
{
	long cRef = InterlockedDecrement(&_cRef);
	if (!cRef)
		delete this;
	return cRef;
}

// Определяет, была ли выбрана папка.
BOOL DialogEventHandler::IsFolderPathEmpty(HWND hwnd)
{
	wstring fName = L"Ошибка в функции IsFolderPathEmpty: ";
	HRESULT hr;

	hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, reinterpret_cast<void**>(&pClientUIA));
	if (S_OK != hr)
	{
		wstring errMsd = fName + UsualHelper::GetLastErrorMessage(GetLastError());
		MessageBox(NULL, errMsd.c_str(), L"Триангулятор", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	hr = pClientUIA->ElementFromHandle(hwnd, &pRootElement);
	if (S_OK != hr)
	{
		wstring errMsd = fName + UsualHelper::GetLastErrorMessage(GetLastError());
		MessageBox(NULL, errMsd.c_str(), L"Триангулятор", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	BSTR name;
	hr = pRootElement->get_CurrentClassName(&name);
	if (S_OK != hr)
	{
		wstring errMsd = fName + UsualHelper::GetLastErrorMessage(GetLastError());
		MessageBox(NULL, errMsd.c_str(), L"Триангулятор", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	IUIAutomationCondition *pCondition;
	VARIANT varProp;
	varProp.vt = VT_I4;
	varProp.uintVal = UIA_EditControlTypeId;
	hr = pClientUIA->CreatePropertyCondition(UIA_ControlTypePropertyId, varProp, &pCondition);
	if (S_OK != hr)
	{
		wstring errMsd = fName + UsualHelper::GetLastErrorMessage(GetLastError());
		MessageBox(NULL, errMsd.c_str(), L"Триангулятор", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	IUIAutomationElementArray *pElementFound;
	hr = pRootElement->FindAll(TreeScope_Descendants, pCondition, &pElementFound);
	if (S_OK != hr)
	{
		wstring errMsd = fName + UsualHelper::GetLastErrorMessage(GetLastError());
		MessageBox(NULL, errMsd.c_str(), L"Триангулятор", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	int eleCount;
	pElementFound->get_Length(&eleCount);
	for (int i = 0; i < eleCount; i++)
	{
		IUIAutomationElement *pElement;
		hr = pElementFound->GetElement(i, &pElement);
		if (S_OK != hr)
		{
			wstring errMsd = fName + UsualHelper::GetLastErrorMessage(GetLastError());
			MessageBox(NULL, errMsd.c_str(), L"Триангулятор", MB_ICONERROR | MB_OK);
			return FALSE;
		}
		hr = pElement->get_CurrentName(&name);
		if (S_OK != hr)
		{
			wstring errMsd = fName + UsualHelper::GetLastErrorMessage(GetLastError());
			MessageBox(NULL, errMsd.c_str(), L"Триангулятор", MB_ICONERROR | MB_OK);
			return FALSE;
		}

		OutputDebugString(name);

		if (0 == wcscmp(name, L"Folder:"))
		{
			VARIANT varPropText;
			hr = pElement->GetCurrentPropertyValue(UIA_ValueValuePropertyId, &varPropText);
			if (S_OK != hr)
			{
				wstring errMsd = fName + UsualHelper::GetLastErrorMessage(GetLastError());
				MessageBox(NULL, errMsd.c_str(), L"Триангулятор", MB_ICONERROR | MB_OK);
				return FALSE;
			}

			if (0 == wcscmp(varPropText.bstrVal, L""))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

// При закрытии диалога выбора папки проверяет была или нет выбрана папка.
// Если папка не была выбрана, а пользователь нажал кнопку "OK", то диалог
// остаётся на экране. Иначе, диалог закрывается.
IFACEMETHODIMP DialogEventHandler::OnFileOk(IFileDialog* pfd)
{
	IOleWindow *pWindow;
	HRESULT hr = pfd->QueryInterface(IID_PPV_ARGS(&pWindow));

	if (SUCCEEDED(hr))
	{
		HWND hwndDialog;
		hr = pWindow->GetWindow(&hwndDialog);

		if (SUCCEEDED(hr))
		{
			if (IsFolderPathEmpty(hwndDialog))
				return S_FALSE;
		}
		pWindow->Release();
	}

	return S_OK;
}

