#pragma once

#define ANSI_RESET   "\033[0m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_DIM     "\033[2m"
#define ANSI_RED     "\033[31m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_BLUE    "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN    "\033[36m"

int color_is_supported(void);
const char *color_code(const char *ansi);
const char *color_for_type(int type);
void print_colored(const char *ansi, const char *text);
void sleep_ms(int ms);
void print_slow(const char *ansi, const char *text, int delay_ms);
