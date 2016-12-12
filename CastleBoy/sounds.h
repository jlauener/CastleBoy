#ifndef SOUNDS_H
#define SOUNDS_H

const uint16_t sfx_intro[] PROGMEM = {
  NOTE_CS6, 30,
  NOTE_CS5, 40,
  TONES_END
};

const uint16_t sfx_select[] PROGMEM = {
  NOTE_CS6, 30,
  TONES_END
};

#define B_X 70
#define B_XX 140
#define B_XXXX 280

const uint16_t beat1[] PROGMEM = {
  // 1:1 ----------------------------------
  NOTE_C5,  B_X,
  NOTE_C4,  B_X,
  0,        B_XX,
  NOTE_C2,  B_XXXX,

  // 1:2 -------------
  0,        B_XX,
  NOTE_C2,  B_XXXX,
  0,        B_XX,

  // 1:3 -------------
  NOTE_C2,  B_XXXX,
  0,        B_XX,
  0,        B_XX,

  // 1:4 -------------
  0,        B_XX,
  0,        B_XX,
  NOTE_C4,  B_XXXX,

  // 2:1 ----------------------------------
  NOTE_C4,  B_X,
  NOTE_C4,        B_X,
  NOTE_C2,  B_XXXX,
  0,        B_XX,

  // 2:2 -------------
  NOTE_C3,  B_XX,
  0,        B_XX,
  NOTE_C2,  B_XXXX,

  // 2:3 -------------
  0,        B_XX,
  NOTE_C2,  B_XXXX,
  0,        B_XX,

  // 3:4 -------------
  0,        B_XX,
  0,        B_XX,
  NOTE_C2,  B_XXXX,

  TONES_REPEAT
};

//const uint16_t music1[] = {
//  // 1:1 ----------------------------------
//  NOTE_C5,  B_X,
//  NOTE_C4,  B_X,
//  0,        B_XX,
//  NOTE_C2,  B_XXXX,
//
//  // 1:2 -------------
//  0,        B_XX,
//  NOTE_C2,  B_XXXX,
//  0,        B_XX,
//
//  // 1:3 -------------
//  NOTE_C2,  B_XXXX,
//  0,        B_XX,
//  0,        B_XX,
//
//  // 1:4 -------------
//  0,        B_XX,
//  0,        B_XX,
//  NOTE_C4,  B_XXXX,
//
//  // 2:1 ----------------------------------
//  NOTE_C4,  B_X,
//  NOTE_C4,        B_X,
//  NOTE_C2,  B_XXXX,
//  0,        B_XX,
//
//  // 2:2 -------------
//  NOTE_C3,  B_XX,
//  0,        B_XX,
//  NOTE_C2,  B_XXXX,
//
//  // 2:3 -------------
//  0,        B_XX,
//  NOTE_C2,  B_XXXX,
//  0,        B_XX,
//
//  // 3:4 -------------
//  0,        B_XX,
//  0,        B_XX,
//  NOTE_C2,  B_XXXX,
//
//  TONES_REPEAT
//};

const uint16_t beat2[] PROGMEM = {
  // ----------------------------------
  NOTE_C4,  B_X,
  NOTE_C4,  B_X,
  NOTE_C2,  B_XXXX,
  0,        B_XX,

  // -------------
  NOTE_C3,  B_XX,
  0,        B_XX,
  NOTE_C2,  B_XXXX,

  // -------------
  0,        B_XX,
  NOTE_C2,  B_XXXX,
  0,        B_XX,

  // -------------
  NOTE_C3,  B_XX,
  0,        B_XX,
  NOTE_C2,  B_XXXX,

  // ----------------------------------
  NOTE_C4,  B_X,
  NOTE_C4,  B_X,
  NOTE_C2,  B_XXXX,
  0,        B_XX,

  // -------------
  NOTE_C3,  B_XX,
  0,        B_XX,
  NOTE_C2,  B_XXXX,

  // -------------
  0,        B_XX,
  NOTE_C2,  B_XXXX,
  0,        B_XX,

  // -------------
  NOTE_C3,  B_XX,
  0,        B_XX,
  0,  B_XXXX,

  TONES_REPEAT
};


#endif
