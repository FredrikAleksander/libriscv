#include "syscall.hpp"
int __testable_global __attribute__((section(".bss"))) = 0;

extern "C"
{
	__attribute__((noreturn))
	void _exit(int exitval) {
		syscall(SYSCALL_EXIT, exitval);
		__builtin_unreachable();
	}
	long write(int fd, const void* data, unsigned long len) {
		return syscall(SYSCALL_WRITE, fd, (long) data, len);
	}
}

extern "C"
void _start()
{
	// initialize the global pointer to __global_pointer
	// NOTE: have to disable relaxing first
	asm volatile
	("   .option push 				\t\n\
		 .option norelax 			\t\n\
		 1:auipc gp, %pcrel_hi(__global_pointer$) \t\n\
  		 addi  gp, gp, %pcrel_lo(1b) \t\n\
		.option pop					\t\n\
	");
	asm volatile("" ::: "memory");
	// testable global
	__testable_global = 1;
	// zero-initialize .bss section
	extern char __bss_start;
	extern char __BSS_END__;
	for (char* bss = &__bss_start; bss < &__BSS_END__; bss++) {
		*bss = 0;
	}
	asm volatile("" ::: "memory");
	// exit out if the .bss section did not get initialized
	if (__testable_global != 0) {
		_exit(-1);
	}

	// call global constructors
	extern void(*__init_array_start [])();
	extern void(*__init_array_end [])();
	int count = __init_array_end - __init_array_start;
	for (int i = 0; i < count; i++) {
		__init_array_start[i]();
	}

	// call main() :)
	extern int main(int, char**);
	_exit(main(0, nullptr));
}
