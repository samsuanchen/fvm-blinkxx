
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

#include "wbfonts.h"
//#include "math.h"

#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   15
#define PIN_NUM_DC   4
#define PIN_NUM_BCKL 27

#define wbBLACK       0x0000      /*   0,   0,   0 */
#define wbNAVY        0x0F00      /*   0,   0, 128 */
#define wbDARKGREEN   0xE003      /*   0, 128,   0 */
#define wbDARKCYAN    0xEF03      /*   0, 128, 128 */
#define wbMAROON      0x0078      /* 128,   0,   0 */
#define wbPURPLE      0x0F78      /* 128,   0, 128 */
#define wbOLIVE       0xE07B      /* 128, 128,   0 */
#define wbLIGHTGREY   0x18C6      /* 192, 192, 192 */
#define wbDARKGREY    0xEF7B      /* 128, 128, 128 */
#define wbBLUE        0x1F00     /*   0,   0, 255 */
#define wbGREEN       0xE007      /*   0, 255,   0 */
#define wbCYAN        0xFF07      /*   0, 255, 255 */
#define wbRED         0x00F8      /* 255,   0,   0 */
#define wbMAGENTA     0x1FF8      /* 255,   0, 255 */
#define wbYELLOW      0xE0FF      /* 255, 255,   0 */
#define wbWHITE       0xFFFF      /* 255, 255, 255 */
#define wbORANGE      0x20FD      /* 255, 165,   0 */
#define wbGREENYELLOW 0xE5AF      /* 173, 255,  47 */
#define wbPINK        0x1FF8

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes;
} lcd_init_cmd_t;

spi_device_handle_t _lcd_spi;
int _board_type = 0; // WiFiBoy Mini=0, Classic=1, Pro=2
int _shx, _shy;
int _w, _h, _lines;

DRAM_ATTR static const lcd_init_cmd_t lcd_init_cmds_mini[]={
    /* Soft Reset */
    {0x1, {0}, 0x80},
    /* Sleep out */
    {0x11, {0}, 0x80},
    {0xB1, {0x05, 0x3c, 0x3c}, 3},
    {0xB2, {0x05, 0x3c, 0x3c}, 3},
    {0xB3, {0x05, 0x3c, 0x3c, 0x05, 0x3c, 0x3c}, 6},
    {0xB4, {0x03}, 1},
    {0xC0, {0x28, 0x08, 0x04}, 3},
    {0xC1, {0xc0}, 1},
    {0xC2, {0x0d, 0x00}, 2},
    {0xC3, {0x8d, 0x2a}, 2},
    {0xC4, {0x8d, 0xee}, 2},
    {0xC5, {0x1a}, 1},
    {0x36, {0xc8}, 1},
    {0x3a, {0x05}, 1},
    /* Positive Voltage Gamma Control */
    {0xE0, {0x04,0x22,0x07,0x0a,0x2e,0x30,0x25,0x2a,0x28,0x26,0x2e,0x3a,0x00,0x01,0x03,0x13}, 16},
    /* Negative Voltage Gamma Control */
    {0xE1, {0x04,0x16,0x06,0x0d,0x2d,0x26,0x23,0x27,0x27,0x25,0x2d,0x3b,0x00,0x01,0x04,0x13}, 16},
    /* Display On */
    {0x29, {0}, 0x80},
    {0, {0}, 0xff}
};

DRAM_ATTR static const lcd_init_cmd_t lcd_init_cmds_classic[]={
    /* Soft Reset */
    {0x1, {0}, 0x80},
    /* Sleep out */
    {0x11, {0}, 0x80},
    {0xB1, {0x05, 0x3c, 0x3c}, 3},
    {0xB2, {0x05, 0x3c, 0x3c}, 3},
    {0xB3, {0x05, 0x3c, 0x3c, 0x05, 0x3c, 0x3c}, 6},
    {0xB4, {0x03}, 1},
    {0xC0, {0x28, 0x08, 0x04}, 3},
    {0xC1, {0xc0}, 1},
    {0xC2, {0x0d, 0x00}, 2},
    {0xC3, {0x8d, 0x2a}, 2},
    {0xC4, {0x8d, 0xee}, 2},
    {0xC5, {0x1a}, 1},
    {0x36, {0xc0}, 1},
    {0x3a, {0x05}, 1},
    /* Positive Voltage Gamma Control */
    {0xE0, {0x04,0x22,0x07,0x0a,0x2e,0x30,0x25,0x2a,0x28,0x26,0x2e,0x3a,0x00,0x01,0x03,0x13}, 16},
    /* Negative Voltage Gamma Control */
    {0xE1, {0x04,0x16,0x06,0x0d,0x2d,0x26,0x23,0x27,0x27,0x25,0x2d,0x3b,0x00,0x01,0x04,0x13}, 16},
    /* Display On */
    {0x29, {0}, 0x80},
    {0, {0}, 0xff}
};

DRAM_ATTR static const lcd_init_cmd_t lcd_init_cmds_pro[]={
    {0xCF, {0x00, 0x83, 0X30}, 3},
    {0xED, {0x64, 0x03, 0X12, 0X81}, 4},
    {0xE8, {0x85, 0x01, 0x79}, 3},
    {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    {0xF7, {0x20}, 1},
    {0xEA, {0x00, 0x00}, 2},
    {0xC0, {0x26}, 1},
    {0xC1, {0x11}, 1},
    {0xC5, {0x35, 0x3E}, 2},
    {0xC7, {0xBE}, 1},
    {0x36, {0x48}, 1},
    {0x3A, {0x55}, 1},
    {0xB1, {0x00, 0x1B}, 2},
    {0xF2, {0x08}, 1},
    {0x26, {0x01}, 1},
    {0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
    {0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
    {0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
    {0x2B, {0x00, 0x00, 0x01, 0x3f}, 4}, 
    {0x2C, {0}, 0},
    {0xB7, {0x07}, 1},
    {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
    {0x11, {0}, 0x80},
    {0x29, {0}, 0x80},
    {0, {0}, 0xff},
};

void lcd_cmd(const uint8_t cmd) 
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=8;
    t.tx_buffer=&cmd;
    t.user=(void*)0;
    spi_device_transmit(_lcd_spi, &t);
}

void lcd_data(const uint8_t *data, int len) 
{
    spi_transaction_t t;
    if (len==0) return;
    memset(&t, 0, sizeof(t));
    t.length=len*8;
    t.tx_buffer=data;
    t.user=(void*)1;
    spi_device_transmit(_lcd_spi, &t);
}

void lcd_spi_pre_transfer_callback(spi_transaction_t *t) 
{
    int dc=(int)t->user;
    gpio_set_level((gpio_num_t)PIN_NUM_DC, dc);
}

void lcd_init(int btype) 
{
    int cmd=0;

    if (btype == 0) {
        _shx = 2; _shy = 3;
        _w = 128; _h = 128;
        _lines = 16;
    } else if (btype == 1) {
        _shx = 2; _shy = 1;
        _w = 128; _h = 160;
        _lines = 20;
    } else if (btype == 2) {
        _shx = 0; _shy = 0;
        _w = 240; _h = 320;
        _lines = 75;
    }

    gpio_set_direction((gpio_num_t)PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)2, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)2, 1);

    _board_type = btype;

    lcd_init_cmd_t *lcd_init_cmds;

    switch(_board_type) {
        case 0: lcd_init_cmds = lcd_init_cmds_mini; break;
        case 1: lcd_init_cmds = lcd_init_cmds_classic; break;
        case 2: lcd_init_cmds = lcd_init_cmds_pro; break;
    }
        
    vTaskDelay(100 / portTICK_RATE_MS);
    while (lcd_init_cmds[cmd].databytes!=0xff) {
        lcd_cmd(lcd_init_cmds[cmd].cmd);
        lcd_data(lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes&0x1F);
        if (lcd_init_cmds[cmd].databytes&0x80) vTaskDelay(100 / portTICK_RATE_MS);
        cmd++;
    }
    //gpio_set_level(PIN_NUM_BCKL, 1);
}

static void wait_trans(int n) 
{
    spi_transaction_t *rtrans;
    for (int i=0; i<n; i++) spi_device_get_trans_result(_lcd_spi, &rtrans, portMAX_DELAY);
}

static spi_transaction_t trans[6];

void wb_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) 
{
    memset(&trans[0], 0, 5*sizeof(spi_transaction_t));
    for(int x=0; x<5; x++) {
        if ((x&1)==0) {
          trans[x].length=8;
          trans[x].user=(void*)0;
        } else {
          trans[x].length=8*4;
          trans[x].user=(void*)1;
        }
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0]=0x2A;
    trans[1].tx_data[0]=0;
    trans[1].tx_data[1]=x0+_shx;
    trans[1].tx_data[2]=0;
    trans[1].tx_data[3]=(x1&0xff)+_shx;
    trans[2].tx_data[0]=0x2B;
    trans[3].tx_data[0]=(y0+_shy)>>8;
    trans[3].tx_data[1]=(y0+_shy)&0xff;
    trans[3].tx_data[2]=(y1+_shy)>>8;
    trans[3].tx_data[3]=(y1+_shy)&0xff;
    trans[4].tx_data[0]=0x2C;
    for(int x=0; x<5; x++) spi_device_queue_trans(_lcd_spi, &trans[x], portMAX_DELAY);
    wait_trans(5);
}

void wb_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
    uint16_t line[256];
    int i, j, s;
    if((x >= _w) || (y >= _h)) return;
    if((x+w-1) >= _w)  w = _w-x;
    if((y+h-1) >= _h) h = _h-y;
    wb_setAddrWindow(x, y, x+w-1, y+h-1);
    if (w*h < 64) {
        line[0]=color;
        for(i=0; i<w*h; i++) lcd_data((uint8_t *)line, 2);
    } else {
        if ((w*h) > 256) {
            s = 256;
            for(i=0; i<s; i++) line[i]=color;
            i=w*h;
            while(i>s){ lcd_data((uint8_t *)line, s*2); i-=s; }
            for(j=0; j<i; j++) lcd_data((uint8_t *)line, 2);
        } else {
            s = w*h;
            for(i=0; i<s; i++) line[i]=color;
            lcd_data((uint8_t *)line, s*2);
        }
    }
}

void wb_fillScreen(uint16_t c) 
{ 
    uint16_t line[1024], x, y;
    for (x=0; x<6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x&1)==0) {
            trans[x].length=8;
            trans[x].user=(void*)0;
        } else {
            trans[x].length=8*4;
            trans[x].user=(void*)1;
        }
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0]=0x2A;
    trans[1].tx_data[0]=0;
    trans[1].tx_data[1]=_shx;
    trans[1].tx_data[2]=(_w-1+_shx)>>8;
    trans[1].tx_data[3]=(_w-1+_shx)&0xff;
    trans[2].tx_data[0]=0x2B;
    trans[3].tx_data[0]=0;
    trans[3].tx_data[1]=_shy;
    trans[3].tx_data[2]=(_h-1+_shy)>>8;
    trans[3].tx_data[3]=(_h-1+_shy)&0xff;
    trans[4].tx_data[0]=0x2C;
    for(x=0; x<5; x++) spi_device_queue_trans(_lcd_spi, &trans[x], portMAX_DELAY);
    wait_trans(5);
    for (x=0; x<1024; x++) line[x] = c;
    for(y=0; y<_lines; y++) lcd_data((uint8_t *)line, 2048);
}


void wb_drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    memset(&trans[0], 0, 6*sizeof(spi_transaction_t));
    for(int x=0; x<6; x++) {
        if ((x&1)==0) {
          trans[x].length=8;
          trans[x].user=(void*)0;
        } else {
          trans[x].length=8*4;
          trans[x].user=(void*)1;
        }
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0]=0x2A;          
    trans[1].tx_data[0]=0;
    trans[1].tx_data[1]=x+_shx;
    trans[1].tx_data[2]=0;
    trans[1].tx_data[3]=x+_shx;
    trans[2].tx_data[0]=0x2B;
    trans[3].tx_data[0]=((y&0xff)+_shy)>>8;
    trans[3].tx_data[1]=((y&0xff)+_shy)&0xff;
    trans[3].tx_data[2]=((y&0xff)+2+_shy)>>8;
    trans[3].tx_data[3]=((y&0xff)+2+_shy)&0xFF;
    trans[4].tx_data[0]=0x2C;
    trans[5].tx_data[0]=color&0xff;
    trans[5].tx_data[1]=color>>8;
    trans[5].length=16;
    for(int x=0; x<6; x++) spi_device_queue_trans(_lcd_spi, &trans[x], portMAX_DELAY);
    wait_trans(6);
}

void wb_pushColor(uint16_t color) 
{
    uint8_t data[2];
    data[1]=color>>8;
    data[0]=color&0xff;
    lcd_data(data,2);
}

void wb_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
{
    uint16_t data[320]; //320?
    if((x >= _w) || (y >= _h)) return;
    if((y+h-1) >= _h) h = _h-y;
    wb_setAddrWindow(x, y, x, y+h-1);
    for(int i=0; i<h; i++) data[i]=color;
    lcd_data((uint8_t *)data,h*2);
}

void wb_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) 
{
    uint16_t data[240]; //240?
    if((x >= _w) || (y >= _h)) return;
    if((x+w-1) >= _w)  w = _w-x;
    wb_setAddrWindow(x, y, x+w-1, y);
    for(int i=0; i<w; i++) data[i]=color;
    lcd_data((uint8_t *)data,w*2);
}

#define abs(x) ((x)<0 ? -(x) : (x))
#define swap(a, b) { int16_t t = a; a = b; b = t; }

void wb_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, int16_t w) 
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) { swap(x0, y0); swap(x1, y1); }
  if (x0 > x1) { swap(x0, x1); swap(y0, y1); }
  int16_t dx, dy;
  dx = x1 - x0; dy = abs(y1 - y0);
  int16_t err = dx / 2;
  int16_t ystep;
  if (y0 < y1) ystep = 1; else ystep = -1;
  for (; x0<=x1; x0++) {
    if (steep) wb_fillRect(y0, x0, w, w, color); else wb_fillRect(x0, y0, w, w, color);
    //if (steep) drawPixel(y0, x0, color); else drawPixel(x0, y0, color);
    err -= dy;
    if (err < 0) { y0 += ystep; err += dx; }
  }
}

void wb_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color, uint16_t w) 
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0, y = r;
    wb_fillRect(x0, y0+r, w, w, color);
    wb_fillRect(x0  , y0-r, w, w, color);
    wb_fillRect(x0+r, y0  , w, w, color);
    wb_fillRect(x0-r, y0  , w, w, color);
    while (x<y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        wb_fillRect(x0 + x, y0 + y, w, w, color); wb_fillRect(x0 + x, y0 - y, w, w, color);
        wb_fillRect(x0 + y, y0 + x, w, w, color); wb_fillRect(x0 + y, y0 - x, w, w, color);
        wb_fillRect(x0 - y, y0 - x, w, w, color); wb_fillRect(x0 - x, y0 + y, w, w, color);
        wb_fillRect(x0 - x, y0 - y, w, w, color); wb_fillRect(x0 - y, y0 + x, w, w, color);
    }
}

void wb_drawCorner(int16_t x0, int16_t y0, int16_t r, uint8_t corner, uint16_t color, uint16_t w) 
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0, y = r;
    while (x<y) {
        if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
        x++; ddF_x += 2; f += ddF_x;
        if (corner & 0x04) { 
            wb_fillRect(x0 + x, y0 + y, w, w, color); 
            wb_fillRect(x0 + y, y0 + x, w, w, color); 
        }
        if (corner & 0x02) {
            wb_fillRect(x0 + x, y0 - y, w, w, color); 
            wb_fillRect(x0 + y, y0 - x, w, w, color);
        }
        if (corner & 0x08) {
            wb_fillRect(x0 - y, y0 + x, w, w, color); 
            wb_fillRect(x0 - x, y0 + y, w, w, color);
        }
        if (corner & 0x01) {
            wb_fillRect(x0 - y, y0 - x, w, w, color); 
            wb_fillRect(x0 - x, y0 - y, w, w, color);
        }
    }
}

void wb_fillCircle2(int16_t x0, int16_t y0, int16_t r, uint16_t color) 
{
    wb_drawFastVLine(x0, y0-r, 2*r+1, color);
    wb_fillCircle(x0, y0, r, 3, 0, color);
}

void wb_fillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) 
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;
    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1) {
            wb_drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
            wb_drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
        }
        if (cornername & 0x2) {
            wb_drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
            wb_drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
        }
    }
}

void wb_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
    wb_drawFastHLine(x, y, w, color);
    wb_drawFastHLine(x, y+h-1, w, color);
    wb_drawFastVLine(x, y, h, color);
    wb_drawFastVLine(x+w-1, y, h, color);
}

void wb_drawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data)
{
    int c, n;
    wb_setAddrWindow(x, y, x+w-1, y+h-1);
    c = w*h;
    if (c>64) {
        n=0;
        while (c > 64) {
            lcd_data((uint8_t *)(data+n),128);
            n+=64;
            c-=64;
        }
        lcd_data((uint8_t *)(data+n), c*2);
    } else {
        lcd_data((uint8_t *)(data), c*2);
    }
}

uint16_t textcolor, textbgcolor, mag=1;

int wb_drawChar(uint16_t uniCode, uint16_t x, uint16_t y, uint8_t size, uint8_t mag)
{
    if (size>0 && size<=5) uniCode -= 32;
    else return(0);

    uint16_t width = 0;
    uint16_t height = 0;
    const uint8_t *flash_address = 0;
    int8_t gap = 0;

    if (size == 1) {
        flash_address = chrtbl_f7[uniCode];
        width = *(widtbl_f7+uniCode);
        height = chr_hgt_f7;
        gap = 0;
    } else if (size == 2) {
        flash_address = chrtbl_f10[uniCode];
        width = *(widtbl_f10+uniCode);
        height = chr_hgt_f10;
        gap = 1;
    } else if (size == 3) {
        flash_address = chrtbl_f16[uniCode];
        width = *(widtbl_f16+uniCode);
        height = chr_hgt_f16;
        gap = 1;
    } else if (size == 4) {
        flash_address = chrtbl_ab16[uniCode];
        width = *(widtbl_ab16+uniCode);
        height = chr_hgt_ab16;
        gap = 1;
    } else if (size == 5) {
        flash_address = chrtbl_ar24[uniCode];
        width = *(widtbl_ar24+uniCode);
        height = chr_hgt_ar24;
        gap = 1;
    }


    uint16_t w = (width+ 7) / 8;
    uint16_t pX      = 0;
    uint16_t pY      = y;
    //uint16_t color   = 0;
    uint8_t line = 0;
    uint16_t i, j, k;

    if (mag==1) {
      for(i=0; i<height; i++) {
        if (textcolor != textbgcolor) wb_drawFastHLine(x, pY, width+gap, textbgcolor);
        for (k = 0; k < w; k++) {
            line = *(flash_address + w*i + k);
            if(line) {
                pX = x + k*8;
                if(line & 0x80) wb_drawPixel(pX, pY, textcolor);
                if(line & 0x40) wb_drawPixel(pX+1, pY, textcolor);
                if(line & 0x20) wb_drawPixel(pX+2, pY, textcolor);
                if(line & 0x10) wb_drawPixel(pX+3, pY, textcolor);
                if(line & 0x8) wb_drawPixel(pX+4, pY, textcolor);
                if(line & 0x4) wb_drawPixel(pX+5, pY, textcolor);
                if(line & 0x2) wb_drawPixel(pX+6, pY, textcolor);
                if(line & 0x1) wb_drawPixel(pX+7, pY, textcolor);
            }
        }
        pY++;
      }
    } else if (mag>=2) {
      for(i=0; i<height; i++) {
        for(j=0; j<mag; j++) {
          if (textcolor != textbgcolor) wb_drawFastHLine(x, pY, mag*(width+gap), textbgcolor);
          for (k = 0; k < w; k++) {
            line = *(flash_address + w*i + k);
            if(line) {
                pX = x + k*8*mag;
                if(line & 0x80) wb_drawFastHLine(pX, pY, mag,textcolor);
                if(line & 0x40) wb_drawFastHLine(pX+1*mag, pY, mag,textcolor);
                if(line & 0x20) wb_drawFastHLine(pX+2*mag, pY, mag,textcolor);
                if(line & 0x10) wb_drawFastHLine(pX+3*mag, pY, mag,textcolor);
                if(line & 0x8) wb_drawFastHLine(pX+4*mag, pY, mag,textcolor);
                if(line & 0x4) wb_drawFastHLine(pX+5*mag, pY, mag,textcolor);
                if(line & 0x2) wb_drawFastHLine(pX+6*mag, pY, mag,textcolor);
                if(line & 0x1) wb_drawFastHLine(pX+7*mag, pY, mag,textcolor);
            }
          }
          pY++;
        }
      }
    }
    return (width+gap)*mag;
}

int wb_drawString(const char *string, uint16_t poX, uint16_t poY, uint8_t size, uint8_t mag)
{
    uint16_t sumX = 0;
    while(*string) {
        uint16_t xPlus = wb_drawChar(*string, poX, poY, size, mag);
        sumX += xPlus;
        string++;
        poX += xPlus;
    }
    return sumX;
}

void wb_setTextColor(uint16_t c1, uint16_t c2)
{
    textcolor = c1;
    textbgcolor = c2;
}

void lcdInit(int t)
{
    esp_err_t ret;
    
    /*spi_bus_config_t buscfg;
    buscfg.mosi_io_num=PIN_NUM_MOSI;
    buscfg.sclk_io_num=PIN_NUM_CLK;
    buscfg.intr_flags=ESP_INTR_FLAG_HIGH|ESP_INTR_FLAG_EDGE|ESP_INTR_FLAG_INTRDISABLED;
    buscfg.quadwp_io_num=-1;
    buscfg.quadhd_io_num=-1;
*/
    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO, .mosi_io_num=PIN_NUM_MOSI, .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1, .quadhd_io_num=-1
    };
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=20000000, .mode=0, .spics_io_num=PIN_NUM_CS,
        .queue_size=7, .pre_cb=lcd_spi_pre_transfer_callback,
    };

    if (t==2) devcfg.clock_speed_hz=40000000;
    else devcfg.clock_speed_hz=20000000;
    
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    //ESP_ERROR_CHECK(ret);
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &_lcd_spi);
    //ESP_ERROR_CHECK(ret);

    //Serial.printf("lcd_spi: %d", ret);
    
    lcd_init(t);
    //display_pretty_colors(spi);
}

//#include "/home/derek/espdev/esp-idf/components/heap/include/multi_heap.h"

void wb_init(int t)
{
    /*gpio_set_direction((gpio_num_t)2, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)2,1);
    gpio_set_direction((gpio_num_t)27, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)27,1);
    */
    lcdInit(t);

    
    gpio_set_level((gpio_num_t)27,1); // turn on lcd backlight
    
    wb_fillScreen(0xf8);
    wb_setTextColor(0xffff, 0xffff);

    REG_CLR_BIT(RTC_IO_XTAL_32K_PAD_REG, RTC_IO_X32P_MUX_SEL); /* gpio32 route to digital io_mux */
    REG_CLR_BIT(RTC_IO_XTAL_32K_PAD_REG, RTC_IO_X32N_MUX_SEL); /* gpio33 route to digital io_mux */
    
}

///////////////////////////////////////////////////////////////

// Off-Screen Engine
uint8_t *wb_buf8;
uint16_t *wb_buf16;
uint16_t wb_pal8[256];

uint16_t swapbyte(uint16_t *x) { 
  *x= (*x>>8) | (*x<<8);
  return *x;
}

uint16_t wb_color565(uint8_t r, uint8_t g, uint8_t b) 
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void wb_setPal8(uint8_t n, uint16_t c)
{
    wb_pal8[n]=(c>>8)|(c<<8);
}

void wb_blit16(uint16_t *buf)
{
    uint16_t line[1024], i, x, y;
    for (x=0; x<6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x&1)==0) {
            trans[x].length=8;
            trans[x].user=(void*)0;
        } else {
            trans[x].length=8*4;
            trans[x].user=(void*)1;
        }
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0]=0x2A;
    trans[1].tx_data[0]=0;
    trans[1].tx_data[1]=2;
    trans[1].tx_data[2]=(_w-1+_shx)>>8;
    trans[1].tx_data[3]=(_w-1+_shx)&0xff;
    trans[2].tx_data[0]=0x2B;
    trans[3].tx_data[0]=0;
    trans[3].tx_data[1]=3;
    trans[3].tx_data[2]=(_h-1+_shy)>>8;
    trans[3].tx_data[3]=(_h-1+_shy)&0xff;
    trans[4].tx_data[0]=0x2C;

    for(x=0; x<5; x++) spi_device_queue_trans(_lcd_spi, &trans[x], portMAX_DELAY);
    wait_trans(5);
    for(y=0; y<_lines; y++) {
        for(i=0;i<1024;i++) swapbyte(buf+i);
        lcd_data((uint8_t *)buf, 2048);
        buf+=1024;
    }
}

void wb_blit8()
{
    uint16_t line[1024], x, y;
    for (x=0; x<6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x&1)==0) {
            trans[x].length=8;
            trans[x].user=(void*)0;
        } else {
            trans[x].length=8*4;
            trans[x].user=(void*)1;
        }
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0]=0x2A;
    trans[1].tx_data[0]=0;
    trans[1].tx_data[1]=2;
    trans[1].tx_data[2]=(_w-1+_shx)>>8;
    trans[1].tx_data[3]=(_w-1+_shx)&0xff;
    trans[2].tx_data[0]=0x2B;
    trans[3].tx_data[0]=0;
    trans[3].tx_data[1]=3;
    trans[3].tx_data[2]=(_h-1+_shy)>>8;
    trans[3].tx_data[3]=(_h-1+_shy)&0xff;
    trans[4].tx_data[0]=0x2C;

    for(x=0; x<5; x++) spi_device_queue_trans(_lcd_spi, &trans[x], portMAX_DELAY);
    wait_trans(5);
    for(y=0; y<_lines; y++) {
        for (x=0; x<1024; x++) {
            line[x] = wb_pal8[wb_buf8[x+y*1024]]; 
        }
        lcd_data((uint8_t *)line, 2048);
    }
}

void wb_blitBuf8(uint16_t xs, uint16_t ys, uint16_t ws, int xd, int yd, uint16_t width, uint16_t height, const uint8_t *data)
{
    uint8_t d;
    int dx, dy;
    for (int y=0; y<height; y++) {
        for(int x=0; x<width; x++) {
            dx=x+xd;
            dy=y+yd;
            if (dx>=0 && dx<_w && dy>=0 && dy<_h) {
                d = data[(y+ys)*ws+x+xs];
                if (d!=0) wb_buf8[dx+(dy)*_w] = d;
            }
        }
    }
}

void wb_rot8(uint16_t dx, uint16_t dy, double angle, double scale, uint16_t offx, uint16_t offy, uint8_t w, uint8_t h, uint8_t *sprite)
{
    double duCol, dvCol, duRow, dvRow, startingu, startingv;
    double fSrcRotCX, fSrcRotCY, fDstRotCX, fDstRotCY;
    double rowu, rowv, u, v;
    int x, y, height, width, hw, hh;
    uint8_t d;
    
    if (w>h) {
        height=w*(1.5*scale);
        width=w*(1.5*scale);
    } else {
        height=h*(1.5*scale);
        width=h*(1.5*scale);
    }
    fSrcRotCX=(double)w/2.0;
    fSrcRotCY=(double)h/2.0;
    fDstRotCX=(double)(width/2); 
    fDstRotCY=(double)(height/2);
    
    duCol = sin(-angle) * (1.0 / scale);
    dvCol = cos(-angle) * (1.0 / scale);
    duRow = dvCol;
    dvRow =-duCol;
    startingu = fSrcRotCX -(fDstRotCX * dvCol + fDstRotCY * duCol);
    startingv = fSrcRotCY -(fDstRotCX * dvRow + fDstRotCY * duRow);
    
    rowu = startingu;
    rowv = startingv;
    hw = width / 2;
    hh = height / 2;
    for(y=0; y<height; y++) {
        u = rowu;
        v = rowv;
        for(x=0; x<width; x++) {
            if(u>0 && v>0 && u<w && v<h) {
                d = sprite[(int)(u)+offx+((int)(v)+offy)*_w];
                if (d>0) wb_buf8[x+dx-hw+(y+dy-hh)*_w] = d;
            }
            //else wb_buf8[x+dx+(y+dy)*240] = 0;
            u += duRow;
            v += dvRow;
        }
        rowu += duCol;
        rowv += dvCol;
    }
}

void wb_setBuf8(uint32_t i, uint8_t d)
{
    wb_buf8[i]=d;
    //wb_buf8[i+1]=d;
}

void wb_setBuf16(uint32_t i, uint16_t d)
{
    wb_buf16[i]=d;
    //wb_buf8[i+1]=d;
}

void wb_clearBuf8()
{
    memset(wb_buf8, 0, _w*_h);
}

void wb_clearBuf16()
{
    memset(wb_buf16, 0, _w*_h*2);
}

void wb_initBuf8()
{
    wb_buf8=(uint8_t *)malloc(_w*_h); // double buffers
}

void wb_initBuf16()
{
    wb_buf16=(uint8_t *)malloc(_w*_h*2); // double buffers
}

void wb_freeBuf8()
{
    free(wb_buf8); //=(uint8_t *)malloc(16384); // double buffers
}

void wb_freeBuf16()
{
    free(wb_buf16); //=(uint8_t *)malloc(16384); // double buffers
}

// TJPGD 

#include "rom/tjpgd.h"

typedef struct {
    const unsigned char *inData;  //Pointer to jpeg data
    FILE *fp;
    int inPos;            //Current position in jpeg data
    uint16_t *outData;        //Array of IMAGE_H pointers to arrays of IMAGE_W 16-bit pixel values
    int outW;           //Width of the resulting file
    int outH;           //Height of the resulting file
} JpegDev;

static UINT infunc(JDEC *decoder, BYTE* buff, UINT nd)
{
    int rb = 0;
    JpegDev *jd = (JpegDev*)decoder->device;
    if (buff) {   // Read nd bytes from the input strem
  rb = fread(buff, 1, nd, jd->fp);
  return rb;  // Returns actual number of bytes read
    } else {    // Remove nd bytes from the input stream
  if (fseek(jd->fp, nd, SEEK_CUR) >= 0) return nd;
  else return 0;
    }
}

static UINT outfunc(JDEC *decoder, void *bitmap, JRECT *rect) 
{
    JpegDev *jd=(JpegDev*)decoder->device;
    uint8_t *in=(uint8_t*)bitmap;
    for (int y=rect->top; y<=rect->bottom; y++) {
        for (int x=rect->left; x<=rect->right; x++) {
            uint16_t v=0;
            v|=((in[0]>>3)<<11);
            v|=((in[1]>>2)<<5);
            v|=((in[2]>>3)<<0);
            //v=(v>>8)|(v<<8);
            jd->outData[y*128+x]=v;
            in+=3;
        }
    }
    return 1;
}

#define WORKSZ 3100
#define IMAGE_W 128
#define IMAGE_H 128

esp_err_t decode_image(char *path) 
{
    char *work=NULL;
    int r;
    JDEC decoder;
    JpegDev jd;

    jd.fp = fopen(path, "rb"); //"/_#!#_sdcard/jpg/Rose.jpg", "rb");
    if (!jd.fp) { printf("File not found!\n"); return 0; }

    work=calloc(WORKSZ, 1);
    if (work==NULL) { printf("Out of memory!\n"); return 0; } 

    jd.inData=NULL; // not used for file decoding
    jd.inPos=0;
    jd.outData=(uint16_t *)wb_buf16;
    jd.outW=IMAGE_W;
    jd.outH=IMAGE_H;
    
    r=jd_prepare(&decoder, infunc, work, WORKSZ, (void*)&jd);
    if (r!=JDR_OK) {
        printf("jd prepare failed. %d\n", r);
        goto err;
    }
    r=jd_decomp(&decoder, outfunc, 0);
    if (r!=JDR_OK) {
        printf("jd decomp failed. %d\n", r);
        goto err;
    }

    wb_blit16((uint16_t *) wb_buf16);
    fclose(jd.fp);

err:
    free(work);
    return 0;
}

int wb_rand(int r)
{
    return (int)esp_random()%r;
}

int wb_getkey(void) 
{
    uint16_t key;

    gpio_set_direction(35, GPIO_MODE_INPUT);
    gpio_set_direction(34, GPIO_MODE_INPUT);
    gpio_set_direction(32, GPIO_MODE_INPUT);
    gpio_set_direction(33, GPIO_MODE_INPUT);

    if (_board_type != 1) {
        gpio_set_direction(39, GPIO_MODE_INPUT);
        gpio_set_direction(36, GPIO_MODE_INPUT);
    }

    if (_board_type == 2) {
        gpio_set_direction(0, GPIO_MODE_INPUT);
        gpio_set_direction(2, GPIO_MODE_INPUT);
        gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);
    }

    key=0;
    if (gpio_get_level(35)==0) key = key + 1;
    if (gpio_get_level(34)==0) key = key + 2;
    if (gpio_get_level(32)==0) key = key + 4;
    if (gpio_get_level(33)==0) key = key + 8;
    if (_board_type != 1) {
        if (gpio_get_level(39)==0) key = key + 16; // 32v2, 32m1, not 32s1
        if (gpio_get_level(36)==0) key = key + 32; // 32v2, 32m1, not 32s1
    }
    if (_board_type == 2) {
        if (gpio_get_level(2)==0) key = key + 64;
        if (gpio_get_level(0)==0) key = key + 128;
    }

    return (int)(key);
}

// Ticker Library

#include "esp32-hal-timer.h"

hw_timer_t * wb_timer = NULL;  // the esp32 timer

void wb_tickerInit(uint32_t us, void code())
{
  wb_timer = timerBegin(0, 80, true); // hw timer #0, divider=80 (1us)
  timerAttachInterrupt(wb_timer, code, true); // attach service routine, true: edge
  timerAlarmWrite(wb_timer, us, true); // timer=500000us=500ms, true=auto-reload
  timerAlarmEnable(wb_timer); // eanble timer#0
}

void wb_tickerAlarm(uint32_t us)
{
  timerAlarmWrite(wb_timer, us, true);
}

void wb_tickerEnable()
{
  timerAlarmEnable(wb_timer);
}

void wb_tickerDisable()
{
  timerAlarmEnable(wb_timer);
}

