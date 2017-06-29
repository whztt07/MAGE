#pragma once

//-----------------------------------------------------------------------------
// Engine Includes
//-----------------------------------------------------------------------------
#pragma region

#include "rendering\rendering_factory.hpp"
#include "logging\error.hpp"
#include "logging\exception.hpp"

#pragma endregion

//-----------------------------------------------------------------------------
// Engine Definitions
//-----------------------------------------------------------------------------
namespace mage {

	template < typename VertexT >
	StaticMesh::StaticMesh(const VertexT *vertices, size_t nb_vertices,
		const uint32_t *indices, size_t nb_indices)
		: StaticMesh(GetRenderingDevice(), GetRenderingDeviceContext(),
			vertices, nb_vertices, indices, nb_indices) {}

	template < typename VertexT >
	StaticMesh::StaticMesh(ID3D11Device2 *device, ID3D11DeviceContext2 *device_context,
		const VertexT *vertices, size_t nb_vertices, const uint32_t *indices, size_t nb_indices)
		: Mesh(device, device_context, sizeof(VertexT)), m_aabb(), m_bs() {

		Assert(vertices);
		Assert(indices);
		
		SetupBoundingVolumes(vertices, nb_vertices);
		SetupVertexBuffer(vertices, nb_vertices);
		SetupIndexBuffer(indices, nb_indices);
	}

	template < typename VertexT >
	StaticMesh::StaticMesh(const vector< VertexT > &vertices, const vector< uint32_t > &indices)
		: StaticMesh(GetRenderingDevice(), GetRenderingDeviceContext(),
			vertices, indices) {}

	template < typename VertexT >
	StaticMesh::StaticMesh(ID3D11Device2 *device, ID3D11DeviceContext2 *device_context,
		const vector< VertexT > &vertices, const vector< uint32_t > &indices)
		: StaticMesh(device, device_context,
			vertices.data(), vertices.size(),
			indices.data(), indices.size()) {}

	template < typename VertexT >
	void StaticMesh::SetupBoundingVolumes(const VertexT *vertices, size_t nb_vertices) noexcept {
		for (const VertexT *v = vertices; v < vertices + nb_vertices; ++v) {
			m_aabb = Union(m_aabb, *v);
		}
		
		m_bs.m_p = m_aabb.Centroid();
		
		for (const VertexT *v = vertices; v < vertices + nb_vertices; ++v) {
			m_bs = Union(m_bs, *v);
		}
	}

	template < typename VertexT >
	void StaticMesh::SetupVertexBuffer(const VertexT *vertices, size_t nb_vertices) {
		const HRESULT result_vertex_buffer = CreateStaticVertexBuffer< VertexT >(m_device, m_vertex_buffer.ReleaseAndGetAddressOf(), vertices, nb_vertices);
		if (FAILED(result_vertex_buffer)) {
			throw FormattedException("Vertex buffer creation failed: %08X.", result_vertex_buffer);
		}

		SetNumberOfVertices(nb_vertices);
	}
}