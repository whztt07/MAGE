//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "renderer\output_manager.hpp"
#include "renderer\pipeline.hpp"
#include "exception\exception.hpp"

// Include HLSL bindings.
#include "hlsl.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// System Includes
//-----------------------------------------------------------------------------
#pragma region

#include <iterator>

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Definitions
//-----------------------------------------------------------------------------
namespace mage::rendering {

	OutputManager::OutputManager(ID3D11Device& device, 
								 DisplayConfiguration& display_configuration, 
								 SwapChain& swap_chain)
		: m_display_configuration(display_configuration),
		m_device(device), 
		m_swap_chain(swap_chain),
		m_srvs{}, 
		m_rtvs{}, 
		m_uavs{}, 
		m_dsv(), 
		m_hdr0_to_hdr1(true), 
		m_msaa(false), 
		m_ssaa(false) {

		SetupBuffers();
	}

	OutputManager::OutputManager(OutputManager&& manager) noexcept = default;

	OutputManager::~OutputManager() = default;

	OutputManager& OutputManager
		::operator=(OutputManager&& manager) noexcept = default;

	void OutputManager::SetupBuffers() {
		U32 nb_samples = 1u;
		auto width     = m_display_configuration.get().GetDisplayWidth();
		auto height    = m_display_configuration.get().GetDisplayHeight();
		auto ss_width  = width;
		auto ss_height = height;
		auto aa        = true;
		
		switch (m_display_configuration.get().GetAADescriptor()) {

		case AADescriptor::MSAA_2x: {
			m_msaa     = true;
			nb_samples = 2u;
			break;
		}
		case AADescriptor::MSAA_4x: {
			m_msaa     = true;
			nb_samples = 4u;
			break;
		}
		case AADescriptor::MSAA_8x: {
			m_msaa     = true;
			nb_samples = 8u;
			break;
		}
		
		case AADescriptor::SSAA_2x: {
			m_ssaa     = true;
			ss_width  *= 2u;
			ss_height *= 2u;
			break;
		}
		case AADescriptor::SSAA_3x: {
			m_ssaa     = true;
			ss_width  *= 3u;
			ss_height *= 3u;
			break;
		}
		case AADescriptor::SSAA_4x: {
			m_ssaa     = true;
			ss_width  *= 4u;
			ss_height *= 4u;
			break;
		}

		case AADescriptor::None: {
			aa = false;
			break;
		}

		}

		// Setup the depth buffer.
		SetupDepthBuffer(ss_width, ss_height, nb_samples);

		// Setup the GBuffer buffers.
		SetupBuffer(ss_width, ss_height, nb_samples, 
			        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
			        ReleaseAndGetAddressOfSRV(SRVIndex::GBuffer_BaseColor),
			        ReleaseAndGetAddressOfRTV(RTVIndex::GBuffer_BaseColor),
			        nullptr);
		SetupBuffer(ss_width, ss_height, nb_samples, 
			        DXGI_FORMAT_R8G8B8A8_UNORM,
			        ReleaseAndGetAddressOfSRV(SRVIndex::GBuffer_Material),
			        ReleaseAndGetAddressOfRTV(RTVIndex::GBuffer_Material),
			        nullptr);
		SetupBuffer(ss_width, ss_height, nb_samples, 
					DXGI_FORMAT_R11G11B10_FLOAT,
			        ReleaseAndGetAddressOfSRV(SRVIndex::GBuffer_Normal),
			        ReleaseAndGetAddressOfRTV(RTVIndex::GBuffer_Normal),
			        nullptr);

		// Setup the HDR buffer.
		if (m_msaa) {
			SetupBuffer(ss_width, ss_height, nb_samples, 
				        DXGI_FORMAT_R16G16B16A16_FLOAT,
				        ReleaseAndGetAddressOfSRV(SRVIndex::HDR),
				        ReleaseAndGetAddressOfRTV(RTVIndex::HDR),
				        nullptr);
		}
		else {
			SetupBuffer(ss_width, ss_height, 1u, 
				        DXGI_FORMAT_R16G16B16A16_FLOAT,
				        ReleaseAndGetAddressOfSRV(SRVIndex::HDR),
				        ReleaseAndGetAddressOfRTV(RTVIndex::HDR),
				        ReleaseAndGetAddressOfUAV(UAVIndex::HDR));
		}
		
		if (aa) {
			SetupBuffer(width, height, 1u, 
				        DXGI_FORMAT_R16G16B16A16_FLOAT,
				        ReleaseAndGetAddressOfSRV(SRVIndex::PostProcessing_HDR0),
				        ReleaseAndGetAddressOfRTV(RTVIndex::PostProcessing_HDR0),
				        ReleaseAndGetAddressOfUAV(UAVIndex::PostProcessing_HDR0));
		}
		else {
			m_srvs[static_cast< size_t >(SRVIndex::PostProcessing_HDR0)]
				= m_srvs[static_cast< size_t >(SRVIndex::HDR)];
			m_rtvs[static_cast< size_t >(RTVIndex::PostProcessing_HDR0)]
				= m_rtvs[static_cast< size_t >(RTVIndex::HDR)];
			m_uavs[static_cast< size_t >(UAVIndex::PostProcessing_HDR0)]
				= m_uavs[static_cast< size_t >(UAVIndex::HDR)];
		}

		SetupBuffer(width, height, 1u, 
			        DXGI_FORMAT_R16G16B16A16_FLOAT,
			        ReleaseAndGetAddressOfSRV(SRVIndex::PostProcessing_HDR1),
			        ReleaseAndGetAddressOfRTV(RTVIndex::PostProcessing_HDR1),
			        ReleaseAndGetAddressOfUAV(UAVIndex::PostProcessing_HDR1));

		if (m_msaa || m_ssaa) {
			SetupBuffer(width, height, 1u,
				        DXGI_FORMAT_R32_FLOAT,
				        ReleaseAndGetAddressOfSRV(SRVIndex::PostProcessing_Depth),
				        nullptr,
				        ReleaseAndGetAddressOfUAV(UAVIndex::PostProcessing_Depth));
			
			SetupBuffer(width, height, 1u, 
				        DXGI_FORMAT_R11G11B10_FLOAT,
				        ReleaseAndGetAddressOfSRV(SRVIndex::PostProcessing_Normal),
				        nullptr,
				        ReleaseAndGetAddressOfUAV(UAVIndex::PostProcessing_Normal));
		}
		else {
			m_srvs[static_cast< size_t >(SRVIndex::PostProcessing_Depth)]
				= m_srvs[static_cast< size_t >(SRVIndex::GBuffer_Depth)];
			m_srvs[static_cast< size_t >(SRVIndex::PostProcessing_Normal)]
				= m_srvs[static_cast< size_t >(SRVIndex::GBuffer_Normal)];
		}
	}

	void OutputManager::SetupBuffer(U32 width, 
		                            U32 height, 
		                            U32 nb_samples, 
		                            DXGI_FORMAT format, 
		                            ID3D11ShaderResourceView** srv, 
		                            ID3D11RenderTargetView** rtv, 
		                            ID3D11UnorderedAccessView** uav) {

		// Create the texture descriptor.
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width            = width;
		texture_desc.Height           = height;
		texture_desc.MipLevels        = 1u;
		texture_desc.ArraySize        = 1u;
		texture_desc.Format           = format;
		texture_desc.SampleDesc.Count = nb_samples;
		texture_desc.Usage            = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
		if (rtv) {
			texture_desc.BindFlags   |= D3D11_BIND_RENDER_TARGET;
		}
		if (uav) {
			texture_desc.BindFlags   |= D3D11_BIND_UNORDERED_ACCESS;
		}

		// Sample quality
		if (1u != nb_samples) {
			const HRESULT result = m_device.get().CheckMultisampleQualityLevels(
				texture_desc.Format, texture_desc.SampleDesc.Count,
				&texture_desc.SampleDesc.Quality);
			ThrowIfFailed(result,
				"Multi-sampled texture 2D creation failed: %08X.", result);
			ThrowIfFailed((0u != texture_desc.SampleDesc.Quality),
				"Multi-sampled texture 2D creation failed.");
			--texture_desc.SampleDesc.Quality;
		}

		ComPtr< ID3D11Texture2D > texture;
		// Texture
		{
			// Create the texture.
			const HRESULT result = m_device.get().CreateTexture2D(
				&texture_desc, nullptr, texture.ReleaseAndGetAddressOf());
			ThrowIfFailed(result, "Texture 2D creation failed: %08X.", result);
		}

		// SRV
		{
			// Create the SRV.
			const HRESULT result = m_device.get().CreateShaderResourceView(
				texture.Get(), nullptr, srv);
			ThrowIfFailed(result, "SRV creation failed: %08X.", result);
		}

		// RTV
		if (rtv) {
			// Create the RTV.
			const HRESULT result = m_device.get().CreateRenderTargetView(
				texture.Get(), nullptr, rtv);
			ThrowIfFailed(result, "RTV creation failed: %08X.", result);
		}

		// UAV
		if (uav) {
			// Create the UAV.
			const HRESULT result = m_device.get().CreateUnorderedAccessView(
				texture.Get(), nullptr, uav);
			ThrowIfFailed(result, "UAV creation failed: %08X.", result);
		}
	}

	void OutputManager::SetupDepthBuffer(U32 width, 
		                                 U32 height, 
		                                 U32 nb_samples) {
		// Create the texture descriptor.
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width            = width;
		texture_desc.Height           = height;
		texture_desc.MipLevels        = 1u;
		texture_desc.ArraySize        = 1u;
		texture_desc.Format           = DXGI_FORMAT_R32_TYPELESS;
		texture_desc.SampleDesc.Count = nb_samples;
		texture_desc.Usage            = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE 
			                          | D3D11_BIND_DEPTH_STENCIL;

		// Sample quality
		if (1u != nb_samples) {
			const HRESULT result = m_device.get().CheckMultisampleQualityLevels(
				texture_desc.Format, texture_desc.SampleDesc.Count,
				&texture_desc.SampleDesc.Quality);
			ThrowIfFailed(result, 
				"Multi-sampled texture 2D creation failed: %08X.", result);
			ThrowIfFailed((0u != texture_desc.SampleDesc.Quality), 
				"Multi-sampled texture 2D creation failed.");
			--texture_desc.SampleDesc.Quality;
		}

		ComPtr< ID3D11Texture2D > texture;
		// Texture
		{
			// Create the texture.
			const HRESULT result = m_device.get().CreateTexture2D(
				&texture_desc, nullptr, 
				texture.ReleaseAndGetAddressOf());
			ThrowIfFailed(result, "Texture 2D creation failed: %08X.", result);
		}

		// SRV
		{
			// Create the SRV descriptor.
			D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
			srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
			if (1u != nb_samples) {
				srv_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2DMS;
			}
			else {
				srv_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
				srv_desc.Texture2D.MipLevels = 1u;
			}
			
			// Create the SRV.
			const HRESULT result = m_device.get().CreateShaderResourceView(
				texture.Get(), &srv_desc,
				ReleaseAndGetAddressOfSRV(SRVIndex::GBuffer_Depth));
			ThrowIfFailed(result, "SRV creation failed: %08X.", result);
		}

		// DSV
		{
			// Create the DSV descriptor.
			D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
			dsv_desc.Format = DXGI_FORMAT_D32_FLOAT;
			dsv_desc.ViewDimension = (1u != nb_samples) ?
				                     D3D11_DSV_DIMENSION_TEXTURE2DMS :
				                     D3D11_DSV_DIMENSION_TEXTURE2D;

			// Create the DSV.
			const HRESULT result = m_device.get().CreateDepthStencilView(
				texture.Get(), &dsv_desc,
				m_dsv.ReleaseAndGetAddressOf());
			ThrowIfFailed(result, "DSV creation failed: %08X.", result);
		}
	}

	void OutputManager::BindBegin(
		ID3D11DeviceContext& device_context) const noexcept {

		static_assert(SLOT_SRV_MATERIAL == SLOT_SRV_BASE_COLOR + 1);
		static_assert(SLOT_SRV_NORMAL   == SLOT_SRV_BASE_COLOR + 2);
		static_assert(SLOT_SRV_DEPTH    == SLOT_SRV_BASE_COLOR + 3);

		// Collect the GBuffer SRVs.
		ID3D11ShaderResourceView* const srvs[4] = {};
		// Bind no GBuffer SRVs.
		Pipeline::PS::BindSRVs(device_context, SLOT_SRV_BASE_COLOR, 
							   static_cast< U32 >(std::size(srvs)), srvs);
		Pipeline::CS::BindSRVs(device_context, SLOT_SRV_BASE_COLOR, 
							   static_cast< U32 >(std::size(srvs)), srvs);

		// Clear the GBuffer RTVs.
		Pipeline::OM::ClearRTV(device_context, 
							   GetRTV(RTVIndex::GBuffer_BaseColor));
		Pipeline::OM::ClearRTV(device_context, 
							   GetRTV(RTVIndex::GBuffer_Material));
		Pipeline::OM::ClearRTV(device_context, 
							   GetRTV(RTVIndex::GBuffer_Normal));
		// Clear the GBuffer DSV.
		Pipeline::OM::ClearDepthOfDSV(device_context, m_dsv.Get());

		// Bind no HDR SRV.
		Pipeline::PS::BindSRV(device_context, SLOT_SRV_IMAGE, nullptr);
		Pipeline::CS::BindSRV(device_context, SLOT_SRV_IMAGE, nullptr);
		
		// Clear the HDR RTV.
		Pipeline::OM::ClearRTV(device_context, GetRTV(RTVIndex::HDR));

		m_hdr0_to_hdr1 = true;
	}

	void OutputManager::BindBeginGBuffer(
		ID3D11DeviceContext& device_context) const noexcept {
		
		// Collect the GBuffer RTVs.
		ID3D11RenderTargetView* const rtvs[] = {
			GetRTV(RTVIndex::GBuffer_BaseColor),
			GetRTV(RTVIndex::GBuffer_Material),
			GetRTV(RTVIndex::GBuffer_Normal)
		};

		// Bind the GBuffer RTVs and GBuffer DSV.
		Pipeline::OM::BindRTVsAndDSV(device_context, 
									 static_cast< U32 >(std::size(rtvs)), 
									 rtvs, m_dsv.Get());
	}
	
	void OutputManager::BindEndGBuffer(
		ID3D11DeviceContext& device_context) const noexcept {

		// Bind no RTV and no DSV.
		Pipeline::OM::BindRTVAndDSV(device_context, nullptr, nullptr);
	}
	
	void OutputManager::BindBeginDeferred(
		ID3D11DeviceContext& device_context) const noexcept {

		static_assert(SLOT_SRV_MATERIAL == SLOT_SRV_BASE_COLOR + 1);
		static_assert(SLOT_SRV_NORMAL   == SLOT_SRV_BASE_COLOR + 2);
		static_assert(SLOT_SRV_DEPTH    == SLOT_SRV_BASE_COLOR + 3);

		// Collect the GBuffer SRVs. 
		ID3D11ShaderResourceView* const srvs[] = {
			GetSRV(SRVIndex::GBuffer_BaseColor),
			GetSRV(SRVIndex::GBuffer_Material),
			GetSRV(SRVIndex::GBuffer_Normal),
			GetSRV(SRVIndex::GBuffer_Depth)
		};
		
		if (m_msaa) {
			// Bind the GBuffer SRVs.
			Pipeline::PS::BindSRVs(device_context, SLOT_SRV_BASE_COLOR, 
								   static_cast< U32 >(std::size(srvs)), srvs);

			// Bind the HDR RTV and no DSV.
			Pipeline::OM::BindRTVAndDSV(device_context, 
										GetRTV(RTVIndex::HDR), nullptr);
		}
		else {
			// Bind the GBuffer SRVs.
			Pipeline::CS::BindSRVs(device_context, SLOT_SRV_BASE_COLOR, 
								   static_cast< U32 >(std::size(srvs)), srvs);

			// Bind the HDR UAV.
			Pipeline::CS::BindUAV(device_context, SLOT_UAV_IMAGE, 
								  GetUAV(UAVIndex::HDR));
		}
	}
	
	void OutputManager::BindEndDeferred(
		ID3D11DeviceContext& device_context) const noexcept {

		static_assert(SLOT_SRV_MATERIAL == SLOT_SRV_BASE_COLOR + 1);
		static_assert(SLOT_SRV_NORMAL   == SLOT_SRV_BASE_COLOR + 2);
		static_assert(SLOT_SRV_DEPTH    == SLOT_SRV_BASE_COLOR + 3);

		// Collect the GBuffer SRVs.
		ID3D11ShaderResourceView* const srvs[4] = {};

		if (m_msaa) {
			// Bind no GBuffer SRVs.
			Pipeline::PS::BindSRVs(device_context, SLOT_SRV_BASE_COLOR, 
								   static_cast< U32 >(std::size(srvs)), srvs);

			// Bind no RTV and no DSV.
			Pipeline::OM::BindRTVAndDSV(device_context, nullptr, nullptr);
		}
		else {
			// Bind no GBuffer SRVs.
			Pipeline::CS::BindSRVs(device_context, SLOT_SRV_BASE_COLOR, 
								   static_cast< U32 >(std::size(srvs)), srvs);

			// Bind no HDR UAV.
			Pipeline::CS::BindUAV(device_context, SLOT_UAV_IMAGE, nullptr);
		}
	}
	
	void OutputManager::BindBeginForward(
		ID3D11DeviceContext& device_context) const noexcept {

		// Collect the RTVs.
		ID3D11RenderTargetView* const rtvs[] = {
			GetRTV(RTVIndex::HDR),
			GetRTV(RTVIndex::GBuffer_Normal)
		};

		// Bind the RTVs and DSV.
		Pipeline::OM::BindRTVsAndDSV(device_context, 
									 static_cast< U32 >(std::size(rtvs)), 
									 rtvs, m_dsv.Get());
	}

	void OutputManager::BindEndForward(
		ID3D11DeviceContext& device_context) const noexcept {
		
		// Bind no RTV and no DSV.
		Pipeline::OM::BindRTVAndDSV(device_context, nullptr, nullptr);
	}

	void OutputManager::BindBeginResolve(
		ID3D11DeviceContext& device_context) const noexcept {

		// Bind the SRVs.
		Pipeline::CS::BindSRV(device_context, SLOT_SRV_IMAGE,  
							  GetSRV(SRVIndex::HDR));
		Pipeline::CS::BindSRV(device_context, SLOT_SRV_NORMAL, 
							  GetSRV(SRVIndex::GBuffer_Normal));
		Pipeline::CS::BindSRV(device_context, SLOT_SRV_DEPTH, 
							  GetSRV(SRVIndex::GBuffer_Depth));

		static_assert(SLOT_UAV_NORMAL == SLOT_UAV_IMAGE + 1);
		static_assert(SLOT_UAV_DEPTH  == SLOT_UAV_IMAGE + 2);

		// Collect the SRVs.
		ID3D11UnorderedAccessView* const uavs[] = {
			GetUAV(UAVIndex::PostProcessing_HDR0),
			GetUAV(UAVIndex::PostProcessing_Normal),
			GetUAV(UAVIndex::PostProcessing_Depth)
		};

		// Bind the UAVs.
		Pipeline::CS::BindUAVs(device_context, SLOT_UAV_IMAGE, 
							   static_cast< U32 >(std::size(uavs)), uavs);
	}

	void OutputManager::BindEndResolve(
		ID3D11DeviceContext& device_context) const noexcept {

		static_assert(SLOT_UAV_NORMAL == SLOT_UAV_IMAGE + 1);
		static_assert(SLOT_UAV_DEPTH  == SLOT_UAV_IMAGE + 2);

		// Bind no SRVs.
		Pipeline::CS::BindSRV(device_context, SLOT_SRV_IMAGE,  nullptr);
		Pipeline::CS::BindSRV(device_context, SLOT_SRV_NORMAL, nullptr);
		Pipeline::CS::BindSRV(device_context, SLOT_SRV_DEPTH,  nullptr);

		// Collect the SRVs.
		ID3D11UnorderedAccessView* const uavs[3] = {};

		// Bind no UAVs.
		Pipeline::CS::BindUAVs(device_context, SLOT_UAV_IMAGE, 
							   static_cast< U32 >(std::size(uavs)), uavs);
	}

	void OutputManager::BindBeginPostProcessing(
		ID3D11DeviceContext& device_context) const noexcept {

		Pipeline::CS::BindSRV(device_context, SLOT_SRV_NORMAL, 
							  GetSRV(SRVIndex::PostProcessing_Normal));
		Pipeline::CS::BindSRV(device_context, SLOT_SRV_DEPTH, 
							  GetSRV(SRVIndex::PostProcessing_Depth));
	}

	void OutputManager::BindPingPong(
		ID3D11DeviceContext& device_context) const noexcept {

		// Bind no HDR UAV.
		Pipeline::CS::BindUAV(device_context, SLOT_UAV_IMAGE, nullptr);
		
		if (m_hdr0_to_hdr1) {
			// Bind HDR UAV.
			Pipeline::CS::BindUAV(device_context, SLOT_UAV_IMAGE, 
								  GetUAV(UAVIndex::PostProcessing_HDR1));
			// Bind HDR SRV.
			Pipeline::CS::BindSRV(device_context, SLOT_SRV_IMAGE, 
								  GetSRV(SRVIndex::PostProcessing_HDR0));
		}
		else {
			// Bind HDR UAV.
			Pipeline::CS::BindUAV(device_context, SLOT_UAV_IMAGE, 
								  GetUAV(UAVIndex::PostProcessing_HDR0));
			// Bind HDR SRV.
			Pipeline::CS::BindSRV(device_context, SLOT_SRV_IMAGE, 
								  GetSRV(SRVIndex::PostProcessing_HDR1));
		}

		m_hdr0_to_hdr1 = !m_hdr0_to_hdr1;
	}

	void OutputManager::BindEnd(
		ID3D11DeviceContext& device_context) const noexcept {

		// Bind the back buffer RTV and no DSV.
		Pipeline::OM::BindRTVAndDSV(device_context, 
									m_swap_chain.get().GetRTV(),
									nullptr);
		
		// Bind no HDR UAV.
		Pipeline::CS::BindUAV(device_context, SLOT_UAV_IMAGE, nullptr);

		if (m_hdr0_to_hdr1) {
			// Bind HDR SRV.
			Pipeline::PS::BindSRV(device_context, SLOT_SRV_IMAGE, 
								  GetSRV(SRVIndex::PostProcessing_HDR0));
		}
		else {
			// Bind HDR SRV.
			Pipeline::PS::BindSRV(device_context, SLOT_SRV_IMAGE, 
								  GetSRV(SRVIndex::PostProcessing_HDR1));
		}
	}
}