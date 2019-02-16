//(C) Copyright 2018, Andrew Heilveil, github/980F.
#pragma once

//avr is missing this stuff: #include <type_traits>
/** simplest printer extension.
 *  This wraps a Print object to allow printing a series of fields in one call:
 *  usage;  ChainPrinter chp(somePrinter); 
 *          ...
 *          chp("This", 42, 'c', 12.4, anythingforwhichthereisaprintvariant);
 *          
 *  Initial design returns what the Print::print routines do, all nicely added up. This however precludes chaining to an end-of-line function.
*/

/** this exists so that we can pick radix in the middle of a print sequence.
the 5 radixable types of Print are:
 size_t print(unsigned char, int = DEC);
    size_t print(int, int = DEC);
    size_t print(unsigned int, int = DEC);
    size_t print(long, int = DEC);
    size_t print(unsigned long, int = DEC);
*/
struct Radix {
	byte base;
	Radix(byte base):base(base){}
	void operator =(const Radix&other){
		base=other.base;
	}
	//Print.print uses int for radix.
	operator int(){
		return take(base);
	}
};

template <typename Numeric> struct IsRadixable {
	static const bool yep=false;
};

#define UseRadixFor(tname) template <> struct IsRadixable< tname > {static const bool yep=true;};
UseRadixFor(unsigned char)
UseRadixFor(int)
UseRadixFor(unsigned int)
UseRadixFor(long)
UseRadixFor(unsigned long)

//precreate the common ones.
const Radix Hex(16);
const Radix Dec(10);
const Radix Binary(2);

struct ChainPrinter {
    Print &raw;
    Radix radix=10;
    ChainPrinter(Print &raw): raw(raw) {}
  private:
    /** this is how you process the nth item of a varargs template group.
        It can generate a surprising amount of code, a function for every combination of argument types, AND all right hand subsets thereof.
        Fortunately multiple print statements with the same argument types share code, there is no dependency on the format arg or the argument values. */
    template<typename First, typename ... Args> unsigned PrintItem(First &&first, Args&& ... args) {  
    	if(IsRadixable<First>::yep){
    		 return raw.print(first,int(radix)) + PrintItem(args ...); 	
    	}
      return raw.print(first) + PrintItem(args ...);
    }


    template<typename ... Args> unsigned PrintItem(Radix &&first, Args&& ... args) {
    	radix=first;//record
      return PrintItem(args ...);
    }

    template<typename Single> unsigned PrintItem(Single &&arg) {
      return raw.print(arg);
    }

  public:
    template<typename ... Args> unsigned operator()(const Args ... args) {
      if (sizeof... (args)) {
        return PrintItem(args ...);
      } else {
        return 0;
      }
    }

    //print with a newline after all the given args, 
    template<typename ... Args> unsigned line(const Args ... args) {
      return operator()(args ...)+endl();
    }

    unsigned endl() {
      return raw.println();
    }

};
