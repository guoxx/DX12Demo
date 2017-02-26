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

private:
	RTCDevice m_Device;
	RTCScene m_Scene;
};
