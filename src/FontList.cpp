#include "FontList.h"

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
