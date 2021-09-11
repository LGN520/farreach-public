// NetBuffer

#ifndef BACKTRACE_H
#define BACKTRACE_H

// ## not only concatenates args, but also delete trailing comma is the parameter is empty
#define print_msg(fmt, ...) printf(fmt, ##__VA_ARGS__)

void backtrace();
//void print_msg(const char * fmt, ...);

#endif
