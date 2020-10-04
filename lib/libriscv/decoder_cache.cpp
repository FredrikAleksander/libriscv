#include "memory.hpp"
#include "machine.hpp"
#include "decoder_cache.hpp"
#include <stdexcept>

#include "rv32i_instr.hpp"

namespace riscv
{

#ifdef RISCV_INSTR_CACHE
	template <int W>
	void Memory<W>::generate_decoder_cache(address_t addr, size_t len)
	{
		constexpr address_t PMASK = Page::size()-1;
		const address_t pbase = addr & ~PMASK;
		const size_t prelen  = addr - pbase;
		const size_t midlen  = len + prelen;
		const size_t plen =
			(PMASK & midlen) ? ((midlen + Page::size()) & ~PMASK) : midlen;

		const size_t n_pages = plen / Page::size();
		auto* decoder_array = new DecoderCache [n_pages];
		this->m_exec_decoder =
			decoder_array[0].get_base() - pbase / DecoderCache::DIVISOR;
		this->m_decoder_cache = &decoder_array[0];

#ifdef RISCV_INSTR_CACHE_PREGEN
		const auto* exec_offset = m_exec_pagedata.get() - pbase;
		// generate instruction handler pointers for machine code
		for (address_t dst = pbase; dst < pbase + plen;)
		{
			const size_t cacheno = page_number(dst - pbase);
			const address_t offset = dst & (Page::size()-1);
			auto& cache = decoder_array[cacheno];

			if (dst >= addr && dst < addr + len)
			{
				rv32i_instruction instruction;
				instruction.whole = *(uint32_t*) &exec_offset[dst];

				cache.set(offset / cache.DIVISOR,
					machine().cpu.decode(instruction).handler);
				dst += instruction.length();
			} else {
				cache.set(offset / cache.DIVISOR,
					machine().cpu.decode({0}).handler);
				dst += 4;
			}
		}
#else
		// zero the whole thing
		std::memset(decoder_array, 0, n_pages * sizeof(decoder_array[0]));
#endif
	}
#endif

#ifndef __GNUG__
	template struct Memory<4>;
	template struct Memory<8>;
#endif
}
