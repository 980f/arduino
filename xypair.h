/** many things come in pairs.
  This template allows assignment of XY pair of assignable entities,
  construction from 0,1, or 2 args of same type.

  Made arduino specific by addition of printable.
*/

#include <Printable.h>
#include "cheaptricks.h"  //for changed()

template<typename T> struct XY: public Printable  {
  T X;
  T Y;

  /**only works if class has default constructor:*/
  XY() {
    //#done
  }

  /** single arg constructor */
  template<typename Xarg, typename Yarg> XY(Xarg x, Yarg y):
    X(x), Y(y) {
    //#nada
  }

  /** two arg constructor */
  template<typename Xarg1, typename Yarg1, typename Xarg2, typename Yarg2> XY(Xarg1 x1, Xarg2 x2, Yarg1 y1, Yarg2 y2):
    X(x1, x2), Y(y1, y2) {
    //#nada
  }

  //type Other must be assignable to typename T
  template <typename Other> XY<T>&operator =(XY<Other> input) {
    X = input.X;
    Y = input.Y;
    return *this;
  }

  /** @returns whether assigning to this from the input caused a change. type Other must be assignable to typename T */
  template <typename Other> bool assignFrom(XY<Other> input) {
    return changed(X, input.X) | changed(Y , input.Y); //# single '|' or required! do not chnge to ||, we need to evaluate both operands.
  }


  size_t printTo(Print& p) const {
    return p.print(X) + p.print(',') + p.print(Y);
  }

};
