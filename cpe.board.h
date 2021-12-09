/**
  piece by piece exposure of circuit playground express libraries, looking for overlaps with my other adafruit M0 based boards.
  #define CPLAY_LEFTBUTTON 4        ///< left button pin
  #define CPLAY_RIGHTBUTTON 5       ///< right button pin
  #define CPLAY_SLIDESWITCHPIN 7    ///< slide switch pin
  #define CPLAY_NEOPIXELPIN 8       ///< neopixel pin
  #define CPLAY_REDLED 13           ///< red led pin
  #define CPLAY_IR_EMITTER 25       ///< IR emmitter pin
  #define CPLAY_IR_RECEIVER 26      ///< IR receiver pin
  #define CPLAY_BUZZER A0           ///< buzzer pin
  #define CPLAY_LIGHTSENSOR A8      ///< light sensor pin
  #define CPLAY_THERMISTORPIN A9    ///< thermistor pin
  #define CPLAY_SOUNDSENSOR A4      ///< TBD I2S
  #define CPLAY_LIS3DH_CS -1        ///< LIS3DH chip select pin
  #define CPLAY_LIS3DH_INTERRUPT 27 ///< LIS3DH interrupt pin
  #define CPLAY_LIS3DH_ADDRESS 0x19 ///< LIS3DH I2C address
*/

//#include "Adafruit_Circuit_Playground/utility/Adafruit_CPlay_NeoPixel.h"
#include "Adafruit_CircuitPlayground.h"

namespace CPE {
//allocated here for convenience of setup() hacking
Adafruit_CPlay_NeoPixel strand(10, CPLAY_NEOPIXELPIN);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  strand.begin();
}

/** makes the CPE neopixel strand work like the single one in the QTPY */
class NeoPixel {
    bool wason = false;

  public:
    class Color {
      public:
        //order below allows traditional order in anonymous {,,} expressions.
        byte red;
        byte green;
        byte blue;
        operator uint32_t()const {
          //todo: GRB/RGB etc operand
          return red << 16 | green << 8 | blue;
        }
        Color &operator =(uint32_t packed) {
          blue = packed;
          green = packed >> 8;
          red = packed >> 16;
          return *this;
        }

    };

    Color whenOn {0, 255, 255};
    /** this constructor does setup operations. The libraryand has been inspected to confirm that it can be run before setup()*/

    bool operator =(bool onish) {
      digitalWrite(LED_BUILTIN, onish);
      if (changed(wason, onish)) {
        dbg("Pixel: ", wason ? "ON" : "off");
        if (wason) {
          refresh();
        } else {
          CircuitPlayground.clearPixels();
        }
      }
      return onish;
    }

    operator bool()const {
      return wason;
    }

    void refresh() {
      strand.fill(0, whenOn, 10);
      strand.show();
      dbg("Sending Color: ", HEXLY(whenOn));
    }

    void sendColor(Color packed) {
      whenOn = packed;
      refresh();
    }


    class ColorChannel {
        NeoPixel &pixel;
        char channel;//r,g,b, maybe someday white if they add one of those.
      public:
        ColorChannel(NeoPixel &pixel, char channel): pixel(pixel), channel(channel) {}

        void operator = (unsigned raw) {
          byte value = constrain(raw, 0, 255);
          switch (channel) {
            case 'r':
              pixel.whenOn.red = value;
              break;
            case 'b':
              pixel.whenOn.blue = value;
              break;
            case 'g':
              pixel.whenOn.green = value;
              break;
          }
          pixel.refresh();
        }

        operator Color() {
          return pixel.whenOn;
        }
    };
};
};
