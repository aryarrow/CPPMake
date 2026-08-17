#pragma once

#ifdef GDB
void breakpoint(){
	asm volatile ("int3");
}
#else 
void breakpoint(){
	return;
}
#endif

