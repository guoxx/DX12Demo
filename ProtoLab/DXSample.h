#pragma once

#include "StepTimer.h"

class DXSample
{
public:
	DXSample(uint32_t width, uint32_t height, std::wstring name);
	virtual ~DXSample();

	void Tick();

	virtual void OnInit(GFX_HWND hwnd);
	virtual void OnUpdate(DX::StepTimer const& timer) = 0;
	virtual void OnRender() = 0;
	virtual void OnFlip() = 0;
	virtual void OnDestroy() = 0;

	// Samples override the event handlers to handle specific messages.
	virtual void OnKeyDown(uint8_t /*key*/)   {}
	virtual void OnKeyUp(uint8_t /*key*/)     {}

#ifdef _XBOX_ONE
	virtual void OnSuspending() = 0;
	virtual void OnResuming() = 0;
#endif

	// Accessors.
	uint32_t GetWidth() const           { return m_Width; }
	uint32_t GetHeight() const          { return m_Height; }
	const wchar_t* GetTitle() const		{ return m_Title.c_str(); }
	const GFX_HWND GetHwnd() const		{ return m_Hwnd; }

	void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

protected:
	// Viewport dimensions.
	uint32_t m_Width;
	uint32_t m_Height;
	float m_AspectRatio;
	GFX_HWND m_Hwnd;

    // Game state
	int64_t m_Frame;
	DX::StepTimer m_Timer;

    bool m_RenderDoc;
    bool m_DebugGfx;

private:
	// Window title.
	std::wstring m_Title;
};

// PIX event colors
const DWORD EVT_COLOR_FRAME = PIX_COLOR_INDEX(1);
const DWORD EVT_COLOR_UPDATE = PIX_COLOR_INDEX(2);
const DWORD EVT_COLOR_RENDER = PIX_COLOR_INDEX(3);
const DWORD EVT_COLOR_PRESENT = PIX_COLOR_INDEX(4);

