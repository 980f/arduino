#pragma once //(C) 2019 Andy Heilveil, github/980f

/** a cheap sequence recognizer.
    present it with your tokens, it returns whether the sequence might be present.
    the ~operator reports on whether the sequence was recognized, and if so then forgets that it was.
    the operator bool reports on whether the complete sequence has been recognized, it is cleared when presented another token so must be checked with each token.
*/
class LinearRecognizer {
    const char * const seq;
    short si = 0; //Arduino optimization

  public:
    LinearRecognizer(const char *seq): seq(seq) {}

    /** @returns whether sequence has been seen */
    operator bool() const {
      return seq[si] == 0;
    }

    /** @returns whether sequence has been recognized, in which case recognition is forgotten. */
    bool operator ~() {
      if (*this) {
        si = 0;
        return true;
      } else {
        return false;
      }
    }

    /** @returns whether @param next was expected part of sequence. NUL's are ignored. */
    bool operator ()(char next) {
      if (next == 0) { //#if we don't exclude nulls we can overrun our sequence definitoin storage.
        return si != 0; //maintain state
      }
      if (next == seq[si]) {
        ++si;
        return true;
      } else {
        si = 0;
        return false;
      }
    }

    /** coerce to 'recognized' else back to 0*/
    void operator=(bool fakeit) {
      if (fakeit) {
        while (seq[si]) {
          ++si;
        }
      } else {
        si = 0;
      }
    }
};
