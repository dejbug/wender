#ifndef WENDER_FONT_LIST2_H
#define WENDER_FONT_LIST2_H

#include <wx/wx.h>
#include <wx/thread.h>
#include <windows.h>
#include "FontList.h"
#include "ProgressBar.h"

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

#endif // WENDER_FONT_LIST2_H
