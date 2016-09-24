#pragma once

class DXSample
{
public:
	DXSample(uint32_t width, uint32_t height, std::wstring name);
	virtual ~DXSample();

	virtual void OnInit(GFX_WHND hwnd);
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;

	// Samples override the event handlers to handle specific messages.
	virtual void OnKeyDown(uint8_t /*key*/)   {}
	virtual void OnKeyUp(uint8_t /*key*/)     {}

#ifdef __XBOX_ONE__
	virtual void OnSuspending() = 0;
	virtual void OnResuming() = 0;
#endif

	// Accessors.
	uint32_t GetWidth() const           { return m_Width; }
	uint32_t GetHeight() const          { return m_Height; }
	const wchar_t* GetTitle() const		{ return m_Title.c_str(); }
	const GFX_WHND GetHwnd() const		{ return m_Hwnd; }

	void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

protected:
	// Viewport dimensions.
	uint32_t m_Width;
	uint32_t m_Height;
	float m_AspectRatio;
	GFX_WHND m_Hwnd;

private:
	// Window title.
	std::wstring m_Title;
};
