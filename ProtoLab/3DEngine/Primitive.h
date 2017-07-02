#pragma once

class Mesh;
class Material;

class Primitive
{
	friend class Model;
	friend class Mesh;

public:
	Primitive();
	~Primitive();

private:
	int32_t m_IndexCount;
	int32_t m_StartIndexLocation;
	std::weak_ptr<Mesh> m_Mesh;
	std::shared_ptr<Material> m_Material;
};

