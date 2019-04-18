#ifndef _WIFIBOY_LIB_H_
#define _WIFIBOY_LIB_H_
#ifdef __cplusplus
extern "C"
{
#endif

#define wbBLACK       0x0000      /*   0,   0,   0 */
#define wbNAVY        0x0F00      /*   0,   0, 128 */
#define wbDARKGREEN   0xE003      /*   0, 128,   0 */
#define wbDARKCYAN    0xEF03      /*   0, 128, 128 */
#define wbMAROON      0x0078      /* 128,   0,   0 */
#define wbPURPLE      0x0F78      /* 128,   0, 128 */
#define wbOLIVE       0xE07B      /* 128, 128,   0 */
#define wbLIGHTGREY   0x18C6      /* 192, 192, 192 */
#define wbDARKGREY    0xEF7B      /* 128, 128, 128 */
#define wbBLUE        0x1F00      /*   0,   0, 255 */
#define wbGREEN       0xE007      /*   0, 255,   0 */
#define wbCYAN        0xFF07      /*   0, 255, 255 */
#define wbRED         0x00F8      /* 255,   0,   0 */
#define wbMAGENTA     0x1FF8      /* 255,   0, 255 */
#define wbYELLOW      0xE0FF      /* 255, 255,   0 */
#define wbWHITE       0xFFFF      /* 255, 255, 255 */
#define wbORANGE      0x20FD      /* 255, 165,   0 */
#define wbGREENYELLOW 0xE5AF      /* 173, 255,  47 */
#define wbPINK        0x1FF8

/*
uint16_t _wbColor[]={
	wbBLACK, wbNAVY, wbDARKGREEN, wbDARKCYAN, wbMAROON, wbPURPLE, wbOLIVE,
	wbLIGHTGREY, wbDARKGREY, wbBLUE, wbGREEN, wbCYAN, wbRED, wbMAGENTA, wbYELLOW,
	wbWHITE, wbORANGE, wbGREENYELLOW, wbPINK 
};
*/
// wb32lcd

void lcdInit(int t);
void wb_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void wb_fillScreen(uint16_t c);
void wb_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void wb_drawPixel(uint16_t x, uint16_t y, uint16_t color);
void wb_pushColor(uint16_t color);
void wb_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void wb_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void wb_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void wb_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, int16_t w);
void wb_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color, uint16_t w);
void wb_drawCorner(int16_t x0, int16_t y0, int16_t r, uint8_t corner, uint16_t color, uint16_t w);
void wb_fillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
void wb_fillCircle2(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void wb_drawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data);
void wb_setTextColor(uint16_t c1, uint16_t c2);
int wb_drawString(const char *string, uint16_t poX, uint16_t poY, uint8_t size, uint8_t mag);
int wb_drawChar(uint16_t uniCode, uint16_t x, uint16_t y, uint8_t size, uint8_t mag);
uint16_t wb_color565(uint8_t r, uint8_t g, uint8_t b);
void wb_init(int t); // mini:0 classic:1 pro:2

void wb_setPal8(uint8_t n, uint16_t c);
void wb_blit8();
void wb_blitBuf8(uint16_t xs, uint16_t ys, uint16_t ws, int xd, int yd, uint16_t width, uint16_t height, const uint8_t *data);
void wb_rot8(uint16_t dx, uint16_t dy, double angle, double scale, uint16_t offx, uint16_t offy, uint8_t w, uint8_t h, uint8_t *sprite);
void wb_setBuf8(uint32_t i, uint8_t d);
void wb_initBuf8(void);
void wb_clearBuf8();
void wb_freeBuf8();

void wb_blit16(uint16_t *buf);
void wb_setBuf16(uint32_t i, uint16_t d);
void wb_initBuf16(void);
void wb_clearBuf16();
void wb_freeBuf16();

int wb_rand(int r);
int wb_getkey(void); 

void wb_tickerInit(uint32_t us, void code());
void wb_tickerAlarm(uint32_t us);
void wb_tickerEnable();
void wb_tickerDisable();

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // _WIFIBOY_LIB_H_
