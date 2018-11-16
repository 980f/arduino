#ifndef STREAMPRINTF_H
#define STREAMPRINTF_H

#include "streamformatter.h"
///////////////////////
/// output stream variation
///
/// //because of templates, we must include some files that normally would only be in a cpp:
#include "index.h"  //isValid
#include "char.h"

#include "Stream.h" //Arduino input stream

/** from Print.h:
 *  size_t print(const __FlashStringHelper *);
    size_t print(const Printable&);
    size_t print(const String &);
    size_t print(const char[]);
    size_t print(char);
    size_t print(unsigned char, int = DEC);
    size_t print(int, int = DEC);
    size_t print(unsigned int, int = DEC);
    size_t print(long, int = DEC);
    size_t print(unsigned long, int = DEC);
    size_t print(double, int = 2);

 * typetrait for types that support a 2nd parameter in their print() command:*/
template <typename Any> struct Formatting { enum { hasBase=true }; };

#define DOESNTSUPPORTFORMATTING(probate) template <> struct Formatting <probate>{ enum { hasBase=false }; }

DOESNTSUPPORTFORMATTING(char);
DOESNTSUPPORTFORMATTING(const __FlashStringHelper *);
DOESNTSUPPORTFORMATTING(const Printable&);
DOESNTSUPPORTFORMATTING(const String &);
DOESNTSUPPORTFORMATTING(const char[]);
//just do a dedicated write():--- template <> struct Formatting<double> { enum { supported=true, hasBase=false }; };
// end type traits

class StreamPrintf:StreamFormatter {
  Print &cout;
  void write(double c){
    cout.print(c,current.precision);    
  }
  template<typename Any> void write(Any &&c){
    if(Formatting<Any>::hasBase){
      if(current.showbase){
        switch(current.base){
          case 2: cout.print("0b"); break;
          case 8: cout.print("0"); break;
          case 16: cout.print("0x"); break;
        }
      }
      cout.print(c,current.base);
    } else {
      cout.print(c);
    }
  }

  void printMissingItem();

  /** ran out of arguments. This is needed even if it never gets called. */
  void PrintItem(unsigned ){
    //return false;//ran off end of arg list
  }

  /** this is how you process the nth item of a varargs template group.
      It can generate a surprising amount of code, a function for every combination of argument types, AND all right hand subsets thereof.
      Fortunately multiple print statements with the same argument types share code, there is no dependency on the format arg or the argument values. */
  template<typename First,typename ... Args> void PrintItem(unsigned which, First &&first, Args&& ... args){
    if(which==0) {
      write(first);
    } else {
      PrintItem(which - 1,args ...);
    }
  }

public:
  StreamPrintf(Print &stream);
  template<typename ... Args> void operator()(const char *fmt, const Args ... args){
    char c;
    beginParse();
    while((c = *fmt++)) {
      switch(applyItem(c)) {
      case DoItem:
        if(argIndex>= sizeof... (args)){
          printMissingItem();
        } else {
          PrintItem(argIndex,args ...);
        }
        afterActing();
        break;
      case Escaped:
        c=Char(c).slashee();
        //#JOIN
      case Pass:
        write(c);
        break;
      case FeedMe:
        //part of format field, nothing need be done.
        break;
      }
    }
    //if format pending then we have a bad trailing format spec, what shall we do?
    if(isValid(argIndex)){
      printMissingItem();//even if item exists we shall not print it.
    }
  } // Printf

};

#include "Stream.h"
class StreamScanf:StreamFormatter {
  Stream&cin;

  /** bytes that are not inside a format spec are to be checked for existence, and must match or we bail out*/
  bool verify(char c){
    char tocheck=cin.read();
    return c==tocheck;
  }

//todo: provide for arguments for LookaheadMode lookahead = SKIP_ALL, char ignore = NO_IGNORE_CHAR
  void scan(int *c){
    *c=cin.parseInt();
  }

  void scan(double *c){
    *c=cin.parseFloat();
  }

  void scan(char *c){
    *c=cin.read();
  }

  //todo: read string for chars speced in current.precision, somehow code access to the readUntil() functions.

  template<typename Any> void scan(Any *c){
    Any local=*c;
//    cin >> local;
    *c=local;
  }


  void scanMissingItem(){
    //not sure what to do ... the number converted will be wrong ... so the caller can debug it.
  }

  /** ran out of arguments. This is needed even if it never gets called. */
  void ScanItem(unsigned ){
    //return false;//ran off end of arg list
  }

  template<typename First,typename ... Args> void ScanItem(unsigned which, First &&first, Args&& ... args){
    if(which==0) {
      scan(first);
    } else {
      ScanItem(which - 1,args ...);
    }
  }

public:
  StreamScanf(Stream &stream);
  /** @returns the number of successfully parsed items. If that is less than the number given then the negative of that is returned.
   * an expert might be able to remove the need to put '7' in front of each argument, but at least this guy won't compile if you don't. */
  template<typename ... Args> int operator()(const char *fmt, Args&& ... args){
    char c;
    int numberScanned=0;
    beginParse();
    while((c = *fmt++)) {
      switch(applyItem(c)) {
      case DoItem:
        if(argIndex>= sizeof... (args)){
          scanMissingItem();
        } else {
          ScanItem(argIndex,args ...);
          ++numberScanned;
        }
        afterActing();
        break;
      case Escaped:
        c=Char(c).slashee();
        //#JOIN
      case Pass:
        if(!verify(c)){
          return -numberScanned;//which is OK for 0, this is a quantity not an index.
        }
        break;
      case FeedMe:
        //part of format field, nothing need be done.
        break;
      }
    }
    //if format pending then we have a bad trailing format spec, what shall we do?
    if(isValid(argIndex)){
      scanMissingItem();//even if item exists we shall not print it.
    }
    return numberScanned;
  }

};

#endif // STREAMPRINTF_H
