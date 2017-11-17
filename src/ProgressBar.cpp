#include "ProgressBar.h"

#include <wx/log.h>

ProgressBar::ProgressBar(wxWindow * parent) : wxPanel(parent, wxID_ANY)
{
	gauge = new wxGauge(this, wxID_ANY, 100);
	cancel = new wxButton(this, wxID_ANY, "Cancel", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

	wxSize bs = cancel->GetSize();
	gauge->SetSize(wxSize(200,bs.y));

	Bind(wxEVT_SIZE, &ProgressBar::OnSize, this);
	Bind(wxEVT_BUTTON, &ProgressBar::OnCancel, this);
}

void ProgressBar::OnCancel(wxCommandEvent & e)
{
	wxLogMessage("ProgressBar::OnCancel");
	gauge->SetFocus();
	e.Skip();
}

void ProgressBar::OnSize(wxSizeEvent & e)
{
	e.Skip();
	wxLogMessage("ProgressBar::OnSize");

	wxSize cs = GetClientSize();
	wxSize s = gauge->GetSize();

	int const x = (cs.x - s.x) >> 1;
	int const y = (cs.y - s.y) >> 1;

	gauge->SetPosition(wxPoint(x, y));
	cancel->SetPosition({x+s.x+4, y});
}
