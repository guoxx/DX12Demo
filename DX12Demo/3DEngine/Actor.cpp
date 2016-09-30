#include "pch.h"
#include "Actor.h"


Actor::Actor()
	: m_Parent{}
	, m_Children{}
	, m_DirtyFlags{0}
{
	m_Translation = DirectX::XMVectorZero();
	m_RotationPitchYawRoll = DirectX::XMVectorZero();
	m_Scale = DirectX::XMVECTOR{1, 1, 1};

	DirectX::XMStoreFloat4x4(&m_LocalMatrix, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&m_WorldMatrix, DirectX::XMMatrixIdentity());
}

Actor::~Actor()
{
}

void Actor::Update()
{
	if (m_DirtyFlags & ACTOR_DIRTY_LOCAL_MATRIX)
	{
		UpdateLocalMatrixImmediate();
	}

	if (m_DirtyFlags & ACTOR_DIRTY_WORLD_MATRIX)
	{
		UpdateWorldMatrixImmediate();
	}

	for (auto aActor : m_Children)
	{
		aActor->Update();
	}
}

void Actor::SetPosition(DirectX::XMVECTOR position)
{
	m_Translation = position;
	UpdateLocalMatrixDeferred();
}

DirectX::XMVECTOR Actor::GetPosition() const
{
	return m_Translation;
}

void Actor::SetRotationPitchYawRoll(float pitch, float yaw, float roll)
{
	SetRotationPitchYawRoll(DirectX::XMVECTOR{pitch, yaw, roll, 0.0f});
}

void Actor::SetRotationPitchYawRoll(DirectX::XMVECTOR pitchYawRollInDegrees)
{
	float degreeToRadian = -DirectX::XM_PI / 180.0f;
	m_RotationPitchYawRoll = DirectX::XMVectorMultiply(pitchYawRollInDegrees, DirectX::XMVECTOR{degreeToRadian, degreeToRadian, degreeToRadian, degreeToRadian});
	UpdateLocalMatrixDeferred();
}

DirectX::XMVECTOR Actor::GetRotationPitchYawRoll() const
{
	return m_RotationPitchYawRoll;
}

void Actor::SetScale(DirectX::XMVECTOR scale)
{
	m_Scale = scale;
	UpdateLocalMatrixDeferred();
}

DirectX::XMVECTOR Actor::GetScale() const
{
	return m_Scale;
}

DirectX::XMMATRIX Actor::GetLocalMatrix() const
{
	assert(!(m_DirtyFlags & Actor::ACTOR_DIRTY_LOCAL_MATRIX));
	DirectX::XMMATRIX mLocal = DirectX::XMLoadFloat4x4(&m_LocalMatrix);
	return mLocal;
}

DirectX::XMMATRIX Actor::GetWorldMatrix() const
{
	assert(!(m_DirtyFlags & Actor::ACTOR_DIRTY_WORLD_MATRIX));
	DirectX::XMMATRIX mWorld = DirectX::XMLoadFloat4x4(&m_WorldMatrix);
	return mWorld;
}

DirectX::XMVECTOR Actor::GetUp() const
{
	DirectX::XMVECTOR upDir{m_LocalMatrix._21, m_LocalMatrix._22, m_LocalMatrix._23, 0};
	return DirectX::XMVector3Normalize(upDir);
}

DirectX::XMVECTOR Actor::GetDown() const
{
	DirectX::XMVECTOR downDir{-m_LocalMatrix._21, -m_LocalMatrix._22, -m_LocalMatrix._23, 0};
	return DirectX::XMVector3Normalize(downDir);
}

DirectX::XMVECTOR Actor::GetLeft() const
{
	DirectX::XMVECTOR leftDir{m_LocalMatrix._11, m_LocalMatrix._12, m_LocalMatrix._13, 0};
	return DirectX::XMVector3Normalize(leftDir);
}

DirectX::XMVECTOR Actor::GetRight() const
{
	DirectX::XMVECTOR rightDir{-m_LocalMatrix._11, -m_LocalMatrix._12, -m_LocalMatrix._13, 0};
	return DirectX::XMVector3Normalize(rightDir);
}

DirectX::XMVECTOR Actor::GetForward() const
{
	// right hand coordinate
	DirectX::XMVECTOR forwardDir{-m_LocalMatrix._31, -m_LocalMatrix._32, -m_LocalMatrix._33, 0};
	return DirectX::XMVector3Normalize(forwardDir);
}

DirectX::XMVECTOR Actor::GetBackward() const
{
	// right hand coordinate
	DirectX::XMVECTOR backwardDir{m_LocalMatrix._31, m_LocalMatrix._32, m_LocalMatrix._33, 0};
	return DirectX::XMVector3Normalize(backwardDir);
}

void Actor::Move(DirectX::XMVECTOR direction, float distance)
{
	m_Translation = DirectX::XMVectorMultiplyAdd(direction, DirectX::XMVECTOR{distance, distance, distance, distance}, m_Translation);
	UpdateLocalMatrixDeferred();
}

void Actor::RotatePitchYawRoll(float pitch, float yaw, float roll)
{
	DirectX::XMVECTOR rotation = DirectX::XMVECTOR{DirectX::XMConvertToRadians(pitch), DirectX::XMConvertToRadians(yaw), DirectX::XMConvertToRadians(roll), 0};
	m_RotationPitchYawRoll = DirectX::XMVectorAdd(m_RotationPitchYawRoll, rotation);
	UpdateLocalMatrixDeferred();
}

void Actor::UpdateLocalMatrixImmediate()
{
	if (true)
	//if (_dirtyFlags & ACTOR_DIRTY_LOCAL_MATRIX)
	{
		DirectX::XMMATRIX mTrans = DirectX::XMMatrixTranslationFromVector(m_Translation);
		DirectX::XMMATRIX mRot = DirectX::XMMatrixRotationRollPitchYawFromVector(m_RotationPitchYawRoll);
		DirectX::XMMATRIX mScale = DirectX::XMMatrixScalingFromVector(m_Scale);
		// mRot * mTrans * mScale
		DirectX::XMMATRIX mLocal = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(mRot, mTrans), mScale);
		DirectX::XMStoreFloat4x4(&m_LocalMatrix, mLocal);

		m_DirtyFlags &= (~ACTOR_DIRTY_LOCAL_MATRIX);
	}
}

void Actor::UpdateWorldMatrixImmediate()
{
	if (true)
	//if (_dirtyFlags & ACTOR_DIRTY_WORLD_MATRIX)
	{
		auto parent = m_Parent.lock();
		if (parent)
		{
			parent->UpdateWorldMatrixImmediate();	
			DirectX::XMMATRIX parentWorldMatrix = parent->GetWorldMatrix();
			UpdateLocalMatrixImmediate();
			DirectX::XMMATRIX currentLocalMatrix = GetLocalMatrix();
			DirectX::XMMATRIX mWorld = DirectX::XMMatrixMultiply(currentLocalMatrix, parentWorldMatrix);
			DirectX::XMStoreFloat4x4(&m_WorldMatrix, mWorld);
		}
		else
		{
			UpdateLocalMatrixImmediate();
			m_WorldMatrix = m_LocalMatrix;
		}

		m_DirtyFlags &= (~ACTOR_DIRTY_WORLD_MATRIX);
	}
}

void Actor::UpdateLocalMatrixDeferred()
{
	UpdateLocalMatrixImmediate();
	UpdateWorldMatrixImmediate();
	//_dirtyFlags |= Actor::ACTOR_DIRTY_LOCAL_MATRIX;	
	//_dirtyFlags |= Actor::ACTOR_DIRTY_WORLD_MATRIX;
}

void Actor::UpdateWorldMatrixDeferred()
{
	UpdateWorldMatrixImmediate();
	//_dirtyFlags |= Actor::ACTOR_DIRTY_WORLD_MATRIX;	
}
