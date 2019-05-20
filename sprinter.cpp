#include "sprinter.h"
#include "stdio.h"
#include <stdarg.h>

int Sprinter::mostused = 0;

Sprinter &Sprinter::rewind() {
  tail = 0;
  return *this;
}

Sprinter::Sprinter(char * buffer, unsigned length): guard(length), buffer(buffer) {
  rewind();
}

char * Sprinter::printf(const char *format, ...) {
  va_list args;
  va_start (args, format);
  tail += vsnprintf(buffer + tail, guard - tail, format, args);
  va_end (args);
  return buffer;//to inline print as an argument
}
