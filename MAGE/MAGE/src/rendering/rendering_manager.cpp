//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "core\engine.hpp"
#include "logging\error.hpp"
#include "logging\exception.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Definitions
//-----------------------------------------------------------------------------
namespace mage {

	const RenderingManager *RenderingManager::Get() noexcept {
		Assert(Engine::Get());

		return Engine::Get()->GetRenderingManager();
	}

	RenderingManager::RenderingManager(HWND hwindow, 
		const DisplayConfiguration *display_configuration) :
		m_hwindow(hwindow), m_fullscreen(false),
		m_in_begin_end_pair(false), 
		m_display_configuration(
			MakeUnique< DisplayConfiguration >(*display_configuration)),
		m_device(), m_device_context(), m_swap_chain(), 
		m_back_buffer_rtv(), m_back_buffer_srv(),
		m_depth_buffer_dsv(), m_depth_buffer_srv(),
		m_rendering_state_manager() {

		Assert(m_hwindow);
		Assert(m_display_configuration);

		InitializeRenderingManager();
	}

	RenderingManager::RenderingManager(
		RenderingManager &&rendering_manager) = default;

	RenderingManager::~RenderingManager() {
		UninitializeRenderingManager();
	}

	void RenderingManager::InitializeRenderingManager() {
		// Setup the device and device context.
		SetupDevice();
		// Setup the swap chain.
		SetupSwapChain();

		// Setup the rendering state manager.
		m_rendering_state_manager 
			= MakeUnique< RenderingStateManager >(m_device.Get());
		// Bind the persistent samplers of the rendering state manager.
		m_rendering_state_manager->BindPersistentSamplers(m_device_context.Get());
	}

	void RenderingManager::UninitializeRenderingManager() noexcept {
		// Switch to windowed mode since Direct3D is incapable to clear its 
		// state properly when in fullscreen mode due to certain threading 
		// issues that occur behind the scenes.
		if (m_swap_chain) {
			m_swap_chain->SetFullscreenState(FALSE, nullptr);
		}

		// Reset any device context to the default settings. 
		if (m_device_context) {
			m_device_context->ClearState();
		}
	}

	void RenderingManager::SetupDevice() {
		// Set the runtime layers to enable.
		U32 create_device_flags = 0u;
#ifdef _DEBUG
		create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		// Get the ID3D11Device and ID3D11DeviceContext.
		ComPtr< ID3D11Device > device;
		ComPtr< ID3D11DeviceContext > device_context;
		const HRESULT result_device = D3D11CreateDevice(
			GetAdapter(),
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			create_device_flags,
			Pipeline::s_feature_levels,
			_countof(Pipeline::s_feature_levels),
			D3D11_SDK_VERSION,
			device.GetAddressOf(),
			&m_feature_level,
			device_context.GetAddressOf()
		);
		ThrowIfFailed(result_device, 
			"ID3D11Device creation failed: %08X.", result_device);

		// Get the ID3D11Device2.
		const HRESULT result_device2 = device.As(&m_device);
		ThrowIfFailed(result_device2,
			"ID3D11Device2 creation failed: %08X.", result_device2);
		// Get the ID3D11DeviceContext2.
		const HRESULT result_device_context2 = device_context.As(&m_device_context);
		ThrowIfFailed(result_device_context2, 
			"ID3D11DeviceContext2 creation failed: %08X.", result_device_context2);
	}

	void RenderingManager::SetupSwapChain() {
		m_display_configuration->UpdateMSAASampleDesc(GetDevice());

		// Create the swap chain.
		CreateSwapChain();
		// Create the RTV/SRV and DSV/SRV.
		CreateBackBufferRTVandSRV();
		CreateDepthBufferDSVandSRV();
		// Bind the RTV and DSV.
		BindRTVAndDSV();
	}

	void RenderingManager::ResetSwapChain() {
		// Recreate the swap chain buffers.
		m_swap_chain->ResizeBuffers(0u, 0u, 0u, 
			DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		// Create the RTV/SRV and DSV/SRV.
		CreateBackBufferRTVandSRV();
		CreateDepthBufferDSVandSRV();
		// Bind the RTV and DSV.
		BindRTVAndDSV();
	}

	void RenderingManager::CreateSwapChain() {
		// Get the IDXGIFactory3.
		ComPtr< IDXGIFactory3 > dxgi_factory3;
		const HRESULT result_dxgi_factory3 = GetAdapter()->GetParent(
			__uuidof(IDXGIFactory3), (void **)dxgi_factory3.GetAddressOf());
		ThrowIfFailed(result_dxgi_factory3, 
			"IDXGIFactory3 creation failed: %08X.", result_dxgi_factory3);
	
		// DXGI_MWA_NO_WINDOW_CHANGES: Prevent DXGI from monitoring an applications message queue; 
		//                             this makes DXGI unable to respond to mode changes.
		// DXGI_MWA_NO_ALT_ENTER:      Prevent DXGI from responding to an alt-enter sequence.
		// DXGI_MWA_NO_PRINT_SCREEN:   Prevent DXGI from responding to a print-screen key.
		dxgi_factory3->MakeWindowAssociation(m_hwindow, 
			DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_PRINT_SCREEN);

		// Create a DXGI_SWAP_CHAIN_DESC1.
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
		swap_chain_desc.Width       = GetWidth();
		swap_chain_desc.Height      = GetHeight();
		swap_chain_desc.Format      = m_display_configuration->GetDisplayFormat();
		swap_chain_desc.SampleDesc  = m_display_configuration->GetMSAASampleDesc();
		swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		swap_chain_desc.BufferCount = 1u;
		swap_chain_desc.Flags       = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// Create a DXGI_SWAP_CHAIN_FULLSCREEN_DESC.
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC swap_chain_fullscreen_desc = {};
		swap_chain_fullscreen_desc.RefreshRate = m_display_configuration->GetDisplayRefreshRate();
		swap_chain_fullscreen_desc.Windowed    = TRUE;

		// Get the IDXGISwapChain1.
		ComPtr< IDXGISwapChain1 > swap_chain1;
		const HRESULT result_swap_chain1 = dxgi_factory3->CreateSwapChainForHwnd(
			m_device.Get(), m_hwindow, 
			&swap_chain_desc, &swap_chain_fullscreen_desc, nullptr, 
			swap_chain1.ReleaseAndGetAddressOf());
		ThrowIfFailed(result_swap_chain1, 
			"IDXGISwapChain1 creation failed: %08X.", result_swap_chain1);
		// Get the IDXGISwapChain2.
		const HRESULT result_swap_chain2 = swap_chain1.As(&m_swap_chain);
		ThrowIfFailed(result_swap_chain2, 
			"IDXGISwapChain2 creation failed: %08X.", result_swap_chain2);

		// Set to windowed mode.
		m_swap_chain->SetFullscreenState(FALSE, nullptr);
	}

	void RenderingManager::CreateBackBufferRTVandSRV() {
		// Access the only back buffer of the swap-chain.
		ComPtr< ID3D11Texture2D > back_buffer;
		const HRESULT result_back_buffer = m_swap_chain->GetBuffer(
			0u, __uuidof(ID3D11Texture2D), (void **)back_buffer.GetAddressOf());
		ThrowIfFailed(result_back_buffer, 
			"Back buffer texture creation failed: %08X.", result_back_buffer);
		
		// Create the RTV.
		const HRESULT result_rtv = m_device->CreateRenderTargetView(
			back_buffer.Get(), nullptr,
			m_back_buffer_rtv.ReleaseAndGetAddressOf());
		ThrowIfFailed(result_rtv, 
			"RTV creation failed: %08X.", result_rtv);

		// Create the SRV.
		const HRESULT result_srv = m_device->CreateShaderResourceView(
			back_buffer.Get(), nullptr,
			m_back_buffer_srv.ReleaseAndGetAddressOf());
		ThrowIfFailed(result_srv, 
			"SRV creation failed: %08X.", result_srv);
	}

	void RenderingManager::CreateDepthBufferDSVandSRV() {
		// Create the texture descriptor.
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width      = GetWidth();
		texture_desc.Height     = GetHeight();
		texture_desc.MipLevels  = 1u;
		texture_desc.ArraySize  = 1u;
		texture_desc.Format     = DXGI_FORMAT_R24G8_TYPELESS;
		texture_desc.SampleDesc = m_display_configuration->GetMSAASampleDesc();
		texture_desc.Usage      = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags  = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		
		// Create the texture.
		ComPtr< ID3D11Texture2D > texture;
		const HRESULT result_texture = m_device->CreateTexture2D(
			&texture_desc, nullptr,
			texture.ReleaseAndGetAddressOf());
		ThrowIfFailed(result_texture, 
			"Texture 2D creation failed: %08X.", result_texture);

		// Create the DSV descriptor.
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
		dsv_desc.Format        = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsv_desc.ViewDimension = m_display_configuration->UseMSAA() ?
									D3D11_DSV_DIMENSION_TEXTURE2DMS :
									D3D11_DSV_DIMENSION_TEXTURE2D;

		// Create the DSV.
		const HRESULT result_dsv = m_device->CreateDepthStencilView(
			texture.Get(), &dsv_desc,
			m_depth_buffer_dsv.ReleaseAndGetAddressOf());
		ThrowIfFailed(result_dsv, 
			"DSV creation failed: %08X.", result_dsv);

		// Create the SRV descriptor.
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Format        = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		if (m_display_configuration->UseMSAA()) {
			srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		}
		else {
			srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MipLevels = 1u;
		}

		// Create the SRV.
		const HRESULT result_srv = m_device->CreateShaderResourceView(
			texture.Get(), &srv_desc,
			m_depth_buffer_srv.ReleaseAndGetAddressOf());
		ThrowIfFailed(result_srv, 
			"SRV creation failed: %08X.", result_srv);
	}

	void RenderingManager::BindRTVAndDSV() const noexcept {
		Pipeline::OM::BindRTVAndDSV(m_device_context.Get(), 
			m_back_buffer_rtv.Get(), m_depth_buffer_dsv.Get());
	}

	void RenderingManager::BeginFrame() noexcept {
		Assert(!m_in_begin_end_pair);

		// Clear the back buffer.
		Pipeline::OM::ClearRTV(m_device_context.Get(), 
			m_back_buffer_rtv.Get());
		// Clear the depth buffer to 1.0 (i.e. max depth).
		// Clear the stencil buffer to 0.
		Pipeline::OM::ClearDSV(m_device_context.Get(), 
			m_depth_buffer_dsv.Get());

		m_in_begin_end_pair = true;
	}

	void RenderingManager::EndFrame() noexcept {
		Assert(m_in_begin_end_pair);

		// Present the back buffer to the front buffer.
		const U32 sync_interval = (m_display_configuration->IsVSynced()) 
								? 1u : 0u;
		m_swap_chain->Present(sync_interval, 0u);

		m_in_begin_end_pair = false;
	}

	void RenderingManager::SwitchMode(bool toggle) {
		// Release the swap chain buffers.
		m_back_buffer_rtv.Reset();
		m_back_buffer_srv.Reset();
		m_depth_buffer_dsv.Reset();
		m_depth_buffer_srv.Reset();

		BOOL current = false;
		if (toggle) {
			m_swap_chain->GetFullscreenState(&current, nullptr);
			current = !current;
			m_swap_chain->SetFullscreenState(current, nullptr);
		}

		ResetSwapChain();

		m_swap_chain->GetFullscreenState(&current, nullptr);
		m_fullscreen = (current != FALSE);
	}
}