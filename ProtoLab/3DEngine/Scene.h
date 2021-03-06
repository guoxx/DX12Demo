#pragma once
#include "Sky.h"
#include "SphericalCoordinates.h"

class Model;
class PointLight;
class DirectionalLight;

class Scene
{
public:
	Scene();
	~Scene();

    std::shared_ptr<Sky> GetSky() const;

	std::shared_ptr<DirectionalLight> GetSunLight() const;

	std::vector<std::shared_ptr<Model>> GetModels() const;
	std::vector<std::shared_ptr<PointLight>> GetPointLights() const;

	void AttachModel(std::shared_ptr<Model> model);
	void DetachModel(std::shared_ptr<Model> model);

	void AttachPointLight(std::shared_ptr<PointLight> light);
	void DetachPointLight(std::shared_ptr<PointLight> light);

	void Update(double delta);

private:
    float m_Turbidity;
    Color m_GroundAlbedo;
    SphericalCoordinates m_SunCoord;

    std::shared_ptr<Sky> m_Sky;
	std::shared_ptr<DirectionalLight> m_SunLight;

	std::vector<std::shared_ptr<PointLight>> m_PointLights;
	std::vector<std::shared_ptr<Model>> m_Models;
};

