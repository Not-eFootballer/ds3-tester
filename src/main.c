/*
 * DS3 Button Tester - PS Vita / PS TV
 * Test all DualShock 3 controller buttons
 *
 * Double-buffered CDRAM framebuffer with 256KB alignment.
 * Based on vitasdk/samples debugScreen pattern.
 */

#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Display ───────────────────────────────────────────────────── */

#define SCREEN_W  960
#define SCREEN_H  544
#define FB_SIZE   (SCREEN_W * SCREEN_H * 4)
#define FB_ALIGN  (256 * 1024)

static uint32_t *vram[2];
static SceUID    vram_uid[2];
static int       cur = 0;  /* index of buffer we draw into */

/* Pointer to the current draw buffer */
#define PX(x,y) vram[cur][(y) * SCREEN_W + (x)]

/* Couleurs (A8B8G8R8 = 0xAABBGGRR) */
#define COL_BG        0xFF1A1A1A
#define COL_BORDER    0xFF888888
#define COL_TEXT      0xFFFFFFFF
#define COL_TITLE     0xFF00D4FF
#define COL_PRESSED   0xFF00FF00
#define COL_RELEASED  0xFF505050
#define COL_ANALOG_BG 0xFF2A2A2A
#define COL_ANALOG_PT 0xFF00AAFF
#define COL_DPAD      0xFF666666
#define COL_PANEL     0xFF333333

/* ── Tiny font 5x7 ────────────────────────────────────────────── */
static const unsigned char font5x7[96][7] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x04,0x04,0x04,0x04,0x00,0x04,0x00},
    {0x0A,0x0A,0x00,0x00,0x00,0x00,0x00},
    {0x0A,0x1F,0x0A,0x1F,0x0A,0x00,0x00},
    {0x0E,0x15,0x0E,0x14,0x0E,0x00,0x00},
    {0x13,0x08,0x04,0x02,0x19,0x00,0x00},
    {0x06,0x09,0x06,0x15,0x0E,0x00,0x00},
    {0x04,0x04,0x00,0x00,0x00,0x00,0x00},
    {0x08,0x04,0x04,0x04,0x08,0x00,0x00},
    {0x02,0x04,0x04,0x04,0x02,0x00,0x00},
    {0x00,0x0A,0x04,0x0A,0x00,0x00,0x00},
    {0x00,0x04,0x0E,0x04,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x04,0x04,0x02,0x00},
    {0x00,0x00,0x0E,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x04,0x00,0x00},
    {0x10,0x08,0x04,0x02,0x01,0x00,0x00},
    {0x0E,0x11,0x11,0x11,0x0E,0x00,0x00},
    {0x04,0x06,0x04,0x04,0x0E,0x00,0x00},
    {0x0E,0x11,0x0C,0x02,0x1F,0x00,0x00},
    {0x0E,0x10,0x0C,0x10,0x0E,0x00,0x00},
    {0x08,0x0C,0x0A,0x1F,0x08,0x00,0x00},
    {0x1F,0x01,0x0F,0x10,0x0F,0x00,0x00},
    {0x0E,0x01,0x0F,0x11,0x0E,0x00,0x00},
    {0x1F,0x10,0x08,0x04,0x04,0x00,0x00},
    {0x0E,0x11,0x0E,0x11,0x0E,0x00,0x00},
    {0x0E,0x11,0x1E,0x10,0x0E,0x00,0x00},
    {0x00,0x04,0x00,0x04,0x00,0x00,0x00},
    {0x00,0x04,0x00,0x04,0x02,0x00,0x00},
    {0x08,0x04,0x02,0x04,0x08,0x00,0x00},
    {0x00,0x0E,0x00,0x0E,0x00,0x00,0x00},
    {0x02,0x04,0x08,0x04,0x02,0x00,0x00},
    {0x0E,0x11,0x08,0x00,0x04,0x00,0x00},
    {0x0E,0x11,0x1D,0x01,0x0E,0x00,0x00},
    {0x0E,0x11,0x1F,0x11,0x11,0x00,0x00},
    {0x0F,0x11,0x0F,0x11,0x0F,0x00,0x00},
    {0x0E,0x11,0x01,0x11,0x0E,0x00,0x00},
    {0x07,0x09,0x11,0x09,0x07,0x00,0x00},
    {0x1F,0x01,0x0F,0x01,0x1F,0x00,0x00},
    {0x1F,0x01,0x0F,0x01,0x01,0x00,0x00},
    {0x0E,0x01,0x19,0x11,0x0E,0x00,0x00},
    {0x11,0x11,0x1F,0x11,0x11,0x00,0x00},
    {0x0E,0x04,0x04,0x04,0x0E,0x00,0x00},
    {0x1C,0x10,0x10,0x11,0x0E,0x00,0x00},
    {0x09,0x05,0x03,0x05,0x09,0x00,0x00},
    {0x01,0x01,0x01,0x01,0x1F,0x00,0x00},
    {0x11,0x1B,0x15,0x11,0x11,0x00,0x00},
    {0x11,0x13,0x15,0x19,0x11,0x00,0x00},
    {0x0E,0x11,0x11,0x11,0x0E,0x00,0x00},
    {0x0F,0x11,0x0F,0x01,0x01,0x00,0x00},
    {0x0E,0x11,0x11,0x09,0x16,0x00,0x00},
    {0x0F,0x11,0x0F,0x09,0x11,0x00,0x00},
    {0x0E,0x01,0x0E,0x10,0x0E,0x00,0x00},
    {0x1F,0x04,0x04,0x04,0x04,0x00,0x00},
    {0x11,0x11,0x11,0x11,0x0E,0x00,0x00},
    {0x11,0x11,0x11,0x0A,0x04,0x00,0x00},
    {0x11,0x11,0x15,0x1B,0x11,0x00,0x00},
    {0x11,0x0A,0x04,0x0A,0x11,0x00,0x00},
    {0x11,0x0A,0x04,0x04,0x04,0x00,0x00},
    {0x1F,0x08,0x04,0x02,0x1F,0x00,0x00},
    {0x0E,0x02,0x02,0x02,0x0E,0x00,0x00},
    {0x01,0x02,0x04,0x08,0x10,0x00,0x00},
    {0x0E,0x08,0x08,0x08,0x0E,0x00,0x00},
    {0x04,0x0A,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x1F,0x00,0x00},
    {0x02,0x04,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x0E,0x12,0x12,0x1C,0x00,0x00},
    {0x01,0x0F,0x11,0x11,0x0F,0x00,0x00},
    {0x00,0x0E,0x01,0x01,0x0E,0x00,0x00},
    {0x10,0x1E,0x11,0x11,0x1E,0x00,0x00},
    {0x00,0x0E,0x1F,0x01,0x0E,0x00,0x00},
    {0x0C,0x02,0x07,0x02,0x02,0x00,0x00},
    {0x00,0x1E,0x11,0x1E,0x10,0x0E,0x00},
    {0x01,0x0F,0x11,0x11,0x11,0x00,0x00},
    {0x04,0x00,0x04,0x04,0x04,0x00,0x00},
    {0x08,0x00,0x08,0x08,0x09,0x06,0x00},
    {0x01,0x09,0x05,0x03,0x09,0x00,0x00},
    {0x06,0x04,0x04,0x04,0x0E,0x00,0x00},
    {0x00,0x0B,0x15,0x15,0x11,0x00,0x00},
    {0x00,0x0F,0x11,0x11,0x11,0x00,0x00},
    {0x00,0x0E,0x11,0x11,0x0E,0x00,0x00},
    {0x00,0x0F,0x11,0x0F,0x01,0x01,0x00},
    {0x00,0x1E,0x11,0x1E,0x10,0x10,0x00},
    {0x00,0x0D,0x03,0x01,0x01,0x00,0x00},
    {0x00,0x0E,0x06,0x08,0x0E,0x00,0x00},
    {0x02,0x07,0x02,0x02,0x0C,0x00,0x00},
    {0x00,0x11,0x11,0x11,0x1E,0x00,0x00},
    {0x00,0x11,0x11,0x0A,0x04,0x00,0x00},
    {0x00,0x11,0x15,0x15,0x0A,0x00,0x00},
    {0x00,0x11,0x0A,0x0A,0x11,0x00,0x00},
    {0x00,0x11,0x0A,0x04,0x02,0x01,0x00},
    {0x00,0x1F,0x08,0x04,0x1F,0x00,0x00},
    {0x08,0x04,0x02,0x04,0x08,0x00,0x00},
    {0x04,0x04,0x04,0x04,0x04,0x00,0x00},
    {0x02,0x04,0x08,0x04,0x02,0x00,0x00},
    {0x00,0x05,0x0A,0x00,0x00,0x00,0x00},
};

/* ── Drawing (writes directly to vram[cur]) ────────────────────── */

static inline void put_pixel(int x, int y, uint32_t c) {
    if ((unsigned)x < SCREEN_W && (unsigned)y < SCREEN_H)
        vram[cur][y * SCREEN_W + x] = c;
}

static void fill_rect(int x, int y, int w, int h, uint32_t c) {
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + w) > SCREEN_W ? SCREEN_W : (x + w);
    int y1 = (y + h) > SCREEN_H ? SCREEN_H : (y + h);
    for (int j = y0; j < y1; j++)
        for (int i = x0; i < x1; i++)
            vram[cur][j * SCREEN_W + i] = c;
}

static void draw_rect(int x, int y, int w, int h, uint32_t c) {
    fill_rect(x, y, w, 1, c);
    fill_rect(x, y + h - 1, w, 1, c);
    fill_rect(x, y, 1, h, c);
    fill_rect(x + w - 1, y, 1, h, c);
}

static void fill_circle(int cx, int cy, int r, uint32_t c) {
    int r2 = r * r;
    for (int dy = -r; dy <= r; dy++) {
        int dx = 0;
        while (dx * dx + dy * dy <= r2) dx++;
        dx--;
        fill_rect(cx - dx, cy + dy, dx * 2 + 1, 1, c);
    }
}

static void draw_char(int x, int y, char ch, uint32_t c, int s) {
    if (ch < 32 || ch > 126) return;
    const unsigned char *g = font5x7[ch - 32];
    for (int row = 0; row < 7; row++)
        for (int col = 0; col < 5; col++)
            if (g[row] & (1 << col))
                fill_rect(x + col * s, y + row * s, s, s, c);
}

static void draw_text(int x, int y, const char *t, uint32_t c, int s) {
    while (*t) { draw_char(x, y, *t, c, s); x += 6 * s; t++; }
}

static void draw_text_c(int cx, int y, const char *t, uint32_t c, int s) {
    draw_text(cx - (int)strlen(t) * 3 * s, y, t, c, s);
}

/* ── UI widgets ────────────────────────────────────────────────── */

static void draw_btn(int x, int y, int w, int h, const char *lbl, int on) {
    fill_rect(x, y, w, h, on ? COL_PRESSED : COL_RELEASED);
    draw_rect(x, y, w, h, on ? COL_PRESSED : COL_BORDER);
    draw_text_c(x + w/2, y + (h - 14)/2, lbl, on ? 0xFF000000 : COL_TEXT, 2);
}

static void draw_stick(int cx, int cy, int r, unsigned char ax, unsigned char ay, const char *lbl) {
    fill_circle(cx, cy, r, COL_ANALOG_BG);
    /* crosshair */
    for (int i = cx - r; i <= cx + r; i++) put_pixel(i, cy, COL_BORDER);
    for (int j = cy - r; j <= cy + r; j++) put_pixel(cx, j, COL_BORDER);
    /* dot */
    int px = cx + (ax - 128) * (r - 8) / 128;
    int py = cy + (ay - 128) * (r - 8) / 128;
    fill_circle(px, py, 8, COL_ANALOG_PT);
    draw_text_c(cx, cy + r + 8, lbl, COL_TEXT, 2);
    char buf[32];
    snprintf(buf, sizeof(buf), "X:%3d Y:%3d", ax, ay);
    draw_text_c(cx, cy + r + 28, buf, COL_TITLE, 1);
}

static void draw_dpad(int cx, int cy, int u, int d, int l, int r) {
    int s = 28;
    fill_rect(cx-s/2, cy-s-s/2, s, s, u ? COL_PRESSED : COL_DPAD);
    draw_text_c(cx, cy-s-s/2+8, "U", u ? 0xFF000000 : COL_TEXT, 2);
    fill_rect(cx-s/2, cy+s/2, s, s, d ? COL_PRESSED : COL_DPAD);
    draw_text_c(cx, cy+s/2+8, "D", d ? 0xFF000000 : COL_TEXT, 2);
    fill_rect(cx-s-s/2, cy-s/2, s, s, l ? COL_PRESSED : COL_DPAD);
    draw_text_c(cx-s, cy-s/2+8, "L", l ? 0xFF000000 : COL_TEXT, 2);
    fill_rect(cx+s/2, cy-s/2, s, s, r ? COL_PRESSED : COL_DPAD);
    draw_text_c(cx+s, cy-s/2+8, "R", r ? 0xFF000000 : COL_TEXT, 2);
    fill_rect(cx-s/2, cy-s/2, s, s, COL_PANEL);
}

/* ── CDRAM alloc with proper 256KB alignment ───────────────────── */

static SceUID alloc_cdram(const char *name, unsigned int size, void **addr) {
    SceKernelAllocMemBlockOpt opt;
    memset(&opt, 0, sizeof(opt));
    opt.size = sizeof(opt);
    opt.attr = 0x00000004;           /* SCE_KERNEL_ALLOC_MEMBLOCK_ATTR_HAS_ALIGNMENT */
    opt.alignment = FB_ALIGN;        /* 256 KB */

    SceUID uid = sceKernelAllocMemBlock(name, SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, size, &opt);
    if (uid >= 0)
        sceKernelGetMemBlockBase(uid, addr);
    return uid;
}

/* ── Main ──────────────────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    /* Allocate two 256KB-aligned CDRAM framebuffers */
    vram_uid[0] = alloc_cdram("fb0", 2 * 1024 * 1024, (void **)&vram[0]);
    vram_uid[1] = alloc_cdram("fb1", 2 * 1024 * 1024, (void **)&vram[1]);

    /* Clear both buffers */
    memset(vram[0], 0, SCREEN_W * SCREEN_H * 4);
    memset(vram[1], 0, SCREEN_W * SCREEN_H * 4);

    SceDisplayFrameBuf fb;
    memset(&fb, 0, sizeof(fb));
    fb.size        = sizeof(fb);
    fb.base        = vram[0];
    fb.pitch       = SCREEN_W;
    fb.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
    fb.width       = SCREEN_W;
    fb.height      = SCREEN_H;
    sceDisplaySetFrameBuf(&fb, SCE_DISPLAY_SETBUF_NEXTFRAME);

    /* Init controller */
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    sceCtrlSetSamplingModeExt(SCE_CTRL_MODE_ANALOG_WIDE);

    SceCtrlData pad;
    unsigned int btns;

    while (1) {
        /* ── Read input from all ports ── */
        btns = 0;
        unsigned char lx = 128, ly = 128, rx = 128, ry = 128;
        for (int p = 0; p < 4; p++) {
            memset(&pad, 0, sizeof(pad));
            sceCtrlPeekBufferPositiveExt2(p, &pad, 1);
            btns |= pad.buttons;
            if (pad.lx != 128) lx = pad.lx;
            if (pad.ly != 128) ly = pad.ly;
            if (pad.rx != 128) rx = pad.rx;
            if (pad.ry != 128) ry = pad.ry;
        }

        /* ── Draw to back buffer (cur) ── */

        /* Clear */
        for (int i = 0; i < SCREEN_W * SCREEN_H; i++)
            vram[cur][i] = COL_BG;

        /* Title */
        draw_text_c(SCREEN_W/2, 15, "DS3 BUTTON TESTER", COL_TITLE, 3);
        draw_text_c(SCREEN_W/2, 50, "PS Vita / PS TV - DualShock 3", COL_TEXT, 2);
        fill_rect(40, 72, SCREEN_W - 80, 2, COL_BORDER);

        /* D-Pad */
        draw_text_c(120, 85, "D-PAD", COL_TITLE, 2);
        draw_dpad(120, 145,
            (btns & SCE_CTRL_UP) != 0,
            (btns & SCE_CTRL_DOWN) != 0,
            (btns & SCE_CTRL_LEFT) != 0,
            (btns & SCE_CTRL_RIGHT) != 0);

        /* Face */
        draw_text_c(840, 85, "FACE", COL_TITLE, 2);
        int fcx = 840, fcy = 140;
        fill_circle(fcx, fcy-35, 16, (btns & SCE_CTRL_TRIANGLE) ? COL_PRESSED : COL_RELEASED);
        draw_text_c(fcx, fcy-42, "T", (btns & SCE_CTRL_TRIANGLE) ? 0xFF000000 : COL_TEXT, 2);
        fill_circle(fcx, fcy+35, 16, (btns & SCE_CTRL_CROSS) ? COL_PRESSED : COL_RELEASED);
        draw_text_c(fcx, fcy+28, "X", (btns & SCE_CTRL_CROSS) ? 0xFF000000 : COL_TEXT, 2);
        fill_circle(fcx-35, fcy, 16, (btns & SCE_CTRL_SQUARE) ? COL_PRESSED : COL_RELEASED);
        draw_text_c(fcx-35, fcy-7, "S", (btns & SCE_CTRL_SQUARE) ? 0xFF000000 : COL_TEXT, 2);
        fill_circle(fcx+35, fcy, 16, (btns & SCE_CTRL_CIRCLE) ? COL_PRESSED : COL_RELEASED);
        draw_text_c(fcx+35, fcy-7, "O", (btns & SCE_CTRL_CIRCLE) ? 0xFF000000 : COL_TEXT, 2);

        /* Shoulders */
        draw_btn(280, 85, 80, 30, "L1", (btns & SCE_CTRL_L1) != 0);
        draw_btn(370, 85, 80, 30, "R1", (btns & SCE_CTRL_R1) != 0);
        draw_btn(280, 123, 80, 30, "L2", (btns & SCE_CTRL_LTRIGGER) != 0);
        draw_btn(370, 123, 80, 30, "R2", (btns & SCE_CTRL_RTRIGGER) != 0);
        draw_btn(460, 123, 80, 30, "L3", (btns & SCE_CTRL_L3) != 0);
        draw_btn(550, 123, 80, 30, "R3", (btns & SCE_CTRL_R3) != 0);

        /* Center */
        draw_btn(380, 165, 90, 28, "SELECT", (btns & SCE_CTRL_SELECT) != 0);
        draw_btn(490, 165, 90, 28, "START",  (btns & SCE_CTRL_START) != 0);

        /* PS */
        fill_circle(SCREEN_W/2, 215, 18, (btns & SCE_CTRL_INTERCEPTED) ? COL_PRESSED : COL_RELEASED);
        draw_text_c(SCREEN_W/2, 208, "PS", (btns & SCE_CTRL_INTERCEPTED) ? 0xFF000000 : COL_TEXT, 2);

        /* Sticks */
        draw_stick(250, 370, 70, lx, ly, "LEFT STICK");
        draw_stick(710, 370, 70, rx, ry, "RIGHT STICK");

        /* Raw hex */
        char hex[64];
        snprintf(hex, sizeof(hex), "RAW: 0x%08X", btns);
        draw_text_c(SCREEN_W/2, 490, hex, COL_TITLE, 2);

        /* Footer */
        draw_text_c(SCREEN_W/2, 515, "START+SELECT to quit", COL_TEXT, 1);
        draw_text_c(SCREEN_W/2, 528, "by Not-eFootballer", COL_TITLE, 1);

        /* ── Present: tell display to use this buffer, then wait ── */
        fb.base = vram[cur];
        sceDisplaySetFrameBuf(&fb, SCE_DISPLAY_SETBUF_NEXTFRAME);
        sceDisplayWaitVblankStart();

        /* Swap to other buffer for next frame */
        cur ^= 1;

        /* Exit */
        if ((btns & SCE_CTRL_START) && (btns & SCE_CTRL_SELECT))
            break;
    }

    sceKernelFreeMemBlock(vram_uid[0]);
    sceKernelFreeMemBlock(vram_uid[1]);
    sceKernelExitProcess(0);
    return 0;
}
