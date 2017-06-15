#include "pch.h"
#include "DirectionalLightFilter2D.h"

#include "../Camera.h"
#include "../RenderContext.h"
#include "../Lights/DirectionalLight.h"

#include "../../Shaders/CompiledShaders/DirectionalLight.h"


DirectionalLightFilter2D::DirectionalLightFilter2D(DX12Device* device)
{
	uint32_t indices[] = {0, 2, 1, 1, 2, 3};

	m_IndexBuffer = std::make_shared<DX12IndexBuffer>(device, sizeof(indices), 0, DXGI_FORMAT_R32_UINT);

	DX12GraphicsContextAutoExecutor executor;
	DX12GraphicsContext* pGfxContext = executor.GetGraphicsContext();

	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->UploadBuffer(m_IndexBuffer.get(), indices, sizeof(indices));
	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

    DX12RootSignatureDeserializer sigDeserialier{g_DirectionalLight_VS, sizeof(g_DirectionalLight_VS)};
	m_RootSig = sigDeserialier.Deserialize(device);

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromBin(DX12ShaderTypeVertex, g_DirectionalLight_VS, sizeof(g_DirectionalLight_VS));
	psoCompiler.SetShaderFromBin(DX12ShaderTypePixel, g_DirectionalLight_PS, sizeof(g_DirectionalLight_PS));
	psoCompiler.SetRoogSignature(m_RootSig.get());
	psoCompiler.SetRenderTargetFormat(GFX_FORMAT_HDR.RTVFormat);
	psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_UNKNOWN);
	psoCompiler.SetBlendState(CD3DX12::BlendAdditive());
	psoCompiler.SetDepthStencilState(CD3DX12::DepthStateDisabled());
	m_PSO = psoCompiler.Compile(device);
}

DirectionalLightFilter2D::~DirectionalLightFilter2D()
{
}

void DirectionalLightFilter2D::Apply(DX12GraphicsContext * pGfxContext, const RenderContext* pRenderContext, const DirectionalLight* pLight)
{
	pGfxContext->SetGraphicsRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());
	pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	HLSL::DirectionalLightConstants constants;

	DirectX::XMMATRIX mInvView = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetViewMatrix());
	DirectX::XMMATRIX mInvProj = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetProjMatrix());
	DirectX::XMStoreFloat4x4(&constants.mInvView, DirectX::XMMatrixTranspose(mInvView));
	DirectX::XMStoreFloat4x4(&constants.mInvProj, DirectX::XMMatrixTranspose(mInvProj));
	DirectX::XMStoreFloat4(&constants.CameraPosition, pRenderContext->GetCamera()->GetTranslation());

	DirectX::XMFLOAT4 direction = pLight->GetDirection();
	DirectX::XMFLOAT4 irradiance = pLight->GetIrradiance();
	constants.m_DirLight.m_Direction = DirectX::XMFLOAT3{ direction.x, direction.y, direction.z };
	constants.m_DirLight.m_Irradiance = DirectX::XMFLOAT3{ irradiance.x, irradiance.y, irradiance.z };

    for (int32_t cascadeIdx = 0; cascadeIdx < DirectionalLight::NUM_CASCADED_SHADOW_MAP; ++cascadeIdx)
    {
        DirectX::XMMATRIX mLightView;
        DirectX::XMMATRIX mLightProj;
        pLight->GetViewAndProjMatrix(cascadeIdx, &mLightView, &mLightProj);
        DirectX::XMMATRIX mLightViewProj = DirectX::XMMatrixMultiply(mLightView, mLightProj);
        DirectX::XMStoreFloat4x4(&constants.m_DirLight.m_mViewProj[cascadeIdx], DirectX::XMMatrixTranspose(mLightViewProj));

        DirectX::XMMATRIX mInvLightViewProj = DirectX::XMMatrixInverse(nullptr, mLightViewProj);
        DirectX::XMStoreFloat4x4(&constants.m_DirLight.m_mInvViewProj[cascadeIdx], DirectX::XMMatrixTranspose(mInvLightViewProj));
    }

	constants.m_DirLight.m_ShadowMapTexId = -1;
	constants.m_DirLight.m_RSMIntensityTexId = -1;
	constants.m_DirLight.m_RSMNormalTexId = -1;

	if (g_RSMEnabled)
	{
		constants.m_RSM.m_Enabled = 1;
		constants.m_RSM.m_SampleRadius = g_RSMSampleRadius;
		constants.m_RSM.m_RSMFactor = g_RSMFactor;
		constants.m_RSM.m_RadiusEnd = g_RSMRadiusEnd;
	}
	else
	{
		constants.m_RSM.m_Enabled = 0;
	}

	constants.m_EVSM.m_Enabled = g_EVSMEnabled ? 1 : 0;
	constants.m_EVSM.m_PositiveExponent = g_EVSMPositiveExponent;
	constants.m_EVSM.m_NegativeExponent = g_EVSMNegativeExponent;
	constants.m_EVSM.m_LightBleedingReduction = g_LightBleedingReduction;
	constants.m_EVSM.m_VSMBias = g_VSMBias;

	pGfxContext->SetGraphicsRootDynamicConstantBufferView(0, &constants, sizeof(constants));
}

void DirectionalLightFilter2D::Draw(DX12GraphicsContext* pGfxContext)
{
	pGfxContext->DrawIndexed(6, 0);
}
