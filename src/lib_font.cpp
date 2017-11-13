#include "lib_font.h"
#include <stdio.h>
#include <algorithm>

lib::font::EnumFontInfo::EnumFontInfo(
		ENUMLOGFONTEXW elfe, NEWTEXTMETRICEXW ntme, int fontType)
		: elfe(elfe), ntme(ntme), fontType(fontType)
{
}

lib::font::EnumFontInfoLoader::EnumFontInfoLoader(
		HDC dc, lib::font::EnumFontInfo & efi)
		: dc(dc)
{
	handle = CreateFontIndirectW(&efi.elfe.elfLogFont);
	old = SelectObject(dc, handle);
}

lib::font::EnumFontInfoLoader::~EnumFontInfoLoader()
{
	if(handle) {
		SelectObject(dc, old);
		DeleteObject(handle);
	}
}

void lib::font::EnumFontInfoLoader::detach()
{
	handle = nullptr;
}

struct ListFontsContext
{
	size_t index = 0;
	std::vector<lib::font::EnumFontInfo> * out = nullptr;
	size_t maxCount = 0xffffffff;
	bool onlyTrueType = false;
};

static int CALLBACK list_fonts_cb(
		LOGFONTW const * lpelf, TEXTMETRICW const * lptm,
		unsigned long int fontType, LPARAM lParam)
{
	auto lpelfe = (ENUMLOGFONTEXW const *) lpelf;
	auto lpntme = (NEWTEXTMETRICEXW const *) lptm;
	ListFontsContext & ctx = * (ListFontsContext *) lParam;

	if (!ctx.onlyTrueType || TRUETYPE_FONTTYPE == fontType)
		ctx.out->push_back(
			lib::font::EnumFontInfo(*lpelfe, *lpntme, fontType));

	++ctx.index;

	return ctx.out->size() < ctx.maxCount ? TRUE : FALSE;
}

void lib::font::list_fonts(std::vector<lib::font::EnumFontInfo> & out,
	BYTE charSet, bool onlyTrueType, HDC hdc, size_t maxCount)
{
	ListFontsContext ctx;
	ctx.out = &out;
	ctx.maxCount = maxCount;
	ctx.onlyTrueType = onlyTrueType;

	// if(!hdc) hdc = GetDC(nullptr);

	LOGFONTW lf;
	memset(&lf, 0, sizeof(LOGFONTW));
	lf.lfCharSet = charSet;
	EnumFontFamiliesExW(hdc, &lf, list_fonts_cb, (LPARAM) &ctx, 0);
}

static bool sort_fonts_cb_1(
		lib::font::EnumFontInfo const & a,
		lib::font::EnumFontInfo const & b)
{
	return _stricmp(
		(char const *) a.elfe.elfFullName,
		(char const *) b.elfe.elfFullName) < 0;
}

void lib::font::sort_fonts(OUT std::vector<EnumFontInfo> & ff)
{
	std::sort(ff.begin(), ff.end(), sort_fonts_cb_1);
}

void lib::font::print_font_info(lib::font::EnumFontInfo & efi)
{
	char const * ft = "?";

	switch(efi.fontType)
	{
		case DEVICE_FONTTYPE: ft = "dev"; break;
		case RASTER_FONTTYPE: ft = "ras"; break;
		case TRUETYPE_FONTTYPE: ft = "ttf"; break;
	}

	printf("%s | %ls | %ls | %d | %ls\n", ft,
		efi.elfe.elfLogFont.lfFaceName,
		efi.elfe.elfStyle,
		efi.elfe.elfLogFont.lfCharSet,
		efi.elfe.elfScript);
}

void lib::font::draw_font_label(HDC dc, RECT & rc, EnumFontInfo & fi)
{
	char const * text = (char const *) fi.elfe.elfFullName;
	size_t const text_len = strlen(text);

	lib::font::EnumFontInfoLoader efil(dc, fi);

	ExtTextOut(dc, rc.left, rc.top,
		0, nullptr, text, text_len, nullptr);
}

int lib::font::get_sysfont_height()
{
	TEXTMETRIC tm;
	HDC dc = GetDC(nullptr);
	SelectObject(dc, GetStockObject(ANSI_FIXED_FONT));
	GetTextMetrics(dc, &tm);
	ReleaseDC(nullptr, dc);
	return tm.tmHeight;
}
