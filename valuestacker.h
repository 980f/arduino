#pragma once  //(C) 2022  Andy Heilveil (github/980f)

/** RAII set a value, restore on exit, nesting 
each level of your program's stacking of a value must have its own instance of ValueStacker,
you are using the stack on which the compiler allocates locals to be the stack of restoration values for the item of interest.
Other stack implementations use heap and lots of ram and execution time.
If your stacking requirements are the same as your codes natural block nesting then this stack is very cheap in ram and time.
*/
template <typename Scalar> class ValueStacker {
    Scalar *flag;
    Scalar was;
  public:
    explicit ValueStacker(Scalar&theFlag, Scalar setit): flag(&theFlag)  {
      was = flag;
      if (flag) { //shouldn't ever be null, but a programmer can cast a nullptr into a null reference so let us guard against that.
        *flag = setit;
      }
    }

    /** make a copy but keep destructor of prior from doing anything, just so parent class can have a factory for these. */
    ValueStacker(ValueStacker &&other): flag(other.flag), was(other.was) {
      other.flag = nullptr;
    }

    /** can't chain as we must alter the argument, see && constructor */
    ValueStacker(const ValueStacker &other) = delete;

    ~ValueStacker() {
      if (flag) {
        *flag = was;
      }
    }
};
