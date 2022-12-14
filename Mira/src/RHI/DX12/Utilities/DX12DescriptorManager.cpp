#include "DX12DescriptorManager.h"
#include "DX12DescriptorHeap.h"
#include "DX12DescriptorAllocatorDMA.h"

DX12DescriptorManager::DX12DescriptorManager(ID3D12Device* device)
{
	init_heaps(device);
	init_allocators();
}

DX12DescriptorManager::~DX12DescriptorManager()
{
}

DX12DescriptorChunk DX12DescriptorManager::allocate(uint32_t num_descriptors, D3D12_DESCRIPTOR_HEAP_TYPE heap_type)
{
	switch (heap_type)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return m_gpu_dh_resource_ator->allocate(num_descriptors);
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		return m_gpu_dh_sampler_ator->allocate(num_descriptors);
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return m_cpu_dh_rtv_ator->allocate(num_descriptors);
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return m_cpu_dh_dsv_ator->allocate(num_descriptors);
	default:
		assert(false);
	}

	assert(false);
	return DX12DescriptorChunk();
}

void DX12DescriptorManager::free(DX12DescriptorChunk* chunk)
{
	switch (chunk->heap_type())
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return m_gpu_dh_resource_ator->free(chunk);
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		return m_gpu_dh_sampler_ator->free(chunk);
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return m_cpu_dh_rtv_ator->free(chunk);
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return m_cpu_dh_dsv_ator->free(chunk);
	default:
		assert(false);
	}
}

ID3D12DescriptorHeap* DX12DescriptorManager::get_gpu_dh_resource() const
{
	return *m_gpu_dh_resource;
}

ID3D12DescriptorHeap* DX12DescriptorManager::get_gpu_dh_sampler() const
{
	return *m_gpu_dh_sampler;
}

void DX12DescriptorManager::init_heaps(ID3D12Device* device)
{
	m_gpu_dh_resource = std::make_unique<DX12DescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100'000, true);
	m_gpu_dh_sampler = std::make_unique<DX12DescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1'000, true);
	m_cpu_dh_rtv = std::make_unique<DX12DescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1'000, false);
	m_cpu_dh_dsv = std::make_unique<DX12DescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1'000, false);
}

void DX12DescriptorManager::init_allocators()
{
	m_gpu_dh_resource_ator = std::make_unique<DX12DescriptorAllocatorDMA>(m_gpu_dh_resource->as_chunk());
	m_gpu_dh_sampler_ator = std::make_unique<DX12DescriptorAllocatorDMA>(m_gpu_dh_sampler->as_chunk());
	m_cpu_dh_rtv_ator = std::make_unique<DX12DescriptorAllocatorDMA>(m_cpu_dh_rtv->as_chunk());
	m_cpu_dh_dsv_ator = std::make_unique<DX12DescriptorAllocatorDMA>(m_cpu_dh_dsv->as_chunk());
}
