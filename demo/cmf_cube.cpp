// Copyright © 2026 Fenris Creations

#include "cmf/bounds.h"
#include "cmf/memallocator.h"
#include "cmf/utils.h"
#include "cmf/writer.h"

#include <array>
#include <cstdint>
#include <cstdio>

namespace
{

struct CubeVertex
{
	float x, y, z;
};

constexpr std::array<CubeVertex, 8> kCubeVertices = { {
	{ -0.5f, -0.5f, -0.5f },
	{ 0.5f, -0.5f, -0.5f },
	{ 0.5f, 0.5f, -0.5f },
	{ -0.5f, 0.5f, -0.5f },
	{ -0.5f, -0.5f, 0.5f },
	{ 0.5f, -0.5f, 0.5f },
	{ 0.5f, 0.5f, 0.5f },
	{ -0.5f, 0.5f, 0.5f },
} };

constexpr std::array<uint32_t, 36> kCubeIndices = { {
	0, 1, 2, 0, 2, 3,
	4, 6, 5, 4, 7, 6,
	0, 3, 7, 0, 7, 4,
	1, 5, 6, 1, 6, 2,
	3, 2, 6, 3, 6, 7,
	0, 4, 5, 0, 5, 1,
} };

struct Cube
{
	cmf::MemoryAllocator allocator;
	cmf::BufferManager buffers;
	cmf::Data data;

	Cube() :
		buffers( allocator )
	{
		const uint32_t vertexStride = sizeof( CubeVertex );
		cmf::BufferView vertices = buffers.AllocateBuffer(
			kCubeVertices.data(),
			uint32_t( kCubeVertices.size() * vertexStride ),
			vertexStride );
		cmf::BufferView indices = buffers.AllocateBuffer(
			kCubeIndices.data(),
			uint32_t( kCubeIndices.size() * sizeof( uint32_t ) ),
			sizeof( uint32_t ) );

		cmf::VertexElement position = {};
		position.usage = cmf::Usage::Position;
		position.usageIndex = 0;
		position.type = cmf::ElementType::Float32;
		position.elementCount = 3;
		position.offset = 0;

		cmf::LodMeshArea areaRange = {};
		areaRange.firstElement = 0;
		areaRange.elementCount = uint32_t( kCubeIndices.size() ) / 3;

		cmf::MeshLod lod = {};
		lod.vb = vertices;
		lod.ib = indices;
		cmf::Modify( lod.areas, allocator ).emplace_back( areaRange );

		cmf::MeshArea area = {};
		area.name = allocator.AllocateString( "CubeMaterial" );
		area.bounds = CcpMath::AxisAlignedBox(
			Vector3( -0.5f, -0.5f, -0.5f ),
			Vector3( 0.5f, 0.5f, 0.5f ) );

		cmf::Mesh mesh = {};
		mesh.name = allocator.AllocateString( "UnitCube" );
		cmf::Modify( mesh.decl, allocator ).emplace_back( position );
		cmf::Modify( mesh.lods, allocator ).emplace_back( lod );
		cmf::Modify( mesh.areas, allocator ).emplace_back( area );
		mesh.topology = cmf::MeshTopology::TriangleList;

		cmf::Modify( data.meshes, allocator ).emplace_back( mesh );
	}
};

}

int main()
{
	Cube cube;
	const CcpMath::AxisAlignedBox bounds = cmf::CalculateBounds( cube.data.meshes[0], cube.buffers );
	const std::vector<uint8_t> file = cmf::BuildFile( cube.data, cube.buffers, nullptr );
	const cmf::ValidationResult validation = cmf::ValidateFile( file.data(), file.size(), { true, true, true } );
	if( !validation || file.empty() )
	{
		std::fprintf( stderr, "CMF_CONSUMER_FAILED %s\n", validation.error.c_str() );
		return 1;
	}
	std::printf(
		"CMF_CONSUMER bytes=%zu min=%.3f,%.3f,%.3f max=%.3f,%.3f,%.3f\n",
		file.size(),
		bounds.m_min.x, bounds.m_min.y, bounds.m_min.z,
		bounds.m_max.x, bounds.m_max.y, bounds.m_max.z );
	return 0;
}
