#pragma once

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "support.hpp"
#include "parallel\atomic.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Declarations and Definitions
//-----------------------------------------------------------------------------
namespace mage {

	/**
	 A class of resources.
	 */
	class Resource {

	public:

		/**
		 Constructs a resource with a given name and path.

		 @param[in]		name
						A reference to the name.
		 @param[in]		path
						A reference to the path.
		 */
		Resource(const string &name, const string &path = "./")
			: m_name(name), m_path(path), m_resource_reference_count(1) {}

		/**
		 Destructs this resource.
		 */
		virtual ~Resource() {}

		/**
		 Returns the name of this resource.

		 @return		A reference to the name of this resource.
		 */
		const string &GetName() const {
			return m_name;
		}
		
		/**
		 Returns the path of this resource.

		 @return		A reference to the path of this resource.
		 */
		const string &GetPath() const {
			return m_path;
		}
		
		
		/**
		 Returns the filename of this resource.

		 @return		The filename of this resource.
		 */
		const string GetFilename() const {
			return m_path + m_name;
		}

	private:

		template< typename T >
		friend class ResourceManager;

		/**
		 Increments the resource reference count of this reference counted object.

		 @return		The final resource reference count of this reference counted object.
		 */
		inline uint32_t IncrementResourceReferenceCount() {
			return (uint32_t)AtomicAdd(&m_resource_reference_count, 1);
		}

		/**
		 Decrements the resource reference count of this reference counted object.

		 @return		The final resource reference count of this reference counted object.
		 */
		inline uint32_t DecrementResourceReferenceCount() {
			return (uint32_t)AtomicAdd(&m_resource_reference_count, -1);
		}

		/**
		 The resource reference count of this resource.
		 */
		AtomicInt32 m_resource_reference_count;

		/**
		 The name of this resource.
		 */
		const string m_name;

		/**
		 The path of this resource.
		 */
		const string m_path;
	};
}