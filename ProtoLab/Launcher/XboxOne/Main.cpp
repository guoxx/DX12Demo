//
// Main.cpp
//

#include "pch.h"
#include "../DX12SponzaDemo.h"

#include <ppltasks.h>

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace DirectX;

ref class ViewProvider sealed : public IFrameworkView
{
public:
    ViewProvider() :
        m_exit(false)
    {
    }

    // IFrameworkView methods
    virtual void Initialize(CoreApplicationView^ applicationView)
    {
        applicationView->Activated +=
            ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &ViewProvider::OnActivated);

        CoreApplication::Suspending +=
            ref new EventHandler<SuspendingEventArgs^>(this, &ViewProvider::OnSuspending);

        CoreApplication::Resuming +=
            ref new EventHandler<Platform::Object^>(this, &ViewProvider::OnResuming);

        m_pSample.reset(new DX12SponzaDemo(1920, 1080, L"DX12SponzaDemo"));
    }

    virtual void Uninitialize()
    {
        m_pSample.reset();
    }

    virtual void SetWindow(CoreWindow^ window)
    {
		window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &ViewProvider::OnKeyDown);
		window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &ViewProvider::OnKeyUp);
        window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &ViewProvider::OnWindowClosed);

        m_pSample->OnInit(reinterpret_cast<IUnknown*>(window));
    }

    virtual void Load(Platform::String^ entryPoint)
    {
    }

    virtual void Run()
    {
        while (!m_exit)
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			m_pSample->Tick();
        }

		m_pSample->OnDestroy();
    }

protected:
    // Event handlers
    void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
    {
        CoreWindow::GetForCurrentThread()->Activate();
    }

    void OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
    {
        SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

        create_task([this, deferral]()
        {
            m_pSample->OnSuspending();

            deferral->Complete();
        });
    }

    void OnResuming(Platform::Object^ sender, Platform::Object^ args)
    {
        m_pSample->OnResuming();
    }

    void OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
    {
        m_exit = true;
    }

	void OnKeyDown(CoreWindow^ window, KeyEventArgs^ keyEventArgs)
	{
		if (static_cast<UINT>(keyEventArgs->VirtualKey) < 256)
		{
			m_pSample->OnKeyDown(static_cast<UINT8>(keyEventArgs->VirtualKey));
		}
	}

	void OnKeyUp(CoreWindow^ window, KeyEventArgs^ keyEventArgs)
	{
		if (static_cast<UINT>(keyEventArgs->VirtualKey) < 256)
		{
			m_pSample->OnKeyUp(static_cast<UINT8>(keyEventArgs->VirtualKey));
		}
	}

private:
    bool						m_exit;
    std::unique_ptr<DXSample>   m_pSample;
};

ref class ViewProviderFactory : IFrameworkViewSource
{
public:
    virtual IFrameworkView^ CreateView()
    {
        return ref new ViewProvider();
    }
};


// Entry point
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ argv)
{
    UNREFERENCED_PARAMETER(argv);

    auto viewProviderFactory = ref new ViewProviderFactory();
    CoreApplication::Run(viewProviderFactory);
    return 0;
}
