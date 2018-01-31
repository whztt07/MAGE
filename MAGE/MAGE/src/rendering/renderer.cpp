//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "rendering\rendering_manager.hpp"
#include "logging\error.hpp"

// Include HLSL bindings.
#include "hlsl.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Definitions
//-----------------------------------------------------------------------------
namespace mage {

	[[nodiscard]] Renderer *Renderer::Get() noexcept {
		Assert(RenderingManager::Get());
		
		return RenderingManager::Get()->GetRenderer();
	}

	Renderer::Renderer(ID3D11Device5 *device,
		               ID3D11DeviceContext4 *device_context, 
		               U32 width, 
		               U32 height)
		: m_device_context(device_context),
		m_maximum_viewport(width, height),
		m_game_buffer(device),
		m_camera_buffer(device),
		m_aa_pass(),
		m_back_buffer_pass(),
		m_bounding_volume_pass(),
		m_constant_component_pass(),
		m_constant_shading_pass(),
		m_deferred_shading_pass(),
		m_depth_pass(),
		m_dof_pass(),
		m_gbuffer_pass(),
		m_lbuffer_pass(),
		m_shading_normal_pass(),
		m_sky_pass(),
		m_sprite_pass(),
		m_variable_component_pass(),
		m_variable_shading_pass(),
		m_voxelization_pass(),
		m_wireframe_pass() {}
	
	Renderer::Renderer(Renderer &&scene_renderer) noexcept = default;
	
	Renderer::~Renderer() = default;

	void Renderer::BindPersistentState() {
		const auto config = DisplayConfiguration::Get();
		
		GameBuffer buffer;
		buffer.m_display_width                = config->GetDisplayWidth();
		buffer.m_display_height               = config->GetDisplayHeight();
		buffer.m_display_inv_width_minus1     = 1.0f / (buffer.m_display_width  - 1.0f);
		buffer.m_display_inv_height_minus1    = 1.0f / (buffer.m_display_height - 1.0f);
		
		buffer.m_ss_display_width             = config->GetSSDisplayWidth();
		buffer.m_ss_display_height            = config->GetSSDisplayHeight();
		buffer.m_ss_display_inv_width_minus1  = 1.0f / (buffer.m_ss_display_width  - 1.0f);
		buffer.m_ss_display_inv_height_minus1 = 1.0f / (buffer.m_ss_display_height - 1.0f);
		
		buffer.m_gamma                        = config->GetGamma();
		buffer.m_inv_gamma                    = 1.0f / buffer.m_gamma;

		// Update the game buffer.
		m_game_buffer.UpdateData(m_device_context, buffer);
		// Bind the game buffer.
		m_game_buffer.Bind< Pipeline >(m_device_context, SLOT_CBUFFER_GAME);
	}

	void XM_CALLCONV Renderer::BindCameraBuffer(const Camera &camera,
		                                        FXMMATRIX view_to_projection,
		                                        CXMMATRIX projection_to_view,
		                                        CXMMATRIX world_to_view,
		                                        CXMMATRIX view_to_world) {
		CameraBuffer buffer;
		buffer.m_view_to_projection            = XMMatrixTranspose(view_to_projection);
		buffer.m_projection_to_view            = XMMatrixTranspose(projection_to_view);
		buffer.m_world_to_view                 = XMMatrixTranspose(world_to_view);
		buffer.m_view_to_world                 = XMMatrixTranspose(view_to_world);
		
		const auto &viewport                   = camera.GetViewport();
		buffer.m_viewport_top_left_x           = static_cast< U32 >(viewport.GetTopLeftX());
		buffer.m_viewport_top_left_y           = static_cast< U32 >(viewport.GetTopLeftY());
		buffer.m_viewport_width                = static_cast< U32 >(viewport.GetWidth());
		buffer.m_viewport_height               = static_cast< U32 >(viewport.GetHeight());
		buffer.m_viewport_inv_width_minus1     = 1.0f / (viewport.GetWidth()  - 1.0f);
		buffer.m_viewport_inv_height_minus1    = 1.0f / (viewport.GetHeight() - 1.0f);

		const auto ss_viewport                 = camera.GetSSViewport();
		buffer.m_ss_viewport_top_left_x        = static_cast< U32 >(ss_viewport.GetTopLeftX());
		buffer.m_ss_viewport_top_left_y        = static_cast< U32 >(ss_viewport.GetTopLeftY());
		buffer.m_ss_viewport_width             = static_cast< U32 >(ss_viewport.GetWidth());
		buffer.m_ss_viewport_height            = static_cast< U32 >(ss_viewport.GetHeight());
		buffer.m_ss_viewport_inv_width_minus1  = 1.0f / (ss_viewport.GetWidth()  - 1.0f);
		buffer.m_ss_viewport_inv_height_minus1 = 1.0f / (ss_viewport.GetHeight() - 1.0f);

		buffer.m_voxel_size                    = 1.0f;
		buffer.m_voxel_inv_size                = 1.0f / buffer.m_voxel_size;
		buffer.m_voxel_grid_resolution         = 256u;
		buffer.m_voxel_grid_inv_resolution     = 1.0f / buffer.m_voxel_grid_resolution;

		const float radius = buffer.m_voxel_size * buffer.m_voxel_grid_resolution;
		#ifdef DISSABLE_INVERTED_Z_BUFFER
		buffer.m_view_to_voxel = XMMatrixOrthographicOffCenterLH(-radius, radius, 
																 -radius, radius, 
																 -radius, radius);
		#else  // DISSABLE_INVERTED_Z_BUFFER
		buffer.m_view_to_voxel = XMMatrixOrthographicOffCenterLH(-radius, radius,
																 -radius, radius,
																 radius, -radius);
		#endif // DISSABLE_INVERTED_Z_BUFFER

		buffer.m_lens_radius                   = camera.GetLens().GetLensRadius();
		buffer.m_focal_length                  = camera.GetLens().GetFocalLength();
		buffer.m_max_coc_radius                = camera.GetLens().GetMaximumCoCRadius();

		// Update the camera buffer.
		m_camera_buffer.UpdateData(m_device_context, buffer);
		// Bind the game buffer.
		m_camera_buffer.Bind< Pipeline >(m_device_context, SLOT_CBUFFER_PRIMARY_CAMERA);
	}

	void Renderer::Render(const Scene &scene) {
		const auto output_manager = RenderingOutputManager::Get();

		scene.ForEach< Camera >([this, &scene, output_manager](const Camera &camera) {
			if (State::Active != camera.GetState()) {
				return;
			}
			
			output_manager->BindBegin(m_device_context);

			const auto &transform          = camera.GetOwner()->GetTransform();
			const auto world_to_view       = transform.GetWorldToViewMatrix();
			const auto view_to_world       = transform.GetViewToWorldMatrix();
			const auto view_to_projection  = camera.GetViewToProjectionMatrix();
			const auto projection_to_view  = camera.GetProjectionToViewMatrix();
			const auto world_to_projection = world_to_view * view_to_projection;

			// Bind the camera buffer.
			BindCameraBuffer(camera, 
				             view_to_projection, 
				             projection_to_view,
				             world_to_view, 
				             view_to_world);

			// RenderMode
			switch (camera.GetSettings().GetRenderMode()) {

			case RenderMode::Forward: {
				ExecuteForwardPipeline(scene, 
					                   camera, 
					                   world_to_projection, 
					                   world_to_view, 
					                   view_to_world);
				break;
			}

			case RenderMode::Deferred: {
				ExecuteDeferredPipeline(scene, 
					                    camera, 
					                    world_to_projection, 
					                    world_to_view, 
					                    view_to_world);
				break;
			}

			case RenderMode::Solid: {
				ExecuteSolidForwardPipeline(scene, 
					                        camera, 
					                        world_to_projection, 
					                        world_to_view, 
					                        view_to_world);
				break;
			}

			case RenderMode::BaseColor:
			case RenderMode::BaseColorCoefficient:
			case RenderMode::BaseColorTexture:
			case RenderMode::Material:
			case RenderMode::MaterialCoefficient:
			case RenderMode::MaterialTexture:
			case RenderMode::NormalTexture: {
				// Bind the viewport.
				const auto ss_viewport = camera.GetSSViewport();
				ss_viewport.BindViewport(m_device_context);

				output_manager->BindBeginForward(m_device_context);

				const auto pass = GetVariableComponentPass();
				pass->BindFixedState(camera.GetSettings().GetRenderMode());
				pass->Render(scene, 
					         world_to_projection, 
					         world_to_view, 
					         view_to_world);

				break;
			}

			case RenderMode::UVTexture:
			case RenderMode::Distance: {
				// Bind the viewport.
				const auto ss_viewport = camera.GetSSViewport();
				ss_viewport.BindViewport(m_device_context);

				output_manager->BindBeginForward(m_device_context);

				const auto pass = GetConstantComponentPass();
				pass->BindFixedState(camera.GetSettings().GetRenderMode());
				pass->Render(scene,
					         world_to_projection, 
					         world_to_view, 
					         view_to_world);

				break;
			}

			case RenderMode::ShadingNormal:
			case RenderMode::TSNMShadingNormal: {
				// Bind the viewport.
				const auto ss_viewport = camera.GetSSViewport();
				ss_viewport.BindViewport(m_device_context);

				output_manager->BindBeginForward(m_device_context);

				const auto pass = GetShadingNormalPass();
				pass->BindFixedState(camera.GetSettings().GetRenderMode());
				pass->Render(scene,
					         world_to_projection, 
					         world_to_view);

				break;
			}

			case RenderMode::None: {
				// Bind the viewport.
				const auto ss_viewport = camera.GetSSViewport();
				ss_viewport.BindViewport(m_device_context);

				output_manager->BindBeginForward(m_device_context);
				break;
			}

			}

			// RenderLayer
			if (camera.GetSettings().ContainsRenderLayer(RenderLayer::Wireframe)) {
				const auto pass = GetWireframePass();
				pass->BindFixedState();
				pass->Render(scene, world_to_projection, world_to_view);
			}
			if (camera.GetSettings().ContainsRenderLayer(RenderLayer::AABB)) {
				const auto pass = GetBoundingVolumePass();
				pass->BindFixedState();
				pass->Render(scene, world_to_projection, world_to_view);
			}
			
			output_manager->BindEndForward(m_device_context);

			ExecuteAAPipeline(camera);

			output_manager->BindBeginPostProcessing(m_device_context);

			// Bind the viewport.
			const auto &viewport = camera.GetViewport();
			viewport.BindViewport(m_device_context);
			
			if (camera.GetLens().HasFiniteAperture()) {
				output_manager->BindPingPong(m_device_context);
				GetDOFPass()->Dispatch(viewport);
			}

			output_manager->BindEnd(m_device_context);

			// Perform a back buffer pass.
			const auto back_buffer_pass = GetBackBufferPass();
			back_buffer_pass->BindFixedState();
			back_buffer_pass->Render();
		});
		
		// Bind the maximum viewport.
		m_maximum_viewport.BindViewport(m_device_context);
		
		// Perform a sprite pass.
		const auto * const sprite_pass = GetSpritePass();
		sprite_pass->BindFixedState();
		sprite_pass->Render(scene);
	}

	void XM_CALLCONV Renderer
		::ExecuteSolidForwardPipeline(const Scene &scene,
		                              const Camera &camera,
		                              FXMMATRIX world_to_projection,
		                              CXMMATRIX world_to_view,
		                              CXMMATRIX view_to_world) {
		
		const auto output_manager = RenderingOutputManager::Get();
		const auto viewport = camera.GetSSViewport();

		// Perform a LBuffer pass.
		const auto lbuffer_pass = GetLBufferPass();
		lbuffer_pass->Render(scene, 
			                 camera.GetSettings().GetFog(),
			                 world_to_projection, 
			                 world_to_view, 
			                 view_to_world);
		// Restore the viewport.
		viewport.BindViewport(m_device_context);
		
		output_manager->BindBeginForward(m_device_context);
		
		// Perform a forward pass.
		const auto forward_pass = GetConstantShadingPass();
		forward_pass->BindFixedState();
		forward_pass->Render(scene, 
			                 world_to_projection, 
			                 world_to_view, 
			                 view_to_world);
	}

	void XM_CALLCONV Renderer
		::ExecuteForwardPipeline(const Scene &scene,
		                         const Camera &camera,
		                         FXMMATRIX world_to_projection,
		                         CXMMATRIX world_to_view,
		                         CXMMATRIX view_to_world) {

		const auto output_manager = RenderingOutputManager::Get();
		const auto viewport = camera.GetSSViewport();
		
		// Perform a LBuffer pass.
		const auto lbuffer_pass = GetLBufferPass();
		lbuffer_pass->Render(scene, 
			                 camera.GetSettings().GetFog(),
			                 world_to_projection, 
			                 world_to_view, 
			                 view_to_world);
		// Restore the viewport.
		viewport.BindViewport(m_device_context);
		
		output_manager->BindBeginForward(m_device_context);

		// Perform a forward pass: opaque fragments.
		const auto forward_pass = GetVariableShadingPass();
		forward_pass->BindFixedState(camera.GetSettings().GetBRDF());
		forward_pass->Render(scene, 
			                 world_to_projection, 
			                 world_to_view, 
			                 view_to_world);

		// Perform a sky pass.
		const auto * const sky_pass = GetSkyPass();
		sky_pass->BindFixedState();
		sky_pass->Render(camera.GetSettings().GetSky());

		// Perform a forward pass: transparent fragments.
		forward_pass->BindFixedState(camera.GetSettings().GetBRDF());
		forward_pass->RenderTransparent(scene, 
			                            world_to_projection, 
			                            world_to_view, 
			                            view_to_world);
	}

	void XM_CALLCONV Renderer
		::ExecuteDeferredPipeline(const Scene &scene,
		                          const Camera &camera,
		                          FXMMATRIX world_to_projection,
		                          CXMMATRIX world_to_view,
		                          CXMMATRIX view_to_world) {

		const auto output_manager = RenderingOutputManager::Get();
		const auto viewport = camera.GetSSViewport();

		// Perform a LBuffer pass.
		const auto lbuffer_pass = GetLBufferPass();
		lbuffer_pass->Render(scene, 
			                 camera.GetSettings().GetFog(),
			                 world_to_projection, 
			                 world_to_view, 
			                 view_to_world);
		// Restore the viewport.
		viewport.BindViewport(m_device_context);

		output_manager->BindBeginGBuffer(m_device_context);

		// Perform a GBuffer pass: opaque fragments.
		const auto gbuffer_pass = GetGBufferPass();
		gbuffer_pass->BindFixedState();
		gbuffer_pass->Render(scene, 
			                 world_to_projection, 
			                 world_to_view, 
			                 view_to_world);

		output_manager->BindEndGBuffer(m_device_context);
		output_manager->BindBeginDeferred(m_device_context);

		// Perform a deferred pass.
		const auto deferred_pass = GetDeferredShadingPass();
		if (DisplayConfiguration::Get()->UsesMSAA()) {
			deferred_pass->BindFixedState(camera.GetSettings().GetBRDF(), false);
			deferred_pass->Render();
		}
		else {
			deferred_pass->BindFixedState(camera.GetSettings().GetBRDF(), true);
			deferred_pass->Dispatch(viewport);
		}

		output_manager->BindEndDeferred(m_device_context);
		output_manager->BindBeginForward(m_device_context);

		// Perform a forward pass: emissive fragments.
		const auto forward_pass = GetVariableShadingPass();
		forward_pass->BindFixedState(camera.GetSettings().GetBRDF());
		forward_pass->RenderEmissive(scene, 
			                         world_to_projection, 
			                         world_to_view, 
			                         view_to_world);

		// Perform a sky pass.
		const auto * const sky_pass = GetSkyPass();
		sky_pass->BindFixedState();
		sky_pass->Render(camera.GetSettings().GetSky());

		// Perform a forward pass: transparent fragments.
		forward_pass->BindFixedState(camera.GetSettings().GetBRDF());
		forward_pass->RenderTransparent(scene, 
			                            world_to_projection, 
			                            world_to_view, 
			                            view_to_world);
	}

	void Renderer::ExecuteAAPipeline(const Camera &camera) {
		const auto output_manager = RenderingOutputManager::Get();
		const auto desc = DisplayConfiguration::Get()->GetAADescriptor();
		const auto viewport = camera.GetSSViewport();

		switch (desc) {

		case AADescriptor::FXAA: {

			output_manager->BindBeginResolve(m_device_context);

			// Perform an AA pass.
			const auto aa_pass = GetAAPass();
			aa_pass->DispatchAAPreprocess(viewport, AADescriptor::FXAA);

			output_manager->BindEndResolve(m_device_context);
			output_manager->BindPingPong(m_device_context);

			// Perform a FXAA pass.
			aa_pass->DispatchAA(viewport, AADescriptor::FXAA);

			break;
		}

		case AADescriptor::MSAA_2x:
		case AADescriptor::MSAA_4x:
		case AADescriptor::MSAA_8x:
		case AADescriptor::SSAA_2x:
		case AADescriptor::SSAA_3x:
		case AADescriptor::SSAA_4x: {
			
			output_manager->BindBeginResolve(m_device_context);

			// Perform an AA pass.
			const auto aa_pass = GetAAPass();
			aa_pass->DispatchAA(viewport, desc);

			output_manager->BindEndResolve(m_device_context);
			break;
		}

		}
	}
}