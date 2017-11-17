#ifndef WENDER_APP_H
#define WENDER_APP_H

#include <wx/wx.h>

struct App : public wxApp
{
	virtual bool OnInit();
};

#endif // WENDER_APP_H
