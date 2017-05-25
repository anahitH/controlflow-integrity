#include <stdio.h>
#include <execinfo.h>
#include <unistd.h>

void check() {
  void *array[100];
  size_t size;

  size = backtrace(array, 100);

  backtrace_symbols_fd(array, size, STDOUT_FILENO);
}
