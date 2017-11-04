#include "main.h"
#include "resource.h"

#include <iostream>

#include <wx/wx.h>
#include <wx/sysopt.h>

struct App : public wxApp
{
	virtual bool OnInit();
};

wxIMPLEMENT_APP(App);

struct MainFrame : public wxFrame
{
	MainFrame();

private:
	void CreateMenu();
	void CreateToolBar();

	void OnExit(wxCommandEvent & e);
	void OnPaste(wxCommandEvent & e);
};

namespace util {

wxIcon load_ico(WORD id);

wxBitmap load_png(WORD id);

struct ResourceLock
{
	HGLOBAL handle = nullptr;
	char const * data = nullptr;
	size_t size = 0;

	ResourceLock(WORD id, HINSTANCE hi=GetModuleHandle(nullptr));
	~ResourceLock();
};

} // namespace util

bool App::OnInit()
{
	wxInitAllImageHandlers();
	wxSystemOptions::SetOption("msw.remap", 0);

	auto frame = new MainFrame();
	frame->Centre();
	frame->Show(true);
	return true;
}

void CreateAndInstallMenuBar(MainFrame * frame)
{
	wxMenu * menuFile = new wxMenu;
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenuBar * menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");

	frame->SetMenuBar(menuBar);
}

void CreateAndInstallToolBar(MainFrame * frame)
{
	wxToolBar * toolBar = new wxToolBar(frame, wxID_ANY);

	toolBar->AddTool(wxID_PASTE, wxT("Paste"), util::load_png(ID_PNG_PASTE_16));

	toolBar->Realize();
	frame->SetToolBar(toolBar);
}

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, APP_NAME)
{
	SetIcon(util::load_ico(ID_ICO_APPLICATION));

	CreateAndInstallMenuBar(this);
	CreateAndInstallToolBar(this);

	CreateStatusBar();
	SetStatusText("");

	wxAcceleratorEntry entries[1] = {
		{wxACCEL_NORMAL, WXK_ESCAPE, wxID_EXIT},
	};

	wxAcceleratorTable accel(1, entries);
	SetAcceleratorTable(accel);

	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_TOOL, &MainFrame::OnPaste, this, wxID_PASTE);
}

void MainFrame::OnExit(wxCommandEvent & e)
{
	Close(true);
}

void MainFrame::OnPaste(wxCommandEvent & e)
{
	std::cout << "OnPaste" << std::endl;
}

wxIcon util::load_ico(WORD id)
{
	HICON hicon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(id));
	assert(!!hicon);

	wxIcon icon;
	icon.CreateFromHICON(hicon);
	return icon;
}

wxBitmap util::load_png(WORD id)
{
	ResourceLock reslock(id);
	return wxBitmap::NewFromPNGData(reslock.data, reslock.size);
}

util::ResourceLock::ResourceLock(WORD id, HINSTANCE hi)
{
	HRSRC res_find_handle = FindResource(hi, MAKEINTRESOURCE(id), RT_RCDATA);
	assert(!!res_find_handle);

	size = (size_t) SizeofResource(hi, res_find_handle);

	HGLOBAL handle = LoadResource(hi, res_find_handle);
	assert(!!handle);

	LPVOID res_lock_handle = LockResource(handle);
	assert(!!res_lock_handle);

	data = (char const *) res_lock_handle;
}

util::ResourceLock::~ResourceLock()
{
	if (handle) FreeResource(handle);
}
