#pragma once

class Actor
{
public:
	enum ACTOR_DIRTY_FLAGS
	{
		ACTOR_DIRTY_LOCAL_MATRIX = 0x01 << 0,
		ACTOR_DIRTY_WORLD_MATRIX = 0x01 << 1,
	};

	Actor();
	virtual ~Actor();

	void Update();

	void SetTranslation(DirectX::XMVECTOR translation);
	DirectX::XMVECTOR GetTranslation() const;

	void SetRotationPitchYawRoll(float pitch, float yaw, float roll);
	void SetRotationPitchYawRoll(DirectX::XMVECTOR pitchYawRollInDegrees);
	DirectX::XMVECTOR GetRotationPitchYawRoll() const;

	void SetScale(DirectX::XMVECTOR scale);
	DirectX::XMVECTOR GetScale() const;

	DirectX::XMMATRIX GetLocalMatrix() const;
	DirectX::XMMATRIX GetWorldMatrix() const;

	// get directions in local space
	DirectX::XMVECTOR GetUp() const;
	DirectX::XMVECTOR GetDown() const;
	DirectX::XMVECTOR GetLeft() const;
	DirectX::XMVECTOR GetRight() const;
	DirectX::XMVECTOR GetForward() const;
	DirectX::XMVECTOR GetBackward() const;

	// transformation in local space
	void Move(DirectX::XMVECTOR direction, float distance);
	// rotate in local space, angle is measured by degrees
	void RotatePitchYawRoll(float pitch, float yaw, float roll);

protected:

	void UpdateLocalMatrixImmediate();
	void UpdateWorldMatrixImmediate();

	void UpdateLocalMatrixDeferred();
	void UpdateWorldMatrixDeferred();

private:

	std::weak_ptr<Actor> m_Parent;
	std::vector<std::shared_ptr<Actor>> m_Children;

protected:

	uint32_t m_DirtyFlags;

	DirectX::XMFLOAT4 m_Translation;
	DirectX::XMFLOAT4 m_RotationPitchYawRoll;
	DirectX::XMFLOAT4 m_Scale;

	DirectX::XMFLOAT4X4 m_LocalMatrix;
	DirectX::XMFLOAT4X4 m_WorldMatrix;
};

