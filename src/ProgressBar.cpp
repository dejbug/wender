#include "ProgressBar.h"

#include <wx/log.h>

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
