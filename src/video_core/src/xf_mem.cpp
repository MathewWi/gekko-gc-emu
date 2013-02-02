/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    xf_mem.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-12
 * @brief   Implementation of CXF for the graphics processor
 *
 * @section LICENSE
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

#include <GL/glew.h>

#include "common.h"
#include "crc.h"
#include "memory.h"

#include "video_core.h"

#include "bp_mem.h"
#include "cp_mem.h"
#include "xf_mem.h"

#define XF_VIEWPORT_ZMAX            16777215.0f

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

namespace gp {

u32         g_xf_mem[0x800];            ///< Transformation memory
XFMemory    g_xf_regs;                  ///< XF registers
f32         g_projection_matrix[16];    ///< Decoded projection matrix

void XF_UpdateViewport() {
    int scissor_x_offs = g_bp_regs.scissor_offset.x * 2;
    int scissor_y_offs = g_bp_regs.scissor_offset.y * 2;

    // NOTE: ceil is unverified but seems to be correct in my testing, might be worth writing a demo
    // to check if it should be floor sometime in the future
    int x = ((int)ceil(g_xf_regs.viewport.x_orig - g_xf_regs.viewport.wd - (float)scissor_x_offs));
    int y = ((int)ceil((float)kGCEFBHeight - g_xf_regs.viewport.y_orig + g_xf_regs.viewport.ht + 
        (float)scissor_y_offs));
    int width = ((int)ceil(2.0f * g_xf_regs.viewport.wd));
    int height = ((int)ceil(-2.0f * g_xf_regs.viewport.ht));
    double znear = (g_xf_regs.viewport.far_z - g_xf_regs.viewport.z_range) / 16777216.0f;
    double zfar = g_xf_regs.viewport.far_z / 16777216.0f;
    if (width < 0) {
        x += width;
        width *= -1;
    }
    if (height < 0) {
        y += height;
        height *= -1;
    }
    // Update the view port
    video_core::g_renderer->SetViewport(x, y, width, height);
    video_core::g_renderer->SetDepthRange(znear, zfar);
}

/// Sets the decoded XF projection matrix
void XF_UpdateProjection() {
    memset(g_projection_matrix, 0, sizeof(g_projection_matrix));

    g_projection_matrix[0]  = g_xf_regs.projection_matrix[0]; // XF_PROJECTION_A;
    g_projection_matrix[5]  = g_xf_regs.projection_matrix[2]; // XF_PROJECTION_C;
    g_projection_matrix[10] = g_xf_regs.projection_matrix[4]; // XF_PROJECTION_E;
    g_projection_matrix[14] = g_xf_regs.projection_matrix[5]; // XF_PROJECTION_F;

    // Is it orthographic mode...
    if (g_xf_regs.projection_mode.is_orthographic) { 
        g_projection_matrix[12] = g_xf_regs.projection_matrix[1]; // XF_PROJECTION_B;
        g_projection_matrix[13] = g_xf_regs.projection_matrix[3]; // XF_PROJECTION_D;
        g_projection_matrix[15] = 1.0f;

    // Otherwise it is perspective mode
    } else { 
        g_projection_matrix[8]  = g_xf_regs.projection_matrix[1]; // XF_PROJECTION_B;
        g_projection_matrix[9]  = g_xf_regs.projection_matrix[3]; // XF_PROJECTION_D;
        g_projection_matrix[11] = -1.0f;
    }
}

void XF_Load(u32 length, u32 base_addr, u32* data) {

    // Register write
    if (base_addr & 0x1000) {
        u8 addr = (base_addr & 0xff);
        memcpy(&g_xf_regs.mem[addr], data, length << 2);

        // Viewport register range
        if (addr >= 0x1A && addr <= 0x1F) {
            XF_UpdateViewport();
        // Projection matrix register range
        } else if (addr >= 0x20 && addr <= 0x26) {
            XF_UpdateProjection();
        }

    // Transformation memory
    } else if (base_addr < 0x100) {
        memcpy(&g_xf_mem[base_addr], data, length << 2);
        video_core::g_renderer->WriteXF(base_addr, length, data);
    }
}

/// Write data into a XF register indexed-form
void XF_LoadIndexed(u8 n, u16 index, u8 length, u16 addr) {
    u32* data = (u32*)&Mem_RAM[CP_IDX_ADDR(index, n) & RAM_MASK];
    memcpy(&g_xf_mem[addr], data, length << 2);
    video_core::g_renderer->WriteXF(addr, length, data);
}

/// Initialize XF
void XF_Init() {
    memset(g_xf_mem, 0, sizeof(g_xf_mem));
    memset(&g_xf_regs, 0, sizeof(g_xf_regs));
}

} // namespace