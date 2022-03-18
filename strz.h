#pragma once

/**
  someday the char * and allocated will be a pair and this will be a class.
*/

//copy protecting and enforcing a null terminator.
void strzcpy(char *target, unsigned allocated, const char *source);

unsigned strztok(char *sbuf, unsigned len, char cutter = ':');

//write fixed size block of eeprom, nulling all locations starting with the given null.
unsigned strzsave(char *thing, unsigned allocated, unsigned offset);

//read from fixed size block of eeprom, ignoring data after given null.
unsigned strzload(char *thing, unsigned allocated, unsigned offset);
