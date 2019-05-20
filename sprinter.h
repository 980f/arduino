

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

  //diagnostic:
  static int mostused;
  ~Sprinter() {
    if (mostused < tail) {
      mostused = tail;
    }
  }
};
