
/** very slow pwm.
base frequency is whatever rate you call operator bool at.
typically this is called from the millisecond rollover that you should be checking in loop(), see millievent.h */
class SoftPwm {
  unsigned twiddler=0;
public:
  unsigned early;
  unsigned later;

  /**
  default args set up to give a one tick pulse per 1st param+1 cycles.
  */
  SoftPwm(unsigned early,unsigned later=1):early(early),later(later){}

  void reset(){
    twiddler=0;
  }

  /* call from a periodic source */
  operator bool() {
    ++twiddler;
    if(twiddler<early){
      return 0;
    }
    if(twiddler>=later){
      twiddler=0;
    }
    return 1;
  }
};
