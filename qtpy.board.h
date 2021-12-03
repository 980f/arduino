#include "Adafruit_NeoPixel.h"

namespace QTPY {

void setup() {
  pinMode(12, OUTPUT);
}

class NeoPixel {

    Adafruit_NeoPixel strand;
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
    NeoPixel() {
      strand = Adafruit_NeoPixel(1, 11);
      strand.begin();
    }

    bool operator =(bool onish) {
      digitalWrite(12, onish);
      if (changed(wason, onish)) {
        dbg("Pixel: ", wason ? "ON" : "off");
        if (wason) {
          refresh();
        }
      }
      return onish;
    }

    operator bool()const {
      return wason;
    }

    void refresh() {
      strand.setPixelColor(0, whenOn);
      strand.show();
      dbg("Sending Color: ", HEXLY(whenOn));
    }

    void sendColor(Color packed) {
      whenOn = packed;
      refresh();
      //      dbg("Setting Color: ", HEXLY(packed));
    }


    class ColorChannel {
        NeoPixel &pixel;
        char channel;//r,g,b, maybe someday white if they add one of those.
      public:
        ColorChannel(NeoPixel &pixel, char channel): pixel(pixel), channel(channel) {}

        void operator = (byte value) {
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
