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

void PathTracer::AddTriangleMesh(float* pVertexBufferCopy, int32_t* pIndexBufferCopy, int32_t numTriangles)
{
	uint32_t numVertices = numTriangles * 3;
	uint32_t geoId = rtcNewTriangleMesh(m_Scene, RTC_GEOMETRY_STATIC, numTriangles, numVertices);

	void* pVertexBuffer = rtcMapBuffer(m_Scene, geoId, RTC_VERTEX_BUFFER);
	memcpy(pVertexBuffer, pVertexBufferCopy, sizeof(float) * 4 * numVertices);
	rtcUnmapBuffer(m_Scene, geoId, RTC_VERTEX_BUFFER);

	void* pIndexBuffer = rtcMapBuffer(m_Scene, geoId, RTC_INDEX_BUFFER);
	memcpy(pIndexBuffer, pIndexBufferCopy, sizeof(int32_t) * numVertices);
	rtcUnmapBuffer(m_Scene, geoId, RTC_INDEX_BUFFER);

	m_Geometries.push_back(geoId);
}
