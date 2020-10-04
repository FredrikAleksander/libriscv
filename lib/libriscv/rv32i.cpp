#include "rv32i_instr.hpp"
#include "machine.hpp"

#define INSTRUCTION(x, ...) static constexpr CPU<4>::instruction_t instr32i_##x { __VA_ARGS__ }
#define DECODED_INSTR(x) instr32i_##x
#include "rvi_instr.cpp"
#ifdef RISCV_EXT_ATOMICS
#include "rva_instr.cpp"
#endif
#include "rvc_instr.cpp"
#include "rvf_instr.cpp"

namespace riscv
{
	template<>
	const CPU<4>::instruction_t& CPU<4>::decode(const format_t instruction) const
	{
#define DECODER(x) return(x)
#include "instr_decoding.inc"
#undef DECODER
	}

	template<>
	void CPU<4>::execute(const format_t instruction)
	{
#define DECODER(x) { x.handler(*this, instruction); return; }
#include "instr_decoding.inc"
#undef DECODER
	}

	std::string RV32I::to_string(CPU<4>& cpu, instruction_format<4> format, const instruction_t& instr)
	{
		char buffer[256];
		char ibuffer[128];
		int  ibuflen = instr.printer(ibuffer, sizeof(ibuffer), cpu, format);
		int  len = 0;
		if (format.length() == 4) {
			len = snprintf(buffer, sizeof(buffer),
					"[%08X] %08X %.*s",
					cpu.pc(), format.whole, ibuflen, ibuffer);
		}
		else if (format.length() == 2) {
			len = snprintf(buffer, sizeof(buffer),
					"[%08X]     %04hX %.*s",
					cpu.pc(), (uint16_t) format.whole, ibuflen, ibuffer);
		}
		else {
			throw MachineException(UNIMPLEMENTED_INSTRUCTION_LENGTH,
				"Unimplemented instruction format length", format.length());
		}
		return std::string(buffer, len);
	}
}

#ifdef RISCV_INSTR_CACHE
#define RVIARCH 4
#include "decoder_translation.cpp"
#endif
