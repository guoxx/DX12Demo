#include "pch.h"
#include "DXSample.h"

using namespace Microsoft::WRL;

DXSample::DXSample(UINT width, UINT height, std::wstring name)
	: m_Width(width)
	, m_Height(height)
	, m_Title(name)
{
	m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

DXSample::~DXSample()
{
}

void DXSample::OnInit(GFX_WHND hwnd)
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
		}
	}
}
