#include "pch.h"
#include "Scene.h"
#include "Model.h"

#include "Lights/PointLight.h"
#include "Lights/DirectionalLight.h"


Scene::Scene()
{
    m_Turbidity = 2.0f;
    m_GroundAlbedo = Color(1.0f, 1.0f, 1.0f);
    m_SunCoord = SphericalCoordinates::FromThetaAndPhi(DirectX::XMConvertToRadians(45), DirectX::XMConvertToRadians(20));

    m_Sky = std::make_shared<Sky>();
    m_Sky->UpdateEnvironmentMap(m_Turbidity, m_GroundAlbedo, m_SunCoord);

    m_SunLight = std::make_shared<DirectionalLight>();
    DirectX::XMVECTOR sunDir = SphericalCoordinates::ToSphere(m_SunCoord);
    m_SunLight->SetDirection(-DirectX::XMVectorGetX(sunDir), -DirectX::XMVectorGetY(sunDir), -DirectX::XMVectorGetZ(sunDir));
}

Scene::~Scene()
{
}

void Scene::Update(double delta)
{
	for (auto mod : m_Models)
	{
		mod->Update();
	}
}

std::shared_ptr<Sky> Scene::GetSky() const
{
    return m_Sky;
}

std::shared_ptr<DirectionalLight> Scene::GetSunLight() const
{
	return m_SunLight;
}

std::vector<std::shared_ptr<Model>> Scene::GetModels() const
{
	return m_Models;
}

std::vector<std::shared_ptr<PointLight>> Scene::GetPointLights() const
{
	return m_PointLights;
}

void Scene::AttachModel(std::shared_ptr<Model> model)
{
	m_Models.push_back(model);
}

void Scene::DetachModel(std::shared_ptr<Model> model)
{
	// TODO: 
}

void Scene::AttachPointLight(std::shared_ptr<PointLight> light)
{
	m_PointLights.push_back(light);
}

void Scene::DetachPointLight(std::shared_ptr<PointLight> light)
{
	// TODO:
}
