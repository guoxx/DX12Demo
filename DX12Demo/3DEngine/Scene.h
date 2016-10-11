#pragma once

class Model;
class PointLight;
class DirectionalLight;

class Scene
{
public:
	Scene();
	~Scene();

	std::vector<std::shared_ptr<Model>> GetModels();
	std::vector<std::shared_ptr<PointLight>> GetPointLights();
	std::vector<std::shared_ptr<DirectionalLight>> GetDirectionalLights();

	void AttachModel(std::shared_ptr<Model> model);
	void DetachModel(std::shared_ptr<Model> model);

	void AttachPointLight(std::shared_ptr<PointLight> light);
	void DetachPointLight(std::shared_ptr<PointLight> light);

	void AttachDirectionalLight(std::shared_ptr<DirectionalLight> light);
	void DetachDirectionalLight(std::shared_ptr<DirectionalLight> light);

	void Update(double delta);

private:
	std::vector<std::shared_ptr<PointLight>> m_PointLights;
	std::vector<std::shared_ptr<DirectionalLight>> m_DirectionalLights;
	std::vector<std::shared_ptr<Model>> m_Models;
};

