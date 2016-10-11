#include "pch.h"
#include "Scene.h"
#include "Model.h"

#include "Lights/PointLight.h"
#include "Lights/DirectionalLight.h"


Scene::Scene()
{
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

std::vector<std::shared_ptr<Model>> Scene::GetModels()
{
	return m_Models;
}

std::vector<std::shared_ptr<PointLight>> Scene::GetPointLights()
{
	return m_PointLights;
}

std::vector<std::shared_ptr<DirectionalLight>> Scene::GetDirectionalLights()
{
	return m_DirectionalLights;
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

void Scene::AttachDirectionalLight(std::shared_ptr<DirectionalLight> light)
{
	m_DirectionalLights.push_back(light);
}

void Scene::DetachDirectionalLight(std::shared_ptr<DirectionalLight> light)
{
}
