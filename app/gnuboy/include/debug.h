#ifndef DEBUG_H
#define DEBUG_H

void debug_disassemble(addr a, int c);

#ifdef DEBUG
#define debugf(...) fprintf(stderr, __VA_ARGS__)
#else
#define debugf(...) while(0){}
#endif

#endif

