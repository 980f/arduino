#ifndef STREAMFORMATTER_H
#define STREAMFORMATTER_H  980F

class StreamFormatter {
protected:
  /** for controlling the scope of application of formatting. */
  struct StreamState {
    unsigned base=10;
    unsigned precision=2;
    char fill;//at present only used by scanner as 'ignore' such as for spaces in european numbers.
    bool showbase=false;
    void record(const StreamState &current);
    void restore(StreamState &current);
  };
  //initial implementation is one save and restore per print session:
  StreamState pushed;

  //arduino stream does not have format state, we do.
  StreamState current;

//format string parse states and values:
  bool parsingIndex;
  unsigned argIndex;

  bool parsingFormat;
  unsigned formatValue;

  bool slashed;//next char is fill char
  bool invertOption;
  bool keepFormat;
  //format parse state transition functions:
  /** resets all state to 'no format spec seen', records stream state for later optional restoration. */
  void beginParse();

  /** called after item has been printed, to avert bugs */
  void dropIndex();

/** called after parsing of format is done and any options have been applied */
  void dropFormat();

/** called after 'have format field' has been recognized */
  void startFormat();

/** @returns whether c is a digit,and if so then it has applied it to the accumulator  */
  bool appliedDigit(char c, unsigned &accumulator);
  /** call when the format commands has been fully acted upon, IE after responding to 'DoItem' */
  void afterActing();
  /** begin parsing the item number*/
  void startIndex();
  /** format value has been used, avert applying it to another feature */
  void clearFormatValue();

protected://now for the API:
  enum Action {
    FeedMe, //feed parser more characters
    Pass,   //process the char, for printing: output it; for scanf compare it
    Escaped,//the char was an escape char, e.g. if c==n then emit a newline.
    DoItem, //end of format field, act upon it.
  };
  /** inspects format character @param c and @returns what to do */
  Action applyItem(char c);

// the following did not work as the ostream inherited virtually from the base class that actually had the methods needed:
//  typedef std::streamsize (std::ostream::*Attributor)(std::streamsize);
//  void applyFormat(Attributor func);
  enum FormatValue {Widthly, Precisely};
  void applyFormat(FormatValue whichly);

  /** in arduino the stream doesn't have state so here we do not need to attach to a stream */
  StreamFormatter();
};
#endif // STREAMFORMATTER_H
