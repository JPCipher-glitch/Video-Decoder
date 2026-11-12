#ifndef LOADING_WIDGET_HPP
#define LOADING_WIDGET_HPP

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <stdint.h>

class LoadingWidget final
{
private:
    HWND widget{ 0 };
	HWND progressBar{ 0 };

	COLORREF captionColor;
	COLORREF textColor;

	uint8_t progress{ 0 };
	uint8_t progressGain{ 0 };

	bool inDarkMode{ false };

	void createWidget();
	void createProgressBar();
	void update();
public:
	LoadingWidget();
	~LoadingWidget();
};

#endif