#ifndef WENDER_FONT_LIST_H
#define WENDER_FONT_LIST_H

#include <wx/wx.h>
#include <wx/htmllbox.h>

struct FontList : public wxSimpleHtmlListBox
{
	FontList(wxWindow * parent);

private:
	virtual void OnDrawSeparator(wxDC & dc, wxRect & r, size_t n) const;
};

#endif // WENDER_FONT_LIST_H
