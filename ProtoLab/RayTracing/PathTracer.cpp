#include "pch.h"
#include "PathTracer.h"


static void RTCErrorHandler(const RTCError code, const char* str)
{
	ASSERT(false, str);
}

PathTracer::PathTracer()
{
	m_Device = rtcNewDevice(nullptr);
	ASSERT(rtcDeviceGetError(m_Device) == RTC_NO_ERROR, "Device creation failed");

	rtcDeviceSetErrorFunction(m_Device, RTCErrorHandler);
}

PathTracer::~PathTracer()
{
	rtcDeleteDevice(m_Device);
}

void PathTracer::InitializeStaticScene()
{
	m_Scene = rtcDeviceNewScene(m_Device, RTC_SCENE_STATIC, RTC_INTERSECT_STREAM);
}

void PathTracer::FinalizeScene()
{
	if (m_Scene != nullptr)
	{
		rtcDeleteScene(m_Scene);
	}
}

void PathTracer::BeginUpdateScene()
{
}

void PathTracer::EndUpdateScene()
{
	rtcCommit(m_Scene);
}
