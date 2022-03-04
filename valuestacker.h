#pragma once  //(C) 2022  Andy Heilveil (github/980f)

/** RAII set a value, restore on exit, nesting */
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

