#pragma once

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "resource\resource_pool.hpp"
#include "model\model_descriptor.hpp"
#include "shader\shader.hpp"
#include "sprite\font\sprite_font.hpp"
#include "scripting\variable_script.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Declarations and Definitions
//-----------------------------------------------------------------------------
namespace mage {

	/**
	 A class of resource managers.
	 */
	class ResourceManager final {

	public:

		//---------------------------------------------------------------------
		// Type Declarations and Definitions
		//---------------------------------------------------------------------

		/**
		 A struct of resource records.

		 @tparam		ResourceT
						The resource type.
		 */
		template< typename ResourceT >
		struct ResourceRecord { 

			/**
			 The pool type of resource pools containing resources of the given
			 type.
			 */
			using pool_type = ResourcePool< wstring, const ResourceT >;

			/**
			 The key type of resource pools containing resources of the given 
			 type.
			 */
			using key_type   = typename pool_type::key_type;

			/**
			 The value type of resource pools containing resources of the given 
			 type.
			 */
			using value_type = typename pool_type::value_type;
		};
		
		/**
		 A struct of vertex shader resource records.
		 */
		template<>
		struct ResourceRecord< VertexShader > {

			/**
			 The pool type of resource pools containing vertex shaders.
			 */
			using pool_type  = PersistentResourcePool< wstring, const VertexShader >;
			
			/**
			 The key type of resource pools containing vertex shaders.
			 */
			using key_type   = typename pool_type::key_type;
			
			/**
			 The value type of resource pools containing vertex shaders.
			 */
			using value_type = typename pool_type::value_type;
		};

		/**
		 A struct of hull shader resource records.
		 */
		template<>
		struct ResourceRecord< HullShader > {
			
			/**
			 The pool type of resource pools containing hull shaders.
			 */
			using pool_type  = PersistentResourcePool< wstring, const HullShader >;
			
			/**
			 The key type of resource pools containing hull shaders.
			 */
			using key_type   = typename pool_type::key_type;
			
			/**
			 The value type of resource pools containing hull shaders.
			 */
			using value_type = typename pool_type::value_type;
		};

		/**
		 A struct of domain shader resource records.
		 */
		template<>
		struct ResourceRecord< DomainShader > {
			
			/**
			 The pool type of resource pools containing domain shaders.
			 */
			using pool_type  = PersistentResourcePool< wstring, const DomainShader >;
			
			/**
			 The key type of resource pools containing domain shaders.
			 */
			using key_type   = typename pool_type::key_type;
			
			/**
			 The value type of resource pools containing domain shaders.
			 */
			using value_type = typename pool_type::value_type;
		};

		/**
		 A struct of geometry shader resource records.
		 */
		template<>
		struct ResourceRecord< GeometryShader > {
			
			/**
			 The pool type of resource pools containing geometry shaders.
			 */
			using pool_type  = PersistentResourcePool< wstring, const GeometryShader >;
			
			/**
			 The key type of resource pools containing geometry shaders.
			 */
			using key_type   = typename pool_type::key_type;
			
			/**
			 The value type of resource pools containing geometry shaders.
			 */
			using value_type = typename pool_type::value_type;
		};

		/**
		 A struct of pixel shader resource records.
		 */
		template<>
		struct ResourceRecord< PixelShader > {
			
			/**
			 The pool type of resource pools containing pixel shaders.
			 */
			using pool_type  = PersistentResourcePool< wstring, const PixelShader >;
			
			/**
			 The key type of resource pools containing pixel shaders.
			 */
			using key_type   = typename pool_type::key_type;

			/**
			 The value type of resource pools containing pixel shaders.
			 */
			using value_type = typename pool_type::value_type;
		};

		/**
		 A struct of compute shader resource records.
		 */
		template<>
		struct ResourceRecord< ComputeShader > {

			/**
			 The pool type of resource pools containing compute shaders.
			 */
			using pool_type  = PersistentResourcePool< wstring, const ComputeShader >;
			
			/**
			 The key type of resource pools containing compute shaders.
			 */
			using key_type   = typename pool_type::key_type;
			
			/**
			 The value type of resource pools containing compute shaders.
			 */
			using value_type = typename pool_type::value_type;
		};

		/**
		 A struct of variable script resource records.
		 */
		template<>
		struct ResourceRecord< VariableScript >{
			
			/**
			 The pool type of resource pools containing variable scripts.
			 */
			using pool_type  = ResourcePool< wstring, VariableScript >;
			
			/**
			 The key type of resource pools containing variable scripts.
			 */
			using key_type   = typename pool_type::key_type;
			
			/**
			 The value type of resource pools containing variable scripts.
			 */
			using value_type = typename pool_type::value_type;
		};

		/**
		 The pool type of resource pools containing resources of the given 
		 type.

		 @tparam		ResourceT
						The resource type.
		 */
		template< typename ResourceT >
		using pool_type = typename ResourceRecord< ResourceT >::pool_type;
		
		/**
		 The key type of resource pools containing resources of the given 
		 type.

		 @tparam		ResourceT
						The resource type.
		 */
		template< typename ResourceT >
		using key_type = typename ResourceRecord< ResourceT >::key_type;
		
		/**
		 The value type of resource pools containing resources of the given 
		 type.

		 @tparam		ResourceT
						The resource type.
		 */
		template< typename ResourceT >
		using value_type = typename ResourceRecord< ResourceT >::value_type;

		//---------------------------------------------------------------------
		// Class Member Methods
		//---------------------------------------------------------------------

		/**
		 Returns the resource manager associated with the current engine.

		 @pre			The current engine must exist.
		 @return		A pointer to the resource manager associated with the 
						current engine.
		 */
		[[nodiscard]] static ResourceManager *Get() noexcept;

		//---------------------------------------------------------------------
		// Constructors and Destructors
		//---------------------------------------------------------------------

		/**
		 Constructs a resource manager.
		 */
		ResourceManager();
		
		/**
		 Constructs a resource manager from the given resource manager.

		 @param[in]		manager
						A reference to the resource manager to copy.
		 */
		ResourceManager(const ResourceManager &manager) = delete;
		
		/**
		 Constructs a resource manager by moving the given resource manager.

		 @param[in]		manager
						A reference to the resource manager to move.
		 */
		ResourceManager(ResourceManager &&manager) = delete;
		
		/**
		 Destructs this resource manager.
		 */
		~ResourceManager();

		//---------------------------------------------------------------------
		// Assignment Operators
		//---------------------------------------------------------------------

		/**
		 Copies the given resource manager to this resource manager.

		 @param[in]		manager
						A reference to the resource manager to copy.
		 @return		A reference to the copy of the given resource manager
						(i.e. this resource manager).
		 */
		ResourceManager &operator=(const ResourceManager &manager) = delete;
		
		/**
		 Moves the given resource manager to this resource manager.

		 @param[in]		manager
						A reference to the resource manager to move.
		 @return		A reference to the moved resource manager (i.e. this 
						resource manager).
		 */
		ResourceManager &operator=(ResourceManager &&manager) = delete;
		
		//---------------------------------------------------------------------
		// Member Methods
		//---------------------------------------------------------------------

		/**
		 Checks whether this resource manager contains a resource of the given 
		 type corresponding to the given globally unique identifier.

		 @tparam		ResourceT
						The resource type.
		 @param[in]		guid
						A reference to the globally unique identifier of the 
						resource.
		 @return		@c true if this resource managers contains a resource 
						of the given type corresponding to the given globally 
						unique identifier. @c false otherwise.
		 */
		template< typename ResourceT >
		[[nodiscard]] bool 
			Contains(const typename key_type< ResourceT > &guid) noexcept;
		
		/**
		 Returns the resource of the given type corresponding to the given 
		 globally unique identifier of this resource manager.

		 @tparam		ResourceT
						The resource type.
		 @param[in]		guid
						A reference to the globally unique identifier of the 
						model descriptor.
		 @return		@c nullptr, if this resource managers does not contain
						a resource of the given type corresponding to the given 
						globally unique identifier.
		 @return		A pointer to the resource.
		 */
		template< typename ResourceT >
		[[nodiscard]] SharedPtr< typename value_type< ResourceT > >
			Get(const typename key_type< ResourceT > &guid) noexcept;

		/**
		 Creates a resource of the given type (if not existing).

		 @tparam		ResourceT
						The resource type.
		 @tparam		ConstructorArgsT
						The constructor argument types of the resource 
						(excluding the type of the globally unique identifier).
		 @param[in]		guid
						A reference to the globally unique identifier of the 
						resource.
		 @param[in]		args
						A reference to the constructor arguments for the 
						resource (excluding the globally unique identifier).
		 @return		A pointer to the resource.
		 @throws		Exception
						Failed to create the resource.
		 */
		template< typename ResourceT, typename... ConstructorArgsT >
		SharedPtr< typename value_type< ResourceT > >
			GetOrCreate(const typename key_type< ResourceT > &guid,
				        ConstructorArgsT &&...args);

	private:

		//---------------------------------------------------------------------
		// Member Methods
		//---------------------------------------------------------------------

		/**
		 Returns the resource pool containing resources of the given type of 
		 this resource manager.
		
		 @tparam		ResourceT
						The resource type.
		 @return		A reference to the the resource pool containing 
						resources of the given type of this resource manager.
		 */
		template< typename ResourceT >
		[[nodiscard]] typename pool_type< ResourceT > &
			GetPool() noexcept;

		/**
		 Returns the resource pool containing resources of the given type of 
		 this resource manager.
		
		 @tparam		ResourceT
						The resource type.
		 @return		A reference to the the resource pool containing 
						resources of the given type of this resource manager.
		 */
		template< typename ResourceT >
		[[nodiscard]] const typename pool_type< ResourceT > &
			GetPool() const noexcept;

		//---------------------------------------------------------------------
		// Member Variables
		//---------------------------------------------------------------------

		/**
		 The model descriptor resource pool of this resource manager.
		 */
		typename pool_type< ModelDescriptor > m_model_descriptor_pool;

		/**
		 The vertex shader resource pool of this resource manager.
		 */
		typename pool_type< VertexShader > m_vs_pool;

		/**
		 The hull shader resource pool of this resource manager.
		 */
		typename pool_type< HullShader > m_hs_pool;

		/**
		 The domain shader resource pool of this resource manager.
		 */
		typename pool_type< DomainShader > m_ds_pool;

		/**
		 The geometry shader resource pool of this resource manager.
		 */
		typename pool_type< GeometryShader > m_gs_pool;

		/**
		 The pixel shader resource pool of this resource manager.
		 */
		typename pool_type< PixelShader > m_ps_pool;

		/**
		 The compute shader resource pool of this resource manager.
		 */
		typename pool_type< ComputeShader > m_cs_pool;

		/**
		 The sprite font resource pool of this resource manager.
		 */
		typename pool_type< SpriteFont > m_sprite_font_pool;

		/**
		 The texture resource pool of this resource manager.
		 */
		typename pool_type< Texture > m_texture_pool;

		/**
		 The variable script resource pool of this resource manager.
		 */
		typename pool_type< VariableScript > m_variable_script_pool;
	};
}

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "resource\resource_manager.tpp"

#pragma endregion