#include "App.h"

#include <wx/log.h>
#include <wx/sysopt.h>
#include "MainFrame.h"

bool App::OnInit()
{
	wxLog * logger = new wxLogStream(&std::cerr);
	wxLog::SetActiveTarget(logger);

#ifndef __WXMSW__
	wxLogError("This is an MS Windows app. Sorry.");
	return false;
#endif

	wxInitAllImageHandlers();
	wxSystemOptions::SetOption("msw.remap", 0);

	auto frame = new MainFrame();
	frame->Centre();
	frame->Show(true);
	return true;
}
