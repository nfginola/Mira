#pragma once
#include <stdint.h>
#include <assert.h>
#include <memory>
#include <optional>
#include <vector>
#include <filesystem>
#include <array>
#include <unordered_map>
#include <variant>
#include <span>

using f32 = float;
using f64 = double;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i64 = int64_t;
using i32 = int32_t;

static u32 get_slot(u64 handle)
{
	static const u64 SLOT_MASK = ((u64)1 << std::numeric_limits<uint32_t>::digits) - 1; // Mask of the lower 32-bits
	return (u32)(handle & SLOT_MASK);
}

// Helper for inserting/getting resources in an array-like manner
template <typename T>
static void try_insert(std::vector<std::optional<T>>& vec, const T& element, u32 index)
{
	// resize if needed
	if (vec.size() <= index)
		vec.resize(vec.size() * 4);

	assert(!vec[index].has_value());
	vec[index] = element;
}

// move version
template <typename T>
static void try_insert_move(std::vector<std::optional<T>>& vec, T&& element, u32 index)
{
	// resize if needed
	if (vec.size() <= index)
		vec.resize(vec.size() * 4);

	assert(!vec[index].has_value());
	vec[index] = std::move(element);
}

template <typename T>
static const T& try_get(const std::vector<std::optional<T>>& vec, u32 index)
{
	assert(vec.size() > index);
	assert(vec[index].has_value());
	return *(vec[index]);
}

template <typename T>
static T& try_get(std::vector<std::optional<T>>& vec, u32 index)
{
	assert(vec.size() > index);
	assert(vec[index].has_value());
	return *(vec[index]);
}


