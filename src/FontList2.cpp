#include "FontList2.h"

#include <wx/log.h>
#include <sstream>
#include "FontList.h"
#include "ProgressBar.h"
#include "util.h"
#include "lib_font.h"

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
	__attribute__((unused)) auto addFontListFont = [this](char const * face)
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
