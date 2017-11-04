#include "main.h"
#include "resource.h"

#include <iostream>

#include <wx/wx.h>
#include <wx/sysopt.h>
#include <wx/htmllbox.h>

struct App : public wxApp
{
	virtual bool OnInit();
};

wxIMPLEMENT_APP(App);

struct FontList : wxSimpleHtmlListBox
{
	FontList(wxWindow * parent);

private:
	virtual void OnDrawSeparator(wxDC & dc, wxRect & r, size_t n) const;
};

struct MainFrame : public wxFrame
{
	FontList * fontList = nullptr;

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

FontList::FontList(wxWindow * parent) : wxSimpleHtmlListBox(parent, wxID_ANY)
{
	// SetSelectionBackground(wxColour("orange"));
}

void FontList::OnDrawSeparator(wxDC & dc, wxRect & r, size_t n) const
{
	if (0 == n) return;
	dc.SetPen(*wxBLACK_DASHED_PEN);
	dc.DrawLine(r.x, r.y, r.x + r.width, r.y);
}

static void CreateAndInstallMenuBar(MainFrame * frame)
{
	wxMenu * menuFile = new wxMenu;
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenuBar * menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");

	frame->SetMenuBar(menuBar);
}

static void CreateAndInstallToolBar(MainFrame * frame)
{
	wxToolBar * toolBar = new wxToolBar(frame, wxID_ANY);

	toolBar->AddTool(wxID_PASTE, wxT("Paste"), util::load_png(ID_PNG_PASTE_16));

	toolBar->Realize();
	frame->SetToolBar(toolBar);
}

static void CreateAndInstallFontList(MainFrame * frame)
{
	frame->fontList = new FontList(frame);

	frame->fontList->Append("<H1>hello!</H1>");
	frame->fontList->Append("how do you do?");
	frame->fontList->Append("<TABLE VALIGN=\"CENTER\"><TR><TD><IMG SRC=\"../src/dejbug.ico\" width=\"32\" height=\"32\"></IMG></TD><TD><FONT SIZE=\"16\">see you again soon!</FONT></TD></TR></TABLE>");
	frame->fontList->Append("<TABLE VALIGN=CENTER><TR><TD ROWSPAN=2><IMG SRC=\"../src/dejbug.ico\" width=32 height=32></IMG></TD><TD NOWRAP><FONT SIZE=+3>Home - dejbug/wender</FONT></TD></TR><TR><TD COLSPAN=2 NOWRAP><FONT FACE=\"Courier New\" SIZE=+1>https://github.com/dejbug/wender</FONT></TD></TR></TABLE>");
}

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, APP_NAME)
{
	SetIcon(util::load_ico(ID_ICO_APPLICATION));

	CreateStatusBar();
	SetStatusText("");

	CreateAndInstallMenuBar(this);
	CreateAndInstallToolBar(this);
	CreateAndInstallFontList(this);

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
