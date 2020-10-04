#pragma once
#include <array>
#include "common.hpp"
#include "types.hpp"

namespace riscv {

union DecoderCache
{
	using index_type = uint8_t;

	inline index_type& get(size_t idx) noexcept {
		return cache[idx];
	}

	inline index_type* get_base() noexcept {
		return &cache[0];
	}

	template <int W>
	static index_type translate(instruction_handler<W> h) noexcept;

	template <int W>
	inline void set(size_t idx, instruction_handler<W> h) noexcept {
		get(idx) = translate<W>(h);
	}

	// we are making room for the maximum amount of
	// compressed instructions, which are 16-bits
	static constexpr size_t DIVISOR = (compressed_enabled) ? 2 : 4;

	std::array<index_type, PageSize / DIVISOR> cache {};

	static constexpr size_t OPS = 256;
	template <int W>
	static const instruction_handler<W> lut [OPS];
};

}
