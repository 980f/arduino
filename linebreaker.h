///////////////////////////////////////////////////////
/** send a break to a hareware serial port, which excludes USB */

struct LineBreaker {
  HardwareSerial *port = nullptr;
  uint32_t baud;
  unsigned txpin = NaV;

  MilliTick breakMillis;

  void attach(HardwareSerial &port, uint32_t baudafter, unsigned pin, unsigned overkill = 1) {
    txpin = pin;
    baud = baudafter;
    breakMillis = ceil((11000.0F * overkill) / baud);//11 bits and 1000 millis per second over bits per second
    this->port = &port;
  }

  OneShot breakEnds;
  OneShot okToUse ;

  void engage() {
    if (port && txpin != NaV) {
      port->end();//make uart let go of pin
      pinMode(txpin, OUTPUT); //JIC serial.end() mucked with it
      digitalWrite(txpin, HIGH);
      breakEnds = breakMillis;
    }
  }

  /** @returns true once when break is completed*/
  bool onTick() {
    if (breakEnds) {
      digitalWrite(txpin, LOW);
      port->begin(baud);//restart uart
      okToUse = 1; //too convoluted to wait just one bit so we wait a full ms
      return false;
    }
    return okToUse;
  }

};
