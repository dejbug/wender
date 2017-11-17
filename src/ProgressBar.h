#ifndef WENDER_PROGRESS_BAR_H
#define WENDER_PROGRESS_BAR_H

#include <wx/wx.h>
#include <wx/gauge.h>

struct ProgressBar : public wxPanel
{
	wxGauge * gauge = nullptr;
	wxButton * cancel = nullptr;

	ProgressBar(wxWindow * parent);

protected:
	virtual void OnCancel(wxCommandEvent & e);
	virtual void OnSize(wxSizeEvent & e);
};

#endif // WENDER_PROGRESS_BAR_H
