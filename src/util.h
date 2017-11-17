#ifndef WENDER_UTIL_H
#define WENDER_UTIL_H

#include <wx/wx.h>
#include <windows.h>

namespace util {

struct ResourceLock
{
	HGLOBAL handle = nullptr;
	char const * data = nullptr;
	size_t size = 0;

	ResourceLock(WORD id, HINSTANCE hi=GetModuleHandle(nullptr));
	~ResourceLock();
};

wxIcon load_ico(WORD id);

wxBitmap load_png(WORD id);

bool ConvertUnicodeToAnsi(std::string & out, wchar_t const * text, UINT cp=CP_ACP);

} // namespace util

#endif // WENDER_UTIL_H
