#ifndef _KBHIT_H_
#define _KBHIT_H_

int set_tty_raw();
int set_tty_cbreak();
int set_tty_cooked();
unsigned char kb_getc();
unsigned char kb_getc_w();
void echo(unsigned char ch);


#endif // _KBHIT_H_
