/** many things come in pairs.
This template allows assignment of XY pair of assignable entities, 
construction from 0,1, or 2 args of same type.
 */
template<typename T> struct XY {
  T X;
  T Y;

  /**only works if class has default constructor:*/
  XY() {
    //#done
  }

  /** single arg constructor */
  template<typename Carg>
  XY(Carg x, Carg y):
    X(x), Y(y) {
    //#nada
  }

  /** two arg constructor */
  template<typename Carg1, typename Carg2>
  XY(Carg1 x1, Carg2 x2, Carg1 y1, Carg2 y2):
    X(x1, x2), Y(y1, y2) {
    //#nada
  }

  //type Other must be assignable to typename T
  template <typename Other>operator =(XY<Other> input) {
    X = input.X;
    Y = input.Y;
  }

};

