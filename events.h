#include <string>
#include <windows.h>      
#include <shobjidl.h>     
#include <shlwapi.h>
#include <new>

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "Shlwapi.lib")

// Controls
#define CONTROL_GROUP 2000
#define CONTROL_LABEL 5002
#define CONTROL_EDITBOX_MINDIST 5003

std::string currentFilename;

void StopPreview();
void RetexturePreview(std::string);


class CDialogEventHandler : public IFileDialogEvents, public IFileDialogControlEvents
{
public:
	// IUnknown methods
	IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) {
		static const QITAB qit[] = {
			QITABENT(CDialogEventHandler, IFileDialogEvents),
			QITABENT(CDialogEventHandler, IFileDialogControlEvents),
			{ 0 },
		};
		return QISearch(this, qit, riid, ppv);
	}

	IFACEMETHODIMP_(ULONG) AddRef() {
		return InterlockedIncrement(&_cRef);
	}

	IFACEMETHODIMP_(ULONG) Release() {
		long cRef = InterlockedDecrement(&_cRef);
		if (!cRef)
			delete this;
		return cRef;
	}

	// IFileDialogEvents methods
	IFACEMETHODIMP OnSelectionChange(IFileDialog*);
	IFACEMETHODIMP OnFileOk(IFileDialog*);


	IFACEMETHODIMP OnFolderChange(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnFolderChanging(IFileDialog *, IShellItem *) { return S_OK; };
	IFACEMETHODIMP OnHelp(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *) { return S_OK; };
	IFACEMETHODIMP OnTypeChange(IFileDialog *) { return S_OK; };
	IFACEMETHODIMP OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *) { return S_OK; };

	// IFileDialogControlEvents methods
	//IFACEMETHODIMP OnFileOk(IFileDialog *pfd) { return S_OK; };
	IFACEMETHODIMP OnItemSelected(IFileDialogCustomize *, DWORD, DWORD) { return S_OK; };
	IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *, DWORD) { return S_OK; };
	IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize *, DWORD, BOOL) { return S_OK; };
	IFACEMETHODIMP OnControlActivating(IFileDialogCustomize *, DWORD) { return S_OK; };

	CDialogEventHandler() : _cRef(1) { };
private:
	~CDialogEventHandler() { };
	long _cRef;
};

// Instance creation helper
HRESULT CDialogEventHandler_CreateInstance(REFIID riid, void **ppv)
{
	*ppv = NULL;
	CDialogEventHandler *pDialogEventHandler = new (std::nothrow) CDialogEventHandler();
	HRESULT hr = pDialogEventHandler ? S_OK : E_OUTOFMEMORY;
	if (SUCCEEDED(hr))
	{
		hr = pDialogEventHandler->QueryInterface(riid, ppv);
		pDialogEventHandler->Release();
	}
	return hr;
}

IFACEMETHODIMP CDialogEventHandler::OnSelectionChange(IFileDialog* fileDialog)
{
	//TextureData tex;
	std::string fileName, filePath;
	LPWSTR n;


	//File chosen
	IShellItem* pItem;
	HRESULT hr = fileDialog->GetCurrentSelection(&pItem);
	if (SUCCEEDED(hr))
	{
		//Filepath from file
		pItem->GetDisplayName(SIGDN_FILESYSPATH, &n);
		fileName = WCHAR_TO_STRING(n);

		if (fileName.size() != 0)
		{
			RetexturePreview(fileName);
		}
	}





	return S_OK;
}

IFACEMETHODIMP CDialogEventHandler::OnFileOk(IFileDialog* fileDialog)
{
	StopPreview();
	return S_OK;
}












//Remove ".ps"
//fileName.pop_back();
//fileName.pop_back();
//fileName.pop_back();

//fileName.append(".png");

//Test texture
//fileName = "arrow.png";


//Add Textures filepath
//filePath = "Data/Textures/";
//filePath.append(fileName);


//unsigned int x, y;
//PNGSize(filePath.c_str(), x, y);

//tex.texturename = filePath.c_str();
//tex.width = x;
//tex.height = y;









//IFileDialogCustomize *fileCustomize = NULL;
//fileDialog->QueryInterface(IID_PPV_ARGS(&fileCustomize));

//wchar_t *buf;
//fileCustomize->GetEditBoxText(CONTROL_EDITBOX_MINDIST, &buf);
//MessageBox(0, buf, 0, 0);
//CoTaskMemFree(buf);

//	fileCustomize->Release();