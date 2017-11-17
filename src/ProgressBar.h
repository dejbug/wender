#ifndef WENDER_PROGRESS_BAR_H
#define WENDER_PROGRESS_BAR_H

#include <wx/wx.h>
#include <wx/gauge.h>

struct ProgressBar : public wxPanel
{
	wxGauge * gauge = nullptr;

	ProgressBar(wxWindow * parent);

protected:
	virtual void OnSize(wxSizeEvent & e);
};

#endif // WENDER_PROGRESS_BAR_H
