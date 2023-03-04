
#include "dbgserial.h"

#include "clirp.h" //commandline interpreter wtih reverse polish input, all args preceed operator.

class SUI { //Simple User Interface. Binds together a console and an RPN command parser.
    CLIRP<> cli;
    decltype(Serial) &cin;
    ChainPrinter cout;
  public:
    SUI (decltype(Serial) &keyboard, Print&printer): cin(keyboard), cout(printer, true) {}

    using User = void(*)(char key);

    void operator()(User handler) {
      for (unsigned strokes = cin.available(); strokes-- > 0;) {
        char key = cin.read();
        bool upper = key < 'a';
        if (cli.doKey(key)) {
          handler(key);
        }
      }
    }

    unsigned has2() const {
      return cli.twoargs();
    }

    bool hasarg() const {
      return bool(cli.arg);
    }

    operator unsigned() {
      return cli.arg;
    }

    unsigned more() {
      return cli.pushed;
    }

};

