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

std::vector<std::shared_ptr<ILight>> Scene::GetLights()
{
	return m_Lights;
}

void Scene::AttachModel(std::shared_ptr<Model> model)
{
	m_Models.push_back(model);
}

void Scene::DetachModel(std::shared_ptr<Model> model)
{
	// TODO: 
}

void Scene::AttachLight(std::shared_ptr<ILight> light)
{
	m_Lights.push_back(light);
}

void Scene::DetachLight(std::shared_ptr<ILight> light)
{
	// TODO:
}
