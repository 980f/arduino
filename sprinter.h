/**
wraps use of vsnprintf on a buffer, to prevent writing past the end of a buffer.
*/

struct Sprinter {
  const int guard;
  char * const buffer;

  int tail;
  Sprinter &rewind();

  Sprinter(char * buffer, unsigned length);

  char * printf(const char *format, ...) ;

  operator char *() {
    return buffer;
  }

  //diagnostic, tracks the most used by *any* Sprinter, so that a shared buffer can be precisely allocated.
  static int mostused;
  ~Sprinter() {
    if (mostused < tail) {
      mostused = tail;
    }
  }
};
