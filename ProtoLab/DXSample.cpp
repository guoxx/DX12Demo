#include "pch.h"
#include "DXSample.h"

using namespace Microsoft::WRL;

DXSample::DXSample(UINT width, UINT height, std::wstring name)
	: m_Width(width)
	, m_Height(height)
	, m_Title(name)
	, m_Frame(0)
    , m_RenderDoc(false)
    , m_DebugGfx(false)
{
	m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

DXSample::~DXSample()
{
}

void DXSample::Tick()
{
    PIXBeginEvent(EVT_COLOR_FRAME, L"Frame %I64u", m_Frame);

    m_Timer.Tick([&]()
    {
        OnUpdate(m_Timer);
    });

	OnRender();
	OnFlip();

    PIXEndEvent();
    m_Frame++;
}

void DXSample::OnInit(GFX_HWND hwnd)
{
	m_Hwnd = hwnd;
}

// Helper function for parsing any supplied command line args.
_Use_decl_annotations_
void DXSample::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
	for (int i = 1; i < argc; ++i)
	{
		if (_wcsnicmp(argv[i], L"-renderdoc", wcslen(argv[i])) == 0 || 
			_wcsnicmp(argv[i], L"/renderdoc", wcslen(argv[i])) == 0)
		{
			m_Title = m_Title + L" (RenderDoc)";
            m_RenderDoc = true;
		}

		if (_wcsnicmp(argv[i], L"-debuggfx", wcslen(argv[i])) == 0 || 
			_wcsnicmp(argv[i], L"/debuggfx", wcslen(argv[i])) == 0)
		{
			m_Title = m_Title + L" (DebugGfx)";
            m_DebugGfx = true;
		}
	}
}
