/*!
* Copyright (C) 2005-2012 Gekko Emulator
*
* \file    bp_mem.h
* \author  ShizZy <shizzy247@gmail.com>
* \date    2012-03-10
* \brief   Implementation of BP for the graphics processor
*
* \section LICENSE
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details at
* http://www.gnu.org/copyleft/gpl.html
*
* Official project repository can be found at:
* http://code.google.com/p/gekko-gc-emu/
*/

#ifndef VIDEO_CORE_BP_MEM_H_
#define VIDEO_CORE_BP_MEM_H_

#include "common.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// BP registers

#define BP_REG_GENMODE          0x00
#define BP_REG_DISPLAYCOPYFILER 0x01 // 0x01 + 4
#define BP_REG_IND_MTXA         0x06 // 0x06 + (3 * 3)
#define BP_REG_IND_MTXB         0x07 // 0x07 + (3 * 3)
#define BP_REG_IND_MTXC         0x08 // 0x08 + (3 * 3)
#define BP_REG_IND_IMASK        0x0F
#define BP_REG_IND_CMD          0x10 // 0x10 + 16
#define BP_REG_SCISSORTL        0x20
#define BP_REG_SCISSORBR        0x21
#define BP_REG_LINEPTWIDTH      0x22
#define BP_REG_PERF0_TRI        0x23
#define BP_REG_PERF0_QUAD       0x24
#define BP_REG_RAS1_SS0         0x25
#define BP_REG_RAS1_SS1         0x26
#define BP_REG_IREF             0x27
#define BP_REG_TREF             0x28 // 0x28 + 8
#define BP_REG_SU_SSIZE         0x30 // 0x30 + (2 * 8)
#define BP_REG_SU_TSIZE         0x31 // 0x31 + (2 * 8)
#define BP_REG_PE_ZMODE         0x40
#define BP_REG_PE_CMODE0        0x41
#define BP_REG_PE_CMODE1        0x42
#define BP_REG_PE_CONTROL       0x43
#define BP_REG_FIELDMASK        0x44
#define BP_REG_PE_DRAWDONE      0x45
#define BP_REG_BUSCLOCK0        0x46
#define BP_REG_PE_TOKEN	        0x47
#define BP_REG_PE_TOKEN_INT     0x48
#define BP_REG_EFB_TL           0x49
#define BP_REG_EFB_BR           0x4A
#define BP_REG_EFB_ADDR         0x4B
#define BP_REG_MIPMAP_STRIDE    0x4D
#define BP_REG_COPYYSCALE       0x4E
#define BP_REG_PE_CLEAR_AR      0x4F
#define BP_REG_PE_CLEAR_GB      0x50
#define BP_REG_PE_CLEAR_Z       0x51
#define BP_REG_PE_COPY_EXECUTE  0x52
#define BP_REG_COPYFILTER0      0x53
#define BP_REG_COPYFILTER1      0x54
#define BP_REG_CLEARBBOX1       0x55
#define BP_REG_CLEARBBOX2       0x56
#define BP_REG_UNKOWN_57        0x57
#define BP_REG_REVBITS          0x58
#define BP_REG_SCISSOROFFSET    0x59
#define BP_REG_UNKNOWN_60       0x60
#define BP_REG_UNKNOWN_61       0x61
#define BP_REG_UNKNOWN_62       0x62
#define BP_REG_TEXMODESYNC      0x63
#define BP_REG_LOADTLUT0        0x64
#define BP_REG_LOADTLUT1        0x65
#define BP_REG_TEXINVALIDATE    0x66
#define BP_REG_PERF1            0x67
#define BP_REG_FIELDMODE        0x68
#define BP_REG_BUSCLOCK1        0x69
#define BP_REG_TX_SETMODE0      0x80 // 0x80 + 4
#define BP_REG_TX_SETMODE1      0x84 // 0x84 + 4
#define BP_REG_TX_SETIMAGE0     0x88 // 0x88 + 4
#define BP_REG_TX_SETIMAGE1     0x8C // 0x8C + 4
#define BP_REG_TX_SETIMAGE2     0x90 // 0x90 + 4
#define BP_REG_TX_SETIMAGE3     0x94 // 0x94 + 4
#define BP_REG_TX_SETTLUT       0x98 // 0x98 + 4
#define BP_REG_TX_SETMODE0_4    0xA0 // 0xA0 + 4
#define BP_REG_TX_SETMODE1_4    0xA4 // 0xA4 + 4
#define BP_REG_TX_SETIMAGE0_4   0xA8 // 0xA8 + 4
#define BP_REG_TX_SETIMAGE1_4   0xAC // 0xA4 + 4
#define BP_REG_TX_SETIMAGE2_4   0xB0 // 0xB0 + 4
#define BP_REG_TX_SETIMAGE3_4   0xB4 // 0xB4 + 4
#define BP_REG_TX_SETLUT_4      0xB8 // 0xB8 + 4
#define BP_REG_TEV_COLOR_ENV    0xC0 // 0xC0 + (2 * 16)
#define BP_REG_TEV_ALPHA_ENV    0xC1 // 0xC1 + (2 * 16)
#define BP_REG_TEV_REGISTER_L   0xE0 // 0xE0 + (2 * 4)
#define BP_REG_TEV_REGISTER_H   0xE1 // 0xE1 + (2 * 4)
#define BP_REG_FOGRANGE         0xE8
#define BP_REG_FOGPARAM0        0xEE
#define BP_REG_FOGBMAGNITUDE    0xEF
#define BP_REG_FOGBEXPONENT     0xF0
#define BP_REG_FOGPARAM3        0xF1
#define BP_REG_FOGCOLOR         0xF2
#define BP_REG_ALPHACOMPARE     0xF3
#define BP_REG_BIAS			    0xF4
#define BP_REG_ZTEX2		    0xF5
#define BP_REG_TEV_KSEL         0xF6 // 0xF6 + 8
#define BP_REG_BP_MASK          0xFE

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

namespace gp {

/// BP pixel formats
enum BPPixelFormat {
    BP_PIXELFORMAT_RGB8_Z24     = 0,
    BP_PIXELFORMAT_RGBA6_Z24    = 1,
    BP_PIXELFORMAT_RGB565_Z16   = 2,
    BP_PIXELFORMAT_Z24          = 3,
    BP_PIXELFORMAT_Y8           = 4,
    BP_PIXELFORMAT_U8           = 5,
    BP_PIXELFORMAT_V8           = 6,
    BP_PIXELFORMAT_YUV420       = 7
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// BP register decoding

/// Gen Mode
struct BPGenMode {
    union {
        struct {
            unsigned num_texgens : 4;
            unsigned num_colchans : 5;
            unsigned ms_en : 1;
            unsigned num_tevstages : 4;
            unsigned cull_mode : 2;
            unsigned num_indstages : 3;
            unsigned zfreeze : 5;
            unsigned rid : 8;
        };
        u32 _u32;
    };
};

/// PE Z mode
struct BPPEZMode {
    union {
        struct {
            u32 test_enable     : 1;
            u32 function        : 3;
            u32 update_enable   : 1;
        };
        u32 _u32;
    };
};

/// PE CMode 0
struct BPPECMode0 {
    union {
        struct {
            u32 blend_enable   : 1;
            u32 logicop_enable : 1;
            u32 dither : 1;
            u32 color_update : 1;
            u32 alpha_update : 1;
            u32 dst_factor : 3;
            u32 src_factor : 3;
            u32 subtract : 1;
            u32 logic_mode : 4;
        };
        u32 _u32;
    };
};

/// PE CMode 1
struct BPPECMode1{
    union {
        struct {
            unsigned alpha : 8;
            unsigned enable : 1;
            unsigned pad0 : 15;
            unsigned rid : 8;
        };
        u32 _u32;
    };

    f32 getalpha() { return alpha / 255.0f; }
};

/// PE control
struct BPPEControl {
    union {
        struct {
            u32 pixel_format        : 3;    // 
            u32 z_format            : 3;    // Z Compression for 16-bit Z format
            u32 z_comploc           : 1;    // 1: before tex stage
            u32 rid                 : 25;
        };
        u32 _u32;
    };
};

/// PE Copy Execute
struct BPPECopyExecute {
    union {
        struct {
            u32 clamp0              : 1; // if set clamp top
            u32 clamp1              : 1; // if set clamp bottom
            u32 yuv                 : 1; // if set, color conversion from RGB to YUV
            u32 target_pixel_format : 4; // realformat is (fmt/2)+((fmt&1)*8).
            u32 gamma               : 2; // gamma correction.. 0 = 1.0 ; 1 = 1.7 ; 2 = 2.2 ; 
                                         // 3 is reserved
            u32 half_scale          : 1; // "mipmap" filter... 0 = no filter (scale 1:1) ; 
                                         // 1 = box filter (scale 2:1)
            u32 scale_invert        : 1; // if set vertical scaling is on
            u32 clear               : 1;
            u32 frame_to_field      : 2; // 0 progressive ; 1 is reserved ; 
                                         // 2 = interlaced (even lines) ; 3 = interlaced 1 (odd)
            u32 copy_to_xfb         : 1;
            u32 intensity_fmt       : 1; // if set, is an intensity format (I4,I8,IA4,IA8)
            u32 auto_conv           : 1; // if 0 auto color conversion by texture format/pixel type
        };
        u32 _u32;
    };
    u32 tp_realFormat() { 
        return target_pixel_format / 2 + (target_pixel_format & 1) * 8;
    }
};

/// BP EFB X/Y coordinates - 10-bit
union BPEFBCoords10 {
    struct {
        unsigned x              : 10;   ///< X-component
        unsigned y              : 10;   ///< Y-component
        unsigned rid            : 12;   ///< Unused
    };
    u32 _u32;
};

/// BP EFB X/Y coordinates - 12-bit
union BPEFBCoords12 {
    struct {
        unsigned y              : 12;   ///< Y-component
        unsigned x              : 12;   ///< X-component
        unsigned rid            : 8;    ///< Unused
    };
    u32 _u32;
};

/// TEV color / alpha combiners
struct BPTevCombiner {
    union {
        struct {
            unsigned seld : 4;
            unsigned selc : 4;
            unsigned selb : 4;
            unsigned sela : 4;
            unsigned bias : 2;
            unsigned sub : 1;
            unsigned clamp : 1;
            unsigned shift : 2;
            unsigned dest : 2;
            unsigned rid : 8;
        };
        u32 _u32;
    }color;

    union {
        struct {
            unsigned rswap : 2;
            unsigned tswap : 2;
            unsigned seld : 3;
            unsigned selc : 3;
            unsigned selb : 3;
            unsigned sela : 3;
            unsigned bias : 2;
            unsigned sub : 1;
            unsigned clamp : 1;
            unsigned shift : 2;
            unsigned dest : 2;
            unsigned rid : 8;
        };
        u32 _u32;
    }alpha;
};

/// TEV konstant color/alpha selector
struct BPTevKSel {
    union {
        struct {
            unsigned xrb : 2;
            unsigned xga : 2;
            unsigned kcsel0 : 5;
            unsigned kasel0 : 5;
            unsigned kcsel1 : 5;
            unsigned kasel1 : 5;
            unsigned rid : 8;
        };
        u32 _u32;
    };

    int getkc (int stage) { return (stage&1) ? kcsel1 : kcsel0; }
    int getka (int stage) { return (stage&1) ? kasel1 : kasel0; }
};

/// TEV raster color order
struct BPTevOrder {
    union {
        struct {
            unsigned texmap0 : 3;
            unsigned texcoord0 : 3;
            unsigned texmapenable0 : 1;
            unsigned colorid0 : 3;
            unsigned pad0 : 2;
            unsigned texmap1 : 3;
            unsigned texcoord1 : 3;
            unsigned texmapenable1 : 1;
            unsigned colorid1 : 3;
            unsigned pad1 : 2;
            unsigned rid : 8;
        };
        u32 _u32;
    };

    inline int get_texmap(int stage) { return (stage&1) ? texmap1 : texmap0; }
    inline int get_texcoord(int stage) { return (stage&1) ? texcoord1 : texcoord0; }
    inline int get_enable(int stage) { return (stage&1) ? texmapenable1 : texmapenable0; }
    inline int get_colorchan(int stage) { return (stage&1) ? colorid1 : colorid0; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

union BPMemory {
    struct {
        BPGenMode       genmode;                // 0x00
        u32             pad0[0x1F];             // 0x01
        BPEFBCoords12   scissor_top_left;       // 0x20
        BPEFBCoords12   scissor_bottom_right;   // 0x21
        u32             pad1[0x6];              // 0x22
        BPTevOrder      tevorder[0x8];          // 0x28
        u32             pad2[0x10];             // 0x30
        BPPEZMode       zmode;                  // 0x40
        BPPECMode0      cmode0;                 // 0x41
        BPPECMode1      cmode1;                 // 0x42
        BPPEControl     zcontrol;               // 0x43
        u32             pad3[0x5];              // 0x44
        BPEFBCoords10   efb_top_left;           // 0x49
        BPEFBCoords10   efb_height_width;       // 0x4a
        u32             pad4[0x2];              // 0x4b
        u32             disp_stride;            // 0x4d
        u32             disp_copy_y_scale;      // 0x4e
        u32             clear_ar;               // 0x4f
        u32             clear_gb;               // 0x50
        u32             clear_z;                // 0x51
        u32             pad5[0x7];              // 0x52
        BPEFBCoords10   scissor_offset;         // 0x59
        u32             pad6[0x6E];
        BPTevCombiner   combiner[0x10];
        u32             pad7[0x16];
        BPTevKSel       ksel[0x8];
    };
    u32 mem[0x100];
};

extern BPMemory g_bp_regs; ///< BP memory/registers

/*!
 * \brief Write a BP register
 * \param addr Address (8-bit) of register
 * \param data Data (32-bit) to write to register
 */
void BPRegisterWrite(u8 addr, u32 data);

void LoadTexture(u8 index);

/// Initialize BP
void BPInit();

} // namespace

#endif // VIDEO_CORE_BP_MEM_H_
