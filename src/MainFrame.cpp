#include "MainFrame.h"

#include "resource.h"
#include "util.h"

static void CreateAndInstallMenuBar(MainFrame * frame)
{
	wxMenu * menuFile = new wxMenu;
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenuBar * menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");

	frame->SetMenuBar(menuBar);
}

static void CreateAndInstallToolBar(MainFrame * frame)
{
	wxToolBar * toolBar = new wxToolBar(frame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL);

	toolBar->AddTool(wxID_PASTE, wxT("Paste"), util::load_png(ID_PNG_PASTE_16));

	toolBar->Realize();
	frame->SetToolBar(toolBar);
}

static void CreateAndInstallFontList(MainFrame * frame)
{
	// frame->fontList = new FontList(frame);
	frame->fontList2 = new FontList2(frame);
	frame->fontList = frame->fontList2->fontList;
}

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, APP_NAME, wxDefaultPosition, wxSize(800,400))
{
	SetIcon(util::load_ico(ID_ICO_APPLICATION));

	CreateStatusBar();
	SetStatusText(" (use F8 to trigger user action) ");

	CreateAndInstallMenuBar(this);
	CreateAndInstallToolBar(this);
	CreateAndInstallFontList(this);
	// PopulateFontList(this);

	wxAcceleratorEntry entries[2] = {
		{wxACCEL_NORMAL, WXK_ESCAPE, wxID_EXIT},
		{wxACCEL_NORMAL, WXK_F8, 4711},
	};

	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);

	Bind(wxEVT_MENU, &MainFrame::OnF8, this, 4711);

	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_TOOL, &MainFrame::OnPaste, this, wxID_PASTE);
}

void MainFrame::OnF8(wxCommandEvent & e)
{
	// wxLogMessage("f8");
	SetStatusText("");
	// fontList2->ShowProgressBar(!fontList2->IsProgressBarShown());
	fontList2->LoadFonts();
}

void MainFrame::OnExit(wxCommandEvent & e)
{
	Close(true);
}

void MainFrame::OnPaste(wxCommandEvent & e)
{
	std::cout << "OnPaste" << std::endl;
}
