//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "buffer\gbuffer.hpp"
#include "rendering\rendering_manager.hpp"
#include "logging\error.hpp"
#include "logging\exception.hpp"

// Include HLSL bindings.
#include "..\..\shaders\hlsl.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Definitions
//-----------------------------------------------------------------------------
namespace mage {

	GBuffer::GBuffer()
		: GBuffer(Pipeline::GetDevice()) {}

	GBuffer::GBuffer(ID3D11Device2 *device)
		: m_dsv(), m_rtvs{}, m_srvs{} {

		SetupBuffers(device);
	}

	void GBuffer::BindPacking(ID3D11DeviceContext2 *device_context) noexcept {
		// Collect the SRVs.
		ID3D11ShaderResourceView * const srvs[GetNumberOfSRVs()] = {};
		// Bind no SRVs.
		Pipeline::PS::BindSRVs(device_context,
			SLOT_SRV_GBUFFER_START, 
			static_cast< U32 >(GetNumberOfSRVs()), srvs);
		Pipeline::CS::BindSRVs(device_context,
			SLOT_SRV_GBUFFER_START, 
			static_cast< U32 >(GetNumberOfSRVs()), srvs);
		
		// Collect and clear the RTVs.
		ID3D11RenderTargetView *rtvs[GetNumberOfRTVs()];
		for (size_t i = 0; i < GetNumberOfRTVs(); ++i) {
			rtvs[i] = m_rtvs[i].Get();
			Pipeline::OM::ClearRTV(device_context, rtvs[i]);
		}
		// Clear the DSV.
		Pipeline::OM::ClearDSV(device_context, m_dsv.Get());
		// Bind the RTVs and DSV.
		Pipeline::OM::BindRTVsAndDSV(device_context, 
			static_cast< U32 >(GetNumberOfRTVs()), rtvs, m_dsv.Get());
	}

	void GBuffer::BindUnpacking(ID3D11DeviceContext2 *device_context) noexcept {
		
		// Bind no RTVs and no DSV.
		Pipeline::OM::BindRTVAndDSV(device_context, nullptr, nullptr);

		// Collect the SRVs.
		ID3D11ShaderResourceView *srvs[GetNumberOfSRVs()];
		for (size_t i = 0; i < GetNumberOfSRVs(); ++i) {
			srvs[i] = m_srvs[i].Get();
		}
		// Bind the SRVs.
		Pipeline::PS::BindSRVs(device_context,
			SLOT_SRV_GBUFFER_START, 
			static_cast< U32 >(GetNumberOfSRVs()), srvs);
		Pipeline::CS::BindSRVs(device_context, 
			SLOT_SRV_GBUFFER_START, 
			static_cast< U32 >(GetNumberOfSRVs()), srvs);
	}

	void GBuffer::SetupBuffers(ID3D11Device2 *device) {
		Assert(device);
		
		const RenderingManager * const rendering_manager 
			= RenderingManager::Get();
		Assert(rendering_manager);
		const U32 width  = rendering_manager->GetWidth();
		const U32 height = rendering_manager->GetHeight();

		// Setup the depth buffer.
		SetupDepthBuffer(device, width, height);
		// Setup the base color buffer;
		SetupBaseColorBuffer(device, width, height);
		// Setup the material buffer.
		SetupMaterialBuffer(device, width, height);
		// Setup the normal buffer.
		SetupNormalBuffer(device, width, height);
	}

	void GBuffer::SetupDepthBuffer(ID3D11Device2 *device, 
		U32 width, U32 height) {

		// Create the texture descriptor.
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width            = width;
		texture_desc.Height           = height;
		texture_desc.MipLevels        = 1u;
		texture_desc.ArraySize        = 1u;
		texture_desc.Format           = DXGI_FORMAT_R24G8_TYPELESS;
		texture_desc.SampleDesc.Count = 1u;
		texture_desc.Usage            = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags        = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		// Create the texture.
		ComPtr< ID3D11Texture2D > texture;
		const HRESULT result_texture = device->CreateTexture2D(
			&texture_desc, nullptr, 
			texture.ReleaseAndGetAddressOf());
		ThrowIfFailed(result_texture, 
			"Texture 2D creation failed: %08X.", result_texture);

		// Create the DSV descriptor.
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
		dsv_desc.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsv_desc.ViewDimension        = D3D11_DSV_DIMENSION_TEXTURE2D;

		// Create the DSV.
		const HRESULT result_dsv = device->CreateDepthStencilView(
			texture.Get(), &dsv_desc,
			m_dsv.ReleaseAndGetAddressOf());
		ThrowIfFailed(result_dsv, 
			"DSV creation failed: %08X.", result_dsv);

		// Create the SRV descriptor.
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format               = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srv_desc.ViewDimension        = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels  = 1u;
		
		// Create the SRV.
		const HRESULT result_srv = device->CreateShaderResourceView(
			texture.Get(), &srv_desc,
			m_srvs[static_cast< size_t >(GBufferIndex::Depth)].ReleaseAndGetAddressOf());
		ThrowIfFailed(result_srv, 
			"SRV creation failed: %08X.", result_srv);
	}
	
	void GBuffer::SetupBaseColorBuffer(ID3D11Device2 *device,
		U32 width, U32 height) {

		// Setup the diffuse buffer;
		SetupBuffer(device, static_cast< size_t >(GBufferIndex::BaseColor),
			width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	void GBuffer::SetupMaterialBuffer(ID3D11Device2 *device,
		U32 width, U32 height) {

		// Setup the specular buffer.
		SetupBuffer(device, static_cast< size_t >(GBufferIndex::Material),
			width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	}
	
	void GBuffer::SetupNormalBuffer(ID3D11Device2 *device,
		U32 width, U32 height) {

		// Setup the normal buffer.
		SetupBuffer(device, static_cast< size_t >(GBufferIndex::Normal),
			width, height, DXGI_FORMAT_R11G11B10_FLOAT);
	}

	void GBuffer::SetupBuffer(ID3D11Device2 *device, U32 index,
		U32 width, U32 height, DXGI_FORMAT format) {

		// Create the texture descriptor.
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width            = width;
		texture_desc.Height           = height;
		texture_desc.MipLevels        = 1u;
		texture_desc.ArraySize        = 1u;
		texture_desc.Format           = format;
		texture_desc.SampleDesc.Count = 1u;
		texture_desc.Usage            = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags        = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// Create the texture.
		ComPtr< ID3D11Texture2D > texture;
		const HRESULT result_texture = device->CreateTexture2D(
			&texture_desc, nullptr, 
			texture.ReleaseAndGetAddressOf());
		ThrowIfFailed(result_texture, 
			"Texture 2D creation failed: %08X.", result_texture);

		// Create the RTV.
		const HRESULT result_rtv = device->CreateRenderTargetView(
			texture.Get(), nullptr,
			m_rtvs[index].ReleaseAndGetAddressOf());
		ThrowIfFailed(result_rtv, 
			"RTV creation failed: %08X.", result_rtv);

		// Create the SRV.
		const HRESULT result_srv = device->CreateShaderResourceView(
			texture.Get(), nullptr,
			m_srvs[index].ReleaseAndGetAddressOf());
		ThrowIfFailed(result_srv, 
			"SRV creation failed: %08X.", result_srv);
	}
}