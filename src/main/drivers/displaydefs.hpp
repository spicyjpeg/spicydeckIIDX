
#pragma once

#include <stdint.h>

namespace drivers {

/* ST7735 command definitions */

enum ST7735Command : uint8_t {
	ST7735_NOP       = 0x00, // No Operation
	ST7735_SWRESET   = 0x01, // Software reset
	ST7735_RDDID     = 0x04, // Read Display ID
	ST7735_RDDST     = 0x09, // Read Display Status
	ST7735_RDDPM     = 0x0a, // Read Display Power Mode
	ST7735_RDDMADCTL = 0x0b, // Read Display MADCTL
	ST7735_RDDCOLMOD = 0x0c, // Read Display Pixel Format
	ST7735_RDDIM     = 0x0d, // Read Display Image Mode
	ST7735_RDDSM     = 0x0e, // Read Display Signal Mode
	ST7735_RDDSDR    = 0x0f, // Read Display Self-diagnostic result (ST7735S)
	ST7735_SLPIN     = 0x10, // Sleep in & booster off
	ST7735_SLPOUT    = 0x11, // Sleep out & booster on
	ST7735_PTLON     = 0x12, // Partial mode on
	ST7735_NORON     = 0x13, // Partial off (Normal)
	ST7735_INVOFF    = 0x20, // Display inversion off
	ST7735_INVON     = 0x21, // Display inversion on
	ST7735_GAMSET    = 0x26, // Gamma curve select
	ST7735_DISPOFF   = 0x28, // Display off
	ST7735_DISPON    = 0x29, // Display on
	ST7735_CASET     = 0x2a, // Column address set
	ST7735_RASET     = 0x2b, // Row address set
	ST7735_RAMWR     = 0x2c, // Memory write
	ST7735_RGBSET    = 0x2d, // LUT for 4k,65k,262k Color display (ST7735R/S)
	ST7735_RAMRD     = 0x2e, // Memory read
	ST7735_PTLAR     = 0x30, // Partial start/end address set
	ST7735_SCRLAR    = 0x33, // Scroll area set (ST7735S)
	ST7735_TEOFF     = 0x34, // Tearing effect line off
	ST7735_TEON      = 0x35, // Tearing effect mode set & on
	ST7735_MADCTL    = 0x36, // Memory data access control
	ST7735_VSCSAD    = 0x37, // Scroll start address of RAM (ST7735S)
	ST7735_IDMOFF    = 0x38, // Idle mode off
	ST7735_IDMON     = 0x39, // Idle mode on
	ST7735_COLMOD    = 0x3a, // Interface pixel format
	ST7735_FRMCTR1   = 0xb1, // In normal mode (Full colors)
	ST7735_FRMCTR2   = 0xb2, // In Idle mode (8-colors)
	ST7735_FRMCTR3   = 0xb3, // In partial mode + Full colors
	ST7735_INVCTR    = 0xb4, // Display inversion control
	ST7735_DISSET5   = 0xb6, // Display function setting (ST7735)
	ST7735_PWCTR1    = 0xc0, // Power control setting
	ST7735_PWCTR2    = 0xc1, // Power control setting
	ST7735_PWCTR3    = 0xc2, // In normal mode (Full colors)
	ST7735_PWCTR4    = 0xc3, // In Idle mode (8-colors)
	ST7735_PWCTR5    = 0xc4, // In partial mode + Full colors
	ST7735_VMCTR1    = 0xc5, // VCOM control 1
	ST7735_VMOFCTR   = 0xc7, // Set VCOM offset control
	ST7735_WRID2     = 0xd1, // Set LCM version code
	ST7735_WRID3     = 0xd2, // Set the project code at ID3
	ST7735_RDID1     = 0xda, // Read ID1
	ST7735_RDID2     = 0xdb, // Read ID2
	ST7735_RDID3     = 0xdc, // Read ID3
	ST7735_NVCTR1    = 0xd9, // EEPROM control status
	ST7735_NVCTR2    = 0xde, // EEPROM Read Command
	ST7735_NVCTR3    = 0xdf, // EEPROM Write Command
	ST7735_GAMCTRP1  = 0xe0, // Gamma adjustment (+ polarity)
	ST7735_GAMCTRN1  = 0xe1, // Gamma adjustment (- polarity)
	ST7735_EXTCTRL   = 0xf0, // Extension Command Control (ST7735)
	ST7735_PWCTR6    = 0xfc, // In partial mode + Idle (ST7735)
	ST7735_GCV       = 0xfc, // Gate clock variable (ST7735S)
	ST7735_VCOM4L    = 0xff  // Vcom 4 Level control (ST7735)
};

enum ST7735MADCTLFlag : uint8_t {
	ST7735_MADCTL_MH_BITMASK    = 1 << 2, // Horizontal Refresh Order
	ST7735_MADCTL_MH_RIGHT      = 0 << 2, // LCD horizontal refresh Left to right
	ST7735_MADCTL_MH_LEFT       = 1 << 2, // LCD horizontal refresh right to left
	ST7735_MADCTL_ORDER_BITMASK = 1 << 3, // RGB-BGR ORDER
	ST7735_MADCTL_ORDER_RGB     = 0 << 3, // RGB color filter panel
	ST7735_MADCTL_ORDER_BGR     = 1 << 3, // BGR color filter panel
	ST7735_MADCTL_ML_BITMASK    = 1 << 4, // Vertical Refresh Order
	ST7735_MADCTL_ML_DOWN       = 0 << 4, // LCD vertical refresh Top to Bottom
	ST7735_MADCTL_ML_UP         = 1 << 4, // LCD vertical refresh Bottom to Top
	ST7735_MADCTL_MV_BITMASK    = 1 << 5, // Row/Column Exchange
	ST7735_MADCTL_MV_ROW        = 0 << 5,
	ST7735_MADCTL_MV_COLUMN     = 1 << 5,
	ST7735_MADCTL_MX_BITMASK    = 1 << 6, // Column Address Order
	ST7735_MADCTL_MX_RIGHT      = 0 << 6,
	ST7735_MADCTL_MX_LEFT       = 1 << 6,
	ST7735_MADCTL_MY_BITMASK    = 1 << 7, // Row Address Order
	ST7735_MADCTL_MY_DOWN       = 0 << 7,
	ST7735_MADCTL_MY_UP         = 1 << 7
};

enum ST7735COLMODFlag : uint8_t {
	ST7735_COLMOD_IFPF_BITMASK = 7 << 0, // MCU Interface Color Format
	ST7735_COLMOD_IFPF_12BPP   = 3 << 0, // 12-bit/pixel
	ST7735_COLMOD_IFPF_16BPP   = 5 << 0, // 16-bit/pixel
	ST7735_COLMOD_IFPF_18BPP   = 6 << 0  // 18-bit/pixel
};

enum ST7735INVCTRFlag : uint8_t {
	ST7735_INVCTR_NLC_BITMASK = 1 << 0, // Inversion setting in full Colors partial mode
	ST7735_INVCTR_NLC_LINE    = 0 << 0, // Line Inversion
	ST7735_INVCTR_NLC_FRAME   = 1 << 0, // Frame Inversion
	ST7735_INVCTR_NLB_BITMASK = 1 << 1, // Inversion setting in Idle mode
	ST7735_INVCTR_NLB_LINE    = 0 << 1, // Line Inversion
	ST7735_INVCTR_NLB_FRAME   = 1 << 1, // Frame Inversion
	ST7735_INVCTR_NLA_BITMASK = 1 << 2, // Inversion setting in full Colors normal mode
	ST7735_INVCTR_NLA_LINE    = 0 << 2, // Line Inversion
	ST7735_INVCTR_NLA_FRAME   = 1 << 2  // Frame Inversion
};

}
