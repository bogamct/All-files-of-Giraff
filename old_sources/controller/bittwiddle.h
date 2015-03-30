#ifndef BIT_TWIDDLE_H
#define BIT_TWIDDLE_H

#define sbi(reg,bit)    reg |= (BV(bit))
#define cbi(reg,bit)    reg &= ~(BV(bit))
#define BV(bit)                 (1<<(bit))
#define inb(addr)                       (addr)

#endif
