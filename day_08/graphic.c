#include "bootpack.h"

void init_palette(void) {
    static unsigned char table_rgb[16 * 3] = {
        0x00, 0x00, 0x00,   /* 0:黑 */
        0xff, 0x00, 0x00,   /* 1:亮红 */
        0x00, 0xff, 0x00,   /* 2:亮绿 */
        0xff, 0xff, 0x00,   /* 3:亮黄 */
        0x00, 0x00, 0xff,   /* 4:亮蓝 */
        0xff, 0x00, 0xff,   /* 5:亮紫 */
        0x00, 0xff, 0xff,   /* 6:浅亮蓝 */
        0xff, 0xff, 0xff,   /* 7:白 */
        0xc6, 0xc6, 0xc6,   /* 8:亮灰 */
        0x84, 0x00, 0x00,   /* 9:暗红 */
        0x00, 0x84, 0x00,   /* 10:暗绿 */
        0x84, 0x84, 0x00,   /* 11:暗黄 */
        0x00, 0x00, 0x84,   /* 12:暗青 */
        0x84, 0x00, 0x84,   /* 13:暗紫 */
        0x00, 0x84, 0x84,   /* 14:浅暗蓝 */
        0x84, 0x84, 0x84    /* 15:暗灰 */
    };

    set_palette(0, 15, table_rgb);
}

void set_palette(int start, int end, unsigned char* rgb){
    int eflags = io_load_eflags();      /* 记录中断许可标志的值 */
    io_cli();                           /* 将中断许可标志置为0，禁止中断 */
    io_out8(0x03c8, start);

    for (int i = 0; i <= end; i++) {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags);            /* 复原中断许可标志 */
    return;
}

void boxfill8(unsigned char* vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1) {
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            vram[y * xsize + x] = c;
        }
    }
}

void init_screen8(char* vram, int xsize, int ysize) {
    /* 根据 0xa0000 + x + y * 320 计算坐标 8*/

	boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);         /* 浅暗蓝色桌面 */
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);         /* 任务栏上方亮灰色条纹 */
	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);         /* 任务栏上方白色条纹 */
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);         /* 亮灰色任务栏 */
  
	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);         /*  */
	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);         /*  */
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);         /*  */
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);         /*  */
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);         /*  */
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);         /*  */
  
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);         /*  */
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);         /*  */
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);         /*  */
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);         /*  */
}

void putfont8(char* vram, int xsize, int x, int y, char color, char* font) {
    for (int i = 0; i < 16; i++) {
        char* p = vram + (y + i) * xsize + x;
        for (int j = 0; j < 8; j++) {
            if (font[i] & (0x80 >> j)) {
                p[j] = color;
            }
        }
    }
}

void putfonts8_asc(char* vram, int xsize, int x, int y, char color, char* s) {

    extern char hankaku[256 * 16];

    for (int i = 0; s[i]; i++) {
        putfont8(vram, xsize, x + i * 8, y, color, hankaku + *(s + i) * 16);
    }
}

int  m_sprintf(char* string, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int length = 0;
    int i = 0;
    while (format[i]) {
        if (format[i] == '%') {
            if (format[i + 1] == 'd') {
                int num = va_arg(args, int);
                int digits = 1;
                while (digits * 10 < num) {
                    digits *= 10;
                }

                while (digits) {
                    string[length++] = '0' + num / digits;
                    num %= digits;
                    digits /= 10;
                }
                i++;
            }
            else if (format[i + 1] == 's') {
                char* s = va_arg(args, char*);
                int j = 0;
                while (s[j]) {
                    string[length++] = s[j++];
                }
                i++;
            }
        }
        else {
            string[length++] = format[i];
        }
        i++;
    }
    va_end(args);
    string[length] = '\0';
    return length;
}

void init_mouse_cursor8(char* mouse, char bc) {
    static char cursor[16][16] = {
        "**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
    };

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (cursor[y][x] == '*') {
                mouse[y * 16 + x] = COL8_000000;
            }
            else if (cursor[y][x] == 'O') {
                mouse[y * 16 + x] = COL8_FFFFFF;
            }
            else if (cursor[y][x] == '.') {
                mouse[y * 16 + x] = bc;
            }
        }
    }
}

void putblock8_8(char* vram, int vxsize, int pxsize, int pysize, int px0, int py0, char* buf, int bxsize) {
    for (int y = 0; y < pysize; y++) {
        for (int x = 0; x < pxsize; x++) {
            vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
        }
    }
}
