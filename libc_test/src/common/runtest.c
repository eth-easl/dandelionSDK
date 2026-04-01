/*
 * dlibc stub: the real runtest uses fork/exec/waitpid which dlibc
 * does not support.  This placeholder compiles and links but is never
 * actually invoked — RUN_TEST is overridden in config.mak.dlibc to
 * use a Python-based runner instead.
 */
#include "test.h"

int main(int argc, char *argv[])
{
	t_error("runtest: not usable under dlibc, use python3 ./run_test_dlibc.py\n");
	return 1;
}
