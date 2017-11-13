#include "main.h"
#include "resource.h"

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include <wx/wx.h>
#include <wx/log.h>
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
	HDC dc = nullptr;

	FontList2(wxWindow * parent);
	void LoadFonts();
	void ShowProgressBar(bool show=true);
	bool IsProgressBarShown() const;

protected:
	virtual wxThread::ExitCode Entry();
	virtual void OnThreadUpdate(wxThreadEvent & e);
	void NotifyGui(int code, long extra=0);
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

bool ConvertUnicodeToAnsi(std::string & out, wchar_t const * text, UINT cp=CP_ACP);

} // namespace util

bool App::OnInit()
{
	wxLog * logger = new wxLogStream(&std::cerr);
	wxLog::SetActiveTarget(logger);

#ifndef __WXMSW__
	wxLogError("This is an MS Windows app. Sorry.");
	return false;
#endif

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
}

void ProgressBar::OnSize(wxSizeEvent & e)
{
	e.Skip();
	wxLogMessage("ProgressBar::OnSize");

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
	s->Hide((size_t) 1);
	SetSizer(s);
}

void FontList2::LoadFonts()
{
    if (CreateThread(wxTHREAD_DETACHED) != wxTHREAD_NO_ERROR)
    {
    	wxLogError("Could not create the worker thread!");
    	return;
    }

    if (GetThread()->Run() != wxTHREAD_NO_ERROR)
    {
    	wxLogError("Could not run the worker thread!");
    	return;
    }
}

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

void FontList2::NotifyGui(int code, long extra)
{
	wxThreadEvent * e = new wxThreadEvent();
	e->SetInt(code);
	e->SetExtraLong(extra);
	// wxQueueEvent(GetEventHandler(), e);
	wxQueueEvent(this, e);

	// wxThreadEvent e;
	// e.SetInt(code);
	// e.SetExtraLong(extra);
	// // wxTheApp->QueueEvent(e.Clone());
	// // GetEventHandler()->ProcessEvent(e);
	// wxQueueEvent(GetEventHandler(), e.Clone());
}

void FontList2::OnThreadUpdate(wxThreadEvent & e)
{
	wxLogMessage("FontList2::OnThreadUpdate %d %ld", e.GetInt(), e.GetExtraLong());

	if (e.GetInt() == 0)
	{
		ShowProgressBar(false);

		HWND const hwnd = (HWND) GetHandle();
		ReleaseDC(hwnd, dc);
		dc = nullptr;

		wxLogMessage("FontList2 Worker ended!");
	}

	else if (e.GetInt() == 1)
	{
		HWND const hwnd = (HWND) GetHandle();
		dc = GetDC(hwnd);

		progressBar->gauge->SetRange(1);
		progressBar->gauge->SetValue(0);
		ShowProgressBar(true);

		wxLogMessage("FontList2 Worker started!");
	}

	else if (e.GetInt() == 2)
	{
		wxLogMessage("FontList2 Worker: %ld fonts found.", e.GetExtraLong());
		progressBar->gauge->SetRange(e.GetExtraLong());
	}

	else if (e.GetInt() == 3)
	{
		progressBar->gauge->SetValue(e.GetExtraLong());
	}
}

wxThread::ExitCode FontList2::Entry()
{
	auto addFontListFont = [this](char const * face)
	{
		std::stringstream ss;
		ss << "<TABLE VALIGN=CENTER><TR><TD NOWRAP><FONT SIZE=+3 FACE=\"" << face << "\">" << face << "</FONT></TD></TR><TR><TD NOWRAP><FONT FACE=\"" << face << "\" SIZE=+1>The quick brown fox jumps over the lazy dog</FONT></TD></TR></TABLE>";
		this->fontList->Append(ss.str().c_str());
	};

	wxLogMessage("FontList2 Worker: started!");

	// NotifyGui(1);
	// Sleep(1000);

	std::vector<lib::font::EnumFontInfo> ff;

	wxMutexGuiEnter();
	progressBar->gauge->SetRange(1);
	progressBar->gauge->SetValue(0);
	ShowProgressBar(true);

	HWND const hwnd = (HWND) GetHandle();
	wxMutexGuiLeave();

	HDC dc = GetDC(hwnd);
	wxLogMessage("FontList2 Worker: Enumerating fonts...");
	lib::font::list_fonts(ff, ANSI_CHARSET, false, dc);
	ReleaseDC(hwnd, dc);

	wxLogMessage("FontList2 Worker: %ld fonts found.", ff.size());

	// NotifyGui(2, ff.size());

	wxMutexGuiEnter();
	progressBar->gauge->SetRange(ff.size());
	wxMutexGuiLeave();

	wxLogMessage("FontList2 Worker: Sorting fonts...");
	lib::font::sort_fonts(ff);

	std::vector<std::string> nn;
	wxLogMessage("FontList2 Worker: Extracting font names...");
	for (size_t i=0; i<ff.size(); ++i)
	{
		// lib::font::print_font_info(ff[i]);
		std::string n;
		util::ConvertUnicodeToAnsi(n, ff[i].elfe.elfLogFont.lfFaceName);
		// printf("%s\n", n.c_str());
		nn.push_back(n);
	}

	wxLogMessage("FontList2 Worker: Populating list...");
	for (size_t i=0; i<nn.size() && !GetThread()->TestDestroy(); ++i)
	{
		wxMutexGuiEnter();
		// addFontListFont(nn[i].c_str());
		Sleep(100);
		progressBar->gauge->SetValue(i+1);
		wxMutexGuiLeave();

		// NotifyGui(3, i+1);
	}

	// Sleep(100);
	// NotifyGui(0);

	wxMutexGuiEnter();
	ShowProgressBar(false);
	wxMutexGuiLeave();

	wxLogMessage("FontList2 Worker: ended!");

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

static void CreateAndInstallFontList(MainFrame * frame)
{
	// frame->fontList = new FontList(frame);
	frame->fontList2 = new FontList2(frame);
	frame->fontList = frame->fontList2->fontList;
}

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, APP_NAME, wxDefaultPosition, wxSize(800,400))
{
	SetIcon(util::load_ico(ID_ICO_APPLICATION));

	CreateStatusBar();
	SetStatusText(" (use F8 to trigger user action) ");

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
	// wxLogMessage("f8");
	SetStatusText("");
	// fontList2->ShowProgressBar(!fontList2->IsProgressBarShown());
	fontList2->LoadFonts();
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

bool util::ConvertUnicodeToAnsi(std::string & out, wchar_t const * text, UINT cp)
{
	out.erase();
	const int size = WideCharToMultiByte(cp, WC_COMPOSITECHECK|WC_SEPCHARS, text, -1, nullptr, 0, nullptr, nullptr);
	if(!size) return false;
	out.resize(size);
	return 0 != WideCharToMultiByte(cp, WC_COMPOSITECHECK|WC_SEPCHARS, text, -1, const_cast<char *>(out.data()), size, nullptr, nullptr);
}
