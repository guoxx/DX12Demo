#pragma once

#include <embree2/rtcore.h>
#include <embree2/rtcore_ray.h>

class PathTracer
{
public:
	PathTracer();
	~PathTracer();

	void InitializeStaticScene();
	void FinalizeScene();

	void BeginUpdateScene();
	void EndUpdateScene();

	// only triangle list is supported for the moment
	void AddTriangleMesh(float* pVertexBufferCopy, int32_t* pIndexBufferCopy, int32_t numTriangles);

private:
	RTCDevice m_Device;
	RTCScene m_Scene;
	std::vector<uint32_t> m_Geometries;
};
