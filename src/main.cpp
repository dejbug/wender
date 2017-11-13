#include "main.h"
#include "resource.h"

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include <wx/wx.h>
#include <wx/sysopt.h>
#include <wx/thread.h>
#include <wx/htmllbox.h>
#include <wx/gauge.h>

#include "lib_font.h"

struct App : public wxApp
{
	virtual bool OnInit();
};

wxIMPLEMENT_APP(App);

struct FontList : public wxSimpleHtmlListBox
{
	FontList(wxWindow * parent);

private:
	virtual void OnDrawSeparator(wxDC & dc, wxRect & r, size_t n) const;
};

struct ProgressBar : public wxPanel
{
	wxGauge * gauge = nullptr;

	ProgressBar(wxWindow * parent);

protected:
	virtual void OnSize(wxSizeEvent & e);
};

struct FontList2 : public wxPanel, public wxThreadHelper
{
	FontList * fontList = nullptr;
	ProgressBar * progressBar = nullptr;

	FontList2(wxWindow * parent);
	void LoadFonts();
	void ShowProgressBar(bool show=true);
	bool IsProgressBarShown() const;

protected:
	virtual wxThread::ExitCode Entry();
	virtual void OnThreadUpdate(wxThreadEvent & e);
};

struct MainFrame : public wxFrame
{
	FontList * fontList = nullptr;
	FontList2 * fontList2 = nullptr;

	MainFrame();

protected:
	virtual void OnF8(wxCommandEvent & e);

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

ProgressBar::ProgressBar(wxWindow * parent) : wxPanel(parent, wxID_ANY)
{
	gauge = new wxGauge(this, wxID_ANY, 100);

	Bind(wxEVT_SIZE, &ProgressBar::OnSize, this);

	// wxFlexGridSizer * s = new wxFlexGridSizer(3, 3, 0, 0);
	// s->AddGrowableCol(1, 1);
	// s->AddGrowableRow(1, 1);
	// s->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_ALL);
	// s->Add(gauge, 0, wxALIGN_CENTER);
	// SetSizer(s);

	// wxBoxSizer * v = new wxBoxSizer(wxVERTICAL);
	// wxBoxSizer * h = new wxBoxSizer(wxHORIZONTAL);
	// h->Add(gauge, 0, wxSHAPED | wxALIGN_CENTRE);
	// v->Add(h, 0, wxSHAPED | wxALIGN_CENTRE);
	// SetSizer(v);
}

void ProgressBar::OnSize(wxSizeEvent & e)
{
	e.Skip();
	printf("ProgressBar::OnSize\n");

	wxSize cs = GetClientSize();
	wxSize s = gauge->GetSize();

	int const xoff = (cs.x - s.x) >> 1;
	int const yoff = (cs.y - s.y) >> 1;

	gauge->SetPosition(wxPoint(xoff, yoff));
}

FontList2::FontList2(wxWindow * parent) : wxPanel(parent, wxID_ANY)
{
	fontList = new FontList(this);
	progressBar = new ProgressBar(this);

	wxBoxSizer * s = new wxBoxSizer(wxVERTICAL);
	s->Add(fontList, 1, wxEXPAND);
	s->Add(progressBar, 1, wxEXPAND);
	SetSizer(s);

	ShowProgressBar(true);
	progressBar->gauge->SetValue(17);
}

void FontList2::LoadFonts()
{}

void FontList2::ShowProgressBar(bool show)
{
	wxSizer * s = GetSizer();
	s->Hide((size_t) (show ? 0 : 1));
	s->Show((size_t) (show ? 1 : 0));
	s->Layout();
}

bool FontList2::IsProgressBarShown() const
{
	return GetSizer()->IsShown(1);
}

void FontList2::OnThreadUpdate(wxThreadEvent & e)
{}

wxThread::ExitCode FontList2::Entry()
{
	return (wxThread::ExitCode) 0;
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
	wxToolBar * toolBar = new wxToolBar(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL);

	toolBar->AddTool(wxID_PASTE, wxT("Paste"), util::load_png(ID_PNG_PASTE_16));

	toolBar->Realize();
	frame->SetToolBar(toolBar);
}

static bool ConvertUnicodeToAnsi(std::string & out, wchar_t const * text, UINT cp=CP_ACP)
{
	out.erase();
	const int size = WideCharToMultiByte(cp, WC_COMPOSITECHECK|WC_SEPCHARS, text, -1, nullptr, 0, nullptr, nullptr);
	if(!size) return false;
	out.resize(size);
	return 0 != WideCharToMultiByte(cp, WC_COMPOSITECHECK|WC_SEPCHARS, text, -1, const_cast<char *>(out.data()), size, nullptr, nullptr);
}

static void CreateAndInstallFontList(MainFrame * frame)
{
	// frame->fontList = new FontList(frame);
	frame->fontList2 = new FontList2(frame);
	frame->fontList = frame->fontList2->fontList;
}

static void PopulateFontList(MainFrame * frame)
{
#ifndef __WXMSW__
	assert(0);
	return;
#endif

	auto addFontListFont = [&frame](char const * face)
	{
		std::stringstream ss;
		ss << "<TABLE VALIGN=CENTER><TR><TD NOWRAP><FONT SIZE=+3 FACE=\"" << face << "\">" << face << "</FONT></TD></TR><TR><TD NOWRAP><FONT FACE=\"" << face << "\" SIZE=+1>The quick brown fox jumps over the lazy dog</FONT></TD></TR></TABLE>";
		frame->fontList->Append(ss.str().c_str());
	};

	HWND const hwnd = (HWND) frame->GetHandle();
	HDC dc = GetDC(hwnd);
	std::vector<lib::font::EnumFontInfo> ff;
	lib::font::list_fonts(ff, ANSI_CHARSET, false, dc);
	lib::font::sort_fonts(ff);
	ReleaseDC(hwnd, dc);

	for (size_t i=0; i<ff.size(); ++i)
	{
		// lib::font::print_font_info(ff[i]);
		std::string n;
		ConvertUnicodeToAnsi(n, ff[i].elfe.elfLogFont.lfFaceName);
		// printf("%s\n", n.c_str());
		addFontListFont(n.c_str());
	}
}

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, APP_NAME, wxDefaultPosition, wxSize(800,400))
{
	SetIcon(util::load_ico(ID_ICO_APPLICATION));

	CreateStatusBar();
	SetStatusText("");

	CreateAndInstallMenuBar(this);
	CreateAndInstallToolBar(this);
	CreateAndInstallFontList(this);
	// PopulateFontList(this);

	wxAcceleratorEntry entries[2] = {
		{wxACCEL_NORMAL, WXK_ESCAPE, wxID_EXIT},
		{wxACCEL_NORMAL, WXK_F8, 4711},
	};

	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);

	Bind(wxEVT_MENU, &MainFrame::OnF8, this, 4711);

	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_TOOL, &MainFrame::OnPaste, this, wxID_PASTE);
}

void MainFrame::OnF8(wxCommandEvent & e)
{
	// printf("f8\n");
	fontList2->ShowProgressBar(!fontList2->IsProgressBarShown());
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
