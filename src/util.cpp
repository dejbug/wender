#include "util.h"

util::ResourceLock::ResourceLock(WORD id, HINSTANCE hi)
{
	HRSRC res_find_handle = FindResource(hi, MAKEINTRESOURCE(id), RT_RCDATA);
	assert(!!res_find_handle);

	size = (size_t) SizeofResource(hi, res_find_handle);

	HGLOBAL handle = LoadResource(hi, res_find_handle);
	assert(!!handle);

	LPVOID res_lock_handle = LockResource(handle);
	assert(!!res_lock_handle);

	data = (char const *) res_lock_handle;
}

util::ResourceLock::~ResourceLock()
{
	if (handle) FreeResource(handle);
}

wxIcon util::load_ico(WORD id)
{
	HICON hicon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(id));
	assert(!!hicon);

	wxIcon icon;
	icon.CreateFromHICON(hicon);
	return icon;
}

wxBitmap util::load_png(WORD id)
{
	ResourceLock reslock(id);
	return wxBitmap::NewFromPNGData(reslock.data, reslock.size);
}

bool util::ConvertUnicodeToAnsi(std::string & out, wchar_t const * text, UINT cp)
{
	out.erase();
	const int size = WideCharToMultiByte(cp, WC_COMPOSITECHECK|WC_SEPCHARS, text, -1, nullptr, 0, nullptr, nullptr);
	if(!size) return false;
	out.resize(size);
	return 0 != WideCharToMultiByte(cp, WC_COMPOSITECHECK|WC_SEPCHARS, text, -1, const_cast<char *>(out.data()), size, nullptr, nullptr);
}
