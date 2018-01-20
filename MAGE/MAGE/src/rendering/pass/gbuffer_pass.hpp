#pragma once

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "scene\scene.hpp"
#include "rendering\buffer\constant_buffer.hpp"
#include "rendering\buffer\model_buffer.hpp"
#include "shader\shader.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Declarations end Definitions
//-----------------------------------------------------------------------------
namespace mage {

	/**
	 A class of GBuffer passes for packing GBuffers.
	 */
	class GBufferPass final {

	public:

		//---------------------------------------------------------------------
		// Class Member Methods
		//---------------------------------------------------------------------

		/**
		 Returns the GBuffer pass associated with the current engine.

		 @pre			The scene renderer associated with the current engine 
						must be loaded.
		 @return		A pointer to the GBuffer pass associated with the 
						current engine.
		 */
		static GBufferPass *Get();

		//---------------------------------------------------------------------
		// Constructors and Destructors
		//---------------------------------------------------------------------

		/**
		 Constructs a GBuffer pass.

		 @pre			The renderer associated with the current engine must be 
						loaded.
		 @pre			The resource manager associated with the current engine 
						must be loaded.
		 */
		GBufferPass();

		/**
		 Constructs a GBuffer pass from the given GBuffer pass.

		 @param[in]		render_pass
						A reference to the GBuffer pass to copy.
		 */
		GBufferPass(const GBufferPass &render_pass) = delete;

		/**
		 Constructs a GBuffer pass by moving the given GBuffer pass.

		 @param[in]		render_pass
						A reference to the GBuffer pass to move.
		 */
		GBufferPass(GBufferPass &&render_pass) noexcept;

		/**
		 Destructs this GBuffer pass.
		 */
		~GBufferPass();

		//---------------------------------------------------------------------
		// Assignment Operators
		//---------------------------------------------------------------------

		/**
		 Copies the given GBuffer pass to this GBuffer pass.

		 @param[in]		render_pass
						A reference to the GBuffer pass to copy.
		 @return		A reference to the copy of the given GBuffer pass (i.e. 
						this GBuffer pass).
		 */
		GBufferPass &operator=(const GBufferPass &render_pass) = delete;

		/**
		 Moves the given GBuffer pass to this GBuffer pass.

		 @param[in]		render_pass
						A reference to the GBuffer pass to move.
		 @return		A reference to the moved GBuffer pass (i.e. this 
						GBuffer pass).
		 */
		GBufferPass &operator=(GBufferPass &&render_pass) = delete;

		//---------------------------------------------------------------------
		// Member Methods
		//---------------------------------------------------------------------

		/**
		 Binds the fixed state of this GBuffer pass.

		 @throws		Exception
						Failed to bind the fixed state of this GBuffer pass.
		 */
		void BindFixedState();

		/**
		 Renders the scene.

		 @param[in]		scene
						A reference to the scene.
		 @param[in]		world_to_projection
						The world-to-projection transformation matrix.
		 @param[in]		world_to_view
						The world-to-view transformation matrix.
		 @param[in]		view_to_world
						The view-to-world transformation matrix.
		 @throws		Exception
						Failed to render the scene.
		 */
		void XM_CALLCONV Render(const Scene &scene,
			                    FXMMATRIX world_to_projection,
			                    CXMMATRIX world_to_view,
			                    CXMMATRIX view_to_world);
		
	private:

		//---------------------------------------------------------------------
		// Member Methods
		//---------------------------------------------------------------------

		/**
		 An enumeration of the different pixel shader indices for GBuffer 
		 passes.

		 This contains:
		 @c Default and
		 @c TSNM.
		 */
		enum class PSIndex : U8 {
			Default = 0,
			TSNM    = 1,
			Count   = 2
		};

		/**
		 Binds the pixel shader of this GBuffer pass associated with the given 
		 pixel shader index.

		 @param[in]		index
						The pixel shader index.
		 */
		void BindPS(PSIndex index) noexcept;
		
		/**
		 Binds the pixel shader of this GBuffer pass associated with the given 
		 material.

		 @param[in]		material
						A reference to the material.
		 */
		void BindPS(const Material &material) noexcept;
		
		/**
		 Binds the model data of this GBuffer pass.

		 @param[in]		object_to_view
						The object-to-view transformation matrix used for 
						transforming vertices.
		 @param[in]		view_to_object
						The view-to-object transformation matrix used for
						transforming normals.
		 @param[in]		texture_transform
						The texture transformation matrix used for
						transforming texture coordinates.
		 @param[in]		material
						A reference to the material.
		 @throws		Exception
						Failed to bind the model data 
						of this GBuffer pass.
		 */
		void XM_CALLCONV BindModelData(FXMMATRIX object_to_view, 
			                           CXMMATRIX view_to_object,
			                           CXMMATRIX texture_transform,
			                           const Material &material);
		
		//---------------------------------------------------------------------
		// Member Variables
		//---------------------------------------------------------------------

		/**
		 A pointer to the device context of this GBuffer pass. 
		 */
		ID3D11DeviceContext4 * const m_device_context;

		/**
		 A pointer to the vertex shader of this GBuffer pass.
		 */
		const SharedPtr< const VertexShader > m_vs;
		
		/**
		 An array containing pointers to the pixel shaders of this GBuffer 
		 pass.
		 */
		const SharedPtr< const PixelShader > m_ps[static_cast< size_t >(PSIndex::Count)];
		
		/**
		 The pixel shader index of the bound pixel shader of this GBuffer pass.
		 */
		PSIndex m_bound_ps;

		/**
		 The model buffer of this GBuffer pass.
		 */
		ConstantBuffer< ModelBuffer > m_model_buffer;
	};
}