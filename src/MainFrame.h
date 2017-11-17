#ifndef WENDER_MAIN_FRAME_H
#define WENDER_MAIN_FRAME_H

#include <wx/wx.h>
#include "FontList.h"
#include "FontList2.h"

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

#endif // WENDER_MAIN_FRAME_H
