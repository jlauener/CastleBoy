#include "TurboLib.h"

#include "assets.h"

// ---------------------------------------------------------------------------------
// CORE
// ---------------------------------------------------------------------------------

uint16_t ab::frameCount = -1;
color_t screenBuffer[(HEIGHT*WIDTH) / 8];

namespace
{
  uint8_t timePerFrame;
  uint32_t nextFrameMillis;

  uint8_t buttonState;
  uint8_t previousButtonState;
}

void ab::init(uint8_t frameRate) {
  core::boot();

  timePerFrame = 1000 / frameRate;
  nextFrameMillis = 0;

  buttonState = 0;
  previousButtonState = 0;
}

bool ab::beginUpdate() {

  uint32_t now = millis();
  if (now < nextFrameMillis)
  {
    return false;
  }

  previousButtonState = buttonState;
  buttonState = core::buttonsState();

  nextFrameMillis = now + timePerFrame;
  frameCount++;
  return true;
}

void ab::endUpdate()
{
  core::paintScreen(screenBuffer);
}

// ---------------------------------------------------------------------------------
// INPUT
// ---------------------------------------------------------------------------------

bool ab::isButtonDown(uint8_t button)
{
  return (buttonState & button) == button;
}

bool ab::wasButtonPressed(uint8_t button)
{
  return (previousButtonState & button) == 0 && isButtonDown(button);
}

bool ab::wasButtonReleased(uint8_t button)
{
  return (previousButtonState & button) == button && !isButtonDown(button);
}

// ---------------------------------------------------------------------------------
// GFX - PRIMITIVES
// ---------------------------------------------------------------------------------

// Used by drawPixel to help with left bitshifting since AVR has no
// multiple bit shift instruction.  We can bit shift from a lookup table
// in flash faster than we can calculate the bit shifts on the CPU.
const uint8_t bitshift_left[] PROGMEM = {
  _BV(0), _BV(1), _BV(2), _BV(3), _BV(4), _BV(5), _BV(6), _BV(7)
};

void ab::setPixel(int16_t x, int16_t y, color_t color)
{
#ifdef PIXEL_SAFE_MODE
  if (x < 0 || x >(WIDTH - 1) || y < 0 || y >(HEIGHT - 1))
  {
    LOG_DEBUG("pixel overflow");
    return;
  }
#endif

#ifdef SDL_TARGET
  uint8_t row = (uint8_t)y / 8;
  uint16_t row_offset = (row * WIDTH) + (uint8_t)x;
  uint8_t bit = _BV((uint8_t)y % 8);
#else
  uint16_t row_offset;
  uint8_t bit;

  // the above math can also be rewritten more simply as;
  //   row_offset = (y * WIDTH/8) & ~0b01111111 + (uint8_t)x;
  // which is what the below assembler does

  // local variable for the bitshift_left array pointer,
  // which can be declared a read-write operand
  const uint8_t* bsl = bitshift_left;

  asm volatile
    (
      "mul %[width_offset], %A[y]\n"
      "movw %[row_offset], r0\n"
      "andi %A[row_offset], 0x80\n" // row_offset &= (~0b01111111);
      "clr __zero_reg__\n"
      "add %A[row_offset], %[x]\n"
      // mask for only 0-7
      "andi %A[y], 0x07\n"
      // Z += y
      "add r30, %A[y]\n"
      "adc r31, __zero_reg__\n"
      // load correct bitshift from program RAM
      "lpm %[bit], Z\n"
      : [row_offset] "=&x" (row_offset), // upper register (ANDI)
      [bit] "=r" (bit),
      [y] "+d" (y), // upper register (ANDI), must be writable
      "+z" (bsl) // is modified to point to the proper shift array element
      : [width_offset] "r" ((uint8_t)(WIDTH / 8)),
      [x] "r" ((uint8_t)x)
      :
      );
#endif

  if (color) {
    screenBuffer[row_offset] |= bit;
  }
  else {
    screenBuffer[row_offset] &= ~bit;
  }
}

color_t ab::getPixel(uint8_t x, uint8_t y)
{
  uint8_t row = y / 8;
  uint8_t bit_position = y % 8;
  return (screenBuffer[(row*WIDTH) + x] & _BV(bit_position)) >> bit_position;
}

void swap(int16_t& a, int16_t& b)
{
  int16_t temp = a;
  a = b;
  b = temp;
}


void ab::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, color_t color)
{
  // bresenham's algorithm - thx wikpedia
  bool steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int8_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  }
  else
  {
    ystep = -1;
  }

  for (; x0 <= x1; x0++)
  {
    if (steep)
    {
      setPixel(y0, x0, color);
    }
    else
    {
      setPixel(x0, y0, color);
    }

    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void ab::drawVLine(int16_t x, int16_t y, uint8_t h, color_t color)
{
  int16_t end = y + h;
  for (int16_t a = max(0, y); a < min(end, HEIGHT); a++)
  {
    setPixel(x, a, color);
  }
}

void ab::drawHLine(int16_t x, int16_t y, uint8_t w, color_t color)
{
  int16_t xEnd; // last x point + 1

  // Do y bounds checks
  if (y < 0 || y >= HEIGHT)
    return;

  xEnd = x + w;

  // Check if the entire line is not on the display
  if (xEnd <= 0 || x >= WIDTH)
    return;

  // Don't start before the left edge
  if (x < 0)
    x = 0;

  // Don't end past the right edge
  if (xEnd > WIDTH)
    xEnd = WIDTH;

  // calculate actual width (even if unchanged)
  w = xEnd - x;

  // buffer pointer plus row offset + x offset
  register uint8_t *pBuf = screenBuffer + ((y / 8) * WIDTH) + x;

  // pixel mask
  register uint8_t mask = 1 << (y & 7);

  switch (color)
  {
  case WHITE:
    while (w--)
    {
      *pBuf++ |= mask;
    }
    break;

  case BLACK:
    mask = ~mask;
    while (w--)
    {
      *pBuf++ &= mask;
    }
    break;
  }
}

void ab::drawRect(int16_t x, int16_t y, uint8_t w, uint8_t h, color_t color)
{
  drawHLine(x, y, w, color);
  drawHLine(x, y + h - 1, w, color);
  drawVLine(x, y, h, color);
  drawVLine(x + w - 1, y, h, color);
}

void ab::fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, color_t color)
{
  // stupidest version - update in subclasses if desired!
  for (int16_t i = x; i < x + w; i++)
  {
    drawVLine(i, y, h, color);
  }
}

void ab::fillScreen(color_t color)
{

#ifdef SDL_TARGET
  if (color != BLACK)
  {
    color = 0xFF; // all pixels on
  }
  for (int16_t i = 0; i < WIDTH * HEIGHT / 8; i++)
  {
    screenBuffer[i] = color;
  }
#else
  // This asm version is hard coded for 1024 bytes. It doesn't use the defined
  // WIDTH and HEIGHT values. It will have to be modified for a different
  // screen buffer size.
  // It also assumes color value for BLACK is 0.

  // local variable for screen buffer pointer,
  // which can be declared a read-write operand
  uint8_t* bPtr = sBuffer;

  asm volatile
    (
      // if value is zero, skip assigning to 0xff
      "cpse %[color], __zero_reg__\n"
      "ldi %[color], 0xFF\n"
      // counter = 0
      "clr __tmp_reg__\n"
      "loopto:\n"
      // (4x) push zero into screen buffer,
      "st Z+, %[color]\n"
      "st Z+, %[color]\n"
      // increase counter
      "inc __tmp_reg__\n"
      // repeat for 256 loops
      // (until counter rolls over back to 0)
      "brne loopto\n"
      : [color] "+d" (color),
      "+z" (bPtr)
      :
      :
      );
#endif
}

// ---------------------------------------------------------------------------------
// GFX - SPRITES
// ---------------------------------------------------------------------------------


#define SPRITE_OVERWRITE 0
#define SPRITE_SELF_MASKED 1
#define SPRITE_PLUS_MASK 2

void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame, uint8_t drawMode)
{
  unsigned int frame_offset;

  uint8_t w = pgm_read_byte(bitmap);
  uint8_t h = pgm_read_byte(++bitmap);

  // no need to draw at all of we're offscreen
  if (x + w <= 0 || x > WIDTH - 1 || y + h <= 0 || y > HEIGHT - 1)
    return;

  bitmap++;
  if (frame > 0) {
    frame_offset = (w * (h / 8 + (h % 8 == 0 ? 0 : 1)));
    // sprite plus mask uses twice as much space for each frame
    if (drawMode == SPRITE_PLUS_MASK) {
      frame_offset *= 2;
    }
    bitmap += frame * frame_offset;
  }

  // xOffset technically doesn't need to be 16 bit but the math operations
  // are measurably faster if it is
  uint16_t xOffset, ofs;
  int8_t yOffset = abs(y) % 8;
  int8_t sRow = y / 8;
  uint8_t renderHeight, start_h, renderWidth;

  if (y < 0 && yOffset > 0) {
    sRow--;
    yOffset = 8 - yOffset;
  }

  // if the left side of the render is offscreen skip those loops
  if (x < 0) {
    xOffset = abs(x);
  }
  else {
    xOffset = 0;
  }

  // if the right side of the render is offscreen skip those loops
  if (x + w > WIDTH - 1) {
    renderWidth = ((WIDTH - x) - xOffset);
  }
  else {
    renderWidth = (w - xOffset);
  }

  // if the top side of the render is offscreen skip those loops
  if (sRow < -1) {
    start_h = abs(sRow) - 1;
  }
  else {
    start_h = 0;
  }

  renderHeight = h / 8 + (h % 8 > 0 ? 1 : 0); // divide, then round up

  // if (sRow + loop_h - 1 > (HEIGHT/8)-1)
  if (sRow + renderHeight > (HEIGHT / 8)) {
    renderHeight = (HEIGHT / 8) - sRow;
  }

  // prepare variables for loops later so we can compare with 0
  // instead of comparing two variables
  renderHeight -= start_h;

  sRow += start_h;
  ofs = (sRow * WIDTH) + x + xOffset;
  uint8_t *bofs = (uint8_t *)bitmap + (start_h * w) + xOffset;
  uint8_t data;

  uint8_t mul_amt = 1 << yOffset;
  uint16_t mask_data;
  uint16_t bitmapData;

  switch (drawMode) {
  case SPRITE_OVERWRITE:
    // we only want to mask the 8 bits of our own sprite, so we can
    // calculate the mask before the start of the loop
    mask_data = ~(0xFF * mul_amt);
    // really if yOffset = 0 you have a faster case here that could be
    // optimized
    for (uint8_t a = 0; a < renderHeight; a++) {
      for (uint8_t iCol = 0; iCol < renderWidth; iCol++) {
        if (sRow >= 0) {
          bitmapData = pgm_read_byte(bofs) * mul_amt;

          data = screenBuffer[ofs];
          data &= (uint8_t)(mask_data);
          data |= (uint8_t)(bitmapData);
          screenBuffer[ofs] = data;

          if (yOffset != 0 && sRow < 7) {
            data = screenBuffer[ofs + WIDTH];
            data &= (*((uint8_t *)(&mask_data) + 1));
            data |= (*((uint8_t *)(&bitmapData) + 1));
            screenBuffer[ofs + WIDTH] = data;
          }
        }
        ofs++;
        bofs++;
      }
      sRow++;
      bofs += w - renderWidth;
      ofs += WIDTH - renderWidth;
    }
    break;

  case SPRITE_SELF_MASKED:
    for (uint8_t iy = 0; iy < renderHeight; iy++) {
      for (uint8_t ix = 0; ix < renderWidth; ix++) {
        bitmapData = pgm_read_byte(bofs) * mul_amt;
        if (sRow >= 0) {
          screenBuffer[ofs] |= (uint8_t)(bitmapData);
        }
        if (yOffset != 0 && sRow < 7) {
          screenBuffer[ofs + WIDTH] |= (*((unsigned char *)(&bitmapData) + 1));
        }
        ofs++;
        bofs++;
      }
      sRow++;
      bofs += w - renderWidth;
      ofs += WIDTH - renderWidth;
    }
    break;

  case SPRITE_PLUS_MASK:
    // *2 because we use double the bits (mask + bitmap)
    bofs = (uint8_t *)(bitmap + ((start_h * w) + xOffset) * 2);

#ifdef SDL_TARGET
    for (uint8_t iy = 0; iy < renderHeight; iy++) {
      for (uint8_t ix = 0; ix < renderWidth; ix++) {
        if (sRow >= 0) {
          bitmapData = pgm_read_byte(bofs) * mul_amt;
          mask_data = ~(pgm_read_byte(bofs + 1) * mul_amt);

          data = screenBuffer[ofs];
          data &= (uint8_t)(mask_data);
          data |= (uint8_t)(bitmapData);
          screenBuffer[ofs] = data;

          if (yOffset != 0 && sRow < 7) {
            data = screenBuffer[ofs + WIDTH];
            data &= (*((uint8_t *)(&mask_data) + 1));
            data |= (*((uint8_t *)(&bitmapData) + 1));
            screenBuffer[ofs + WIDTH] = data;
          }
        }
        ofs++;
        bofs += 2;
      }
      sRow++;
      bofs += (w - renderWidth) * 2;
      ofs += WIDTH - renderWidth;
    }
    break;
#else
    uint8_t xi = rendered_width; // counter for x loop below

    asm volatile(
      "push r28\n" // save Y
      "push r29\n"
      "movw r28, %[buffer_ofs]\n" // Y = buffer_ofs_2
      "adiw r28, 63\n" // buffer_ofs_2 = buffer_ofs + 128
      "adiw r28, 63\n"
      "adiw r28, 2\n"
      "loop_y:\n"
      "loop_x:\n"
      // load bitmap and mask data
      "lpm %A[bitmap_data], Z+\n"
      "lpm %A[mask_data], Z+\n"

      // shift mask and buffer data
      "tst %[yOffset]\n"
      "breq skip_shifting\n"
      "mul %A[bitmap_data], %[mul_amt]\n"
      "movw %[bitmap_data], r0\n"
      "mul %A[mask_data], %[mul_amt]\n"
      "movw %[mask_data], r0\n"

      // SECOND PAGE
      // if yOffset != 0 && sRow < 7
      "cpi %[sRow], 7\n"
      "brge end_second_page\n"
      // then
      "ld %[data], Y\n"
      "com %B[mask_data]\n" // invert high byte of mask
      "and %[data], %B[mask_data]\n"
      "or %[data], %B[bitmap_data]\n"
      // update buffer, increment
      "st Y+, %[data]\n"

      "end_second_page:\n"
      "skip_shifting:\n"

      // FIRST PAGE
      // if sRow >= 0
      "tst %[sRow]\n"
      "brmi skip_first_page\n"
      "ld %[data], %a[buffer_ofs]\n"
      // then
      "com %A[mask_data]\n"
      "and %[data], %A[mask_data]\n"
      "or %[data], %A[bitmap_data]\n"
      // update buffer, increment
      "st %a[buffer_ofs]+, %[data]\n"
      "jmp end_first_page\n"

      "skip_first_page:\n"
      // since no ST Z+ when skipped we need to do this manually
      "adiw %[buffer_ofs], 1\n"

      "end_first_page:\n"

      // "x_loop_next:\n"
      "dec %[xi]\n"
      "brne loop_x\n"

      // increment y
      "next_loop_y:\n"
      "dec %[yi]\n"
      "breq finished\n"
      "mov %[xi], %[x_count]\n" // reset x counter
      // sRow++;
      "inc %[sRow]\n"
      "clr __zero_reg__\n"
      // sprite_ofs += (w - rendered_width) * 2;
      "add %A[sprite_ofs], %A[sprite_ofs_jump]\n"
      "adc %B[sprite_ofs], __zero_reg__\n"
      // buffer_ofs += WIDTH - rendered_width;
      "add %A[buffer_ofs], %A[buffer_ofs_jump]\n"
      "adc %B[buffer_ofs], __zero_reg__\n"
      // buffer_ofs_page_2 += WIDTH - rendered_width;
      "add r28, %A[buffer_ofs_jump]\n"
      "adc r29, __zero_reg__\n"

      "rjmp loop_y\n"
      "finished:\n"
      // put the Y register back in place
      "pop r29\n"
      "pop r28\n"
      "clr __zero_reg__\n" // just in case
      : [xi] "+&a" (xi),
      [yi] "+&a" (loop_h),
      [sRow] "+&a" (sRow), // CPI requires an upper register (r16-r23)
      [data] "=&l" (data),
      [mask_data] "=&l" (mask_data),
      [bitmap_data] "=&l" (bitmap_data)
      :
      [screen_width] "M" (WIDTH),
      [x_count] "l" (rendered_width), // lower register
      [sprite_ofs] "z" (bofs),
      [buffer_ofs] "x" (screenBuffer + ofs),
      [buffer_ofs_jump] "a" (WIDTH - rendered_width), // upper reg (r16-r23)
      [sprite_ofs_jump] "a" ((w - rendered_width) * 2), // upper reg (r16-r23)

      // [sprite_ofs_jump] "r" (0),
      [yOffset] "l" (yOffset), // lower register
      [mul_amt] "l" (mul_amt) // lower register
      // NOTE: We also clobber r28 and r29 (y) but sometimes the compiler
      // won't allow us, so in order to make this work we don't tell it
      // that we clobber them. Instead, we push/pop to preserve them.
      // Then we need to guarantee that the the compiler doesn't put one of
      // our own variables into r28/r29.
      // We do that by specifying all the inputs and outputs use either
      // lower registers (l) or simple (r16-r23) upper registers (a).
      : // pushes/clobbers/pops r28 and r29 (y)
    );
#endif
    break;
  }
}

void ab::drawOverwrite(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame)
{
  drawBitmap(x, y, bitmap, frame, SPRITE_OVERWRITE);
}

void ab::drawSelfMasked(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame)
{
  drawBitmap(x, y, bitmap, frame, SPRITE_SELF_MASKED);
}

void ab::drawPlusMask(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame)
{
  drawBitmap(x, y, bitmap, frame, SPRITE_PLUS_MASK);
}

void ab::drawNumber(uint8_t x, uint8_t y, uint16_t value, uint8_t align)
{
  char buf[10];
#ifdef SDL_TARGET
  _ltoa(value, buf, 10);
#else
  ltoa(value, buf, 10);
#endif
  uint8_t strLength = (uint8_t)strlen(buf);
  int8_t offset;
  switch (align)
  {
  case ALIGN_LEFT:
    offset = 0;
    break;
  case ALIGN_CENTER:
    offset = -(strLength * 2);
    break;
  case ALIGN_RIGHT:
    offset = -(strLength * 4);
    break;
  }

  for (uint8_t i = 0; i < strLength; i++)
  {
    uint8_t digit = (uint8_t)buf[i];
    digit -= 48;
    if (digit > 9) digit = 0;
    drawOverwrite(x + offset + 4 * i, y, font, digit);
  }
}

// -----------------------------------------------------------------------
// SOUND
// -----------------------------------------------------------------------
bool soundEnabled = true;

void ab::toggleSoundEnabled()
{
  soundEnabled = !soundEnabled;
  // TODO
  //if (ab.audio.enabled())
  //{
  //  ab.audio.off();
  //}
  //else
  //{
  //  ab.audio.on();
  //}
  //ab.audio.saveOnOff();
}

bool ab::isSoundEnabled()
{
  return soundEnabled;
}

void ab::tone(uint16_t freq, uint16_t dur)
{
  // TODO
}

void ab::tone(uint16_t freq1, uint16_t dur1, uint16_t freq2, uint16_t dur2)
{
  // TODO
}

void ab::tone(uint16_t freq1, uint16_t dur1, uint16_t freq2, uint16_t dur2, uint16_t freq3, uint16_t dur3)
{
  // TODO
}

// -----------------------------------------------------------------------
// UTILS
// -----------------------------------------------------------------------

bool ab::everyXFrames(uint8_t frame)
{
  return frameCount % frame == 0;
}

void ab::toggle(uint8_t & flags, uint8_t mask)
{
  if (flags & mask)
  {
    flags &= ~mask;
  }
  else
  {
    flags |= mask;
  }
}

bool ab::collide(int16_t x1, int8_t y1, uint8_t w1, uint8_t h1, int16_t x2, int8_t y2, uint8_t w2, uint8_t h2)
{
  return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
}


