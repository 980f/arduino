
/**
  someday the char * and allocated will be a pair and this will be a class.
*/
#include <EEPROM.h>

//copy protecting and enforcing a null terminator.
void strzcpy(char *target, unsigned allocated, const char *source) {
  if (allocated--) { //the decrement reserves space for the null terminator, post decrement to test that the value is nonzero else we don't have room for the terminator
    unsigned si = 0;
    while (*source && si < allocated) {
      target[si++] = *source++;
    }
    target[si] = 0;//guaranteed null terminator
  }
}

unsigned strztok(char *sbuf, unsigned len, char cutter) {
  for (unsigned pi = 0; pi < len; ++pi) {
    char c = *sbuf++;
    if (c == cutter) {
      sbuf[pi] = 0;
      return pi;
    }
    if (c == 0) {
      break;
    }
  }
  return ~0U;
}

//write fixed size block of eeprom, nulling all locations starting with the given null.
unsigned strzsave(char *thing, unsigned allocated, unsigned offset) {
  bool clean = false;
  for (unsigned si = 0; si < allocated; ++ si) {
    char c = clean ? 0 : thing[si];
    clean = (c == 0);
    EEPROM.write(offset++, c);   //no need to finesse with update() on an esp8266.
  }
  return offset;
}

//read from fixed size block of eeprom, ignoring data after given null.
unsigned strzload(char *thing, unsigned allocated, unsigned offset) {
  bool clean = false;
  for (unsigned si = 0; si < allocated; ++ si, ++offset) {
    char c = clean ? 0 : EEPROM.read(offset);
    clean = (c == 0);
    thing[si] = c;
  }
  return offset;
}
