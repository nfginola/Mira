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



