#pragma once

class Model;
class ILight;

class Scene
{
public:
	Scene();
	~Scene();

	std::vector<std::shared_ptr<Model>> GetModels();
	std::vector<std::shared_ptr<ILight>> GetLights();

	void AttachModel(std::shared_ptr<Model> model);
	void DetachModel(std::shared_ptr<Model> model);

	void AttachLight(std::shared_ptr<ILight> light);
	void DetachLight(std::shared_ptr<ILight> light);

	void Update(double delta);

private:
	std::vector<std::shared_ptr<ILight>> m_Lights;
	std::vector<std::shared_ptr<Model>> m_Models;
};

