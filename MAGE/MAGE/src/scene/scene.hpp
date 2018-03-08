#pragma once

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "scene\node.hpp"
#include "collection\vector.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Declarations and Definitions
//-----------------------------------------------------------------------------
namespace mage {

	/**
	 A class of scenes.
	 */
	class Scene {

	public:

		//---------------------------------------------------------------------
		// Destructors
		//---------------------------------------------------------------------

		/**
		 Destructs this scene.
		 */
		virtual ~Scene();

		//---------------------------------------------------------------------
		// Assignment Operators
		//---------------------------------------------------------------------	

		/**
		 Copies the given scene to this scene.

		 @param[in]		scene
						A reference to the scene to copy.
		 @return		A reference to the copy of the given scene (i.e. this 
						scene).
		 */
		Scene& operator=(const Scene& scene) = delete;

		/**
		 Moves the given scene to this scene.

		 @param[in]		scene
						A reference to the scene to move.
		 @return		A reference to the moved scene (i.e. this scene).
		 */
		Scene& operator=(Scene&& scene) noexcept;

		//-------------------------------------------------------------------------
		// Member Methods: Lifecycle
		//-------------------------------------------------------------------------

		/**
		 Initializes this scene.

		 @throws		Exception
						Failed to initialize this scene.
		 */
		void Initialize();

		/**
		 Uninitializes this scene.

		 @throws		Exception
						Failed to unitialize this scene.
		 */
		void Uninitialize();

		//---------------------------------------------------------------------
		// Member Methods: Nodes and Components
		//---------------------------------------------------------------------

		ProxyPtr< Node > Import(const ModelDescriptor &desc);
		ProxyPtr< Node > Import(const ModelDescriptor &desc,
			std::vector< ProxyPtr< Node > > &nodes);

		template< typename ElementT, typename... ConstructorArgsT >
		typename std::enable_if_t< std::is_same_v< Node, ElementT >,
			ProxyPtr< ElementT > > Create(ConstructorArgsT &&...args);

		template< typename ElementT, typename... ConstructorArgsT >
		typename std::enable_if_t< std::is_base_of_v< BehaviorScript, ElementT >,
			ProxyPtr< ElementT > > Create(ConstructorArgsT &&...args);

		template< typename ElementT >
		[[nodiscard]]ElementT &Get(size_t index) noexcept;

		template< typename ElementT >
		[[nodiscard]]const ElementT &Get(size_t index) const noexcept;

		template< typename ElementT >
		[[nodiscard]]size_t GetNumberOf() const noexcept;

		template< typename ElementT, typename ActionT >
		void ForEach(ActionT action);

		template< typename ElementT, typename ActionT >
		void ForEach(ActionT action) const;

		//---------------------------------------------------------------------
		// Member Methods: Identification
		//---------------------------------------------------------------------

		/**
		 Returns the name of this scene.

		 @return		A reference to the name of this scene.
		 */
		[[nodiscard]]
		const string& GetName() const noexcept {
			return m_name;
		}

		/**
		 Sets the name of this scene to the given string.

		 @param[in]		name
						The name.
		 */
		void SetName(string name) noexcept {
			m_name = std::move(name);
		}

	protected:

		//---------------------------------------------------------------------
		// Constructors
		//---------------------------------------------------------------------

		/**
		 Constructs a scene.

		 @param[in]		name
						The name of the scene.
		 */
		explicit Scene(string name);

		/**
		 Constructs a scene from the given scene.

		 @param[in]		scene
						A reference to the scene.
		 */
		Scene(const Scene& scene) = delete;

		/**
		 Constructs a scene by moving the given scene.

		 @param[in]		scene
						A reference to the scene to move.
		 */
		Scene(Scene&& scene) noexcept;

	private:

		//---------------------------------------------------------------------
		// Member Methods: Lifecycle
		//---------------------------------------------------------------------

		/**
		 Loads this scene. Allows this scene to preform any pre-processing 
		 construction.

		 @throws		Exception
						Failed to load this scene.
		 */
		virtual void Load();

		/**
		 Closes this scene. Allows this scene to preform any post-processing 
		 destruction.

		 @throws		Exception
						Failed to close this scene.
		 */
		virtual void Close();

		/**
		 Clears this scene.
		 */
		void Clear() noexcept;

		//---------------------------------------------------------------------
		// Member Variables: Nodes and Components
		//---------------------------------------------------------------------

		/**
		 A vector containing the nodes of this scene.
		 */
		AlignedVector< Node > m_nodes;

		/**
		 A vector containing the pointers to the scripts of this scene.
		 */
		AlignedVector< UniquePtr< BehaviorScript > > m_scripts;

		//---------------------------------------------------------------------
		// Member Variables: Identification
		//---------------------------------------------------------------------

		/**
		 The name of this scene.
		 */
		string m_name;
	};
}

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "scene\scene.tpp"

#pragma endregion