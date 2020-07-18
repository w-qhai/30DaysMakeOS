/* asmhead.nas */
#include <stdarg.h>
struct BOOTINFO {
    char  cyls;             /* 启动区读硬盘到何处为止 */
    char  leds;             /* 启动时键盘LED的状态 */
    char  vmode;            /* 显卡模式为多少位彩色 */
    char  reserve;          
    short scrnx, scrny;     /* 画面分辨率 */
    char* vram;
}; 
#define ADR_BOOTINFO        0x00000ff0;

/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int  io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

/* graphic.c */
#define COL8_000000		0   /* 0:黑 */
#define COL8_FF0000		1   /* 1:亮红 */
#define COL8_00FF00		2   /* 2:亮绿 */
#define COL8_FFFF00		3   /* 3:亮黄 */
#define COL8_0000FF		4   /* 4:亮蓝 */
#define COL8_FF00FF		5   /* 5:亮紫 */
#define COL8_00FFFF		6   /* 6:浅亮蓝 */
#define COL8_FFFFFF		7   /* 7:白 */
#define COL8_C6C6C6		8   /* 8:亮灰 */
#define COL8_840000		9   /* 9:暗红 */
#define COL8_008400		10  /* 10:暗绿 */
#define COL8_848400		11  /* 11:暗黄 */
#define COL8_000084		12  /* 12:暗青 */
#define COL8_840084		13  /* 13:暗紫 */
#define COL8_008484		14  /* 14:浅暗蓝 */
#define COL8_848484		15  /* 15:暗灰 */

void init_palette(void);
void set_palette(int start, int end, unsigned char* rgb);
void boxfill8(unsigned char* vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(char* vram, int xsize, int ysize);
void putfont8(char* varm, int xsize, int x, int y, char color, char* font);
void putfont8_asc(char* varm, int xsize, int x, int y, char color, char* s);
int  m_sprintf(char* string, const char* format, ...);
void init_mouse_cursor8(char* mouse, char bc);
/*
Summary:  draw a block
Parameters:
    char* vram,         
    int vxsize:     screen width;        
    int pxsize:     block width;         
    int pysize:     block height;
    int px0,        block x coordinate 
    int py0,        block y coordinate
    char* buf,      content of block
    int bxsize      pxsize
Return : true is connect successfully.
*/
void putblock8_8(char* vram, int vxsize, int pxsize, int pysize, int px0, int py0, char* buf, int bxsize);

/* destbl.c */
struct SEGMENT_DESCRIPTOR {
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR* sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR* gd, int offset, int selector, int ar);


void HariMain(void) {

    struct BOOTINFO* binfo = (struct BOOTINFO*) ADR_BOOTINFO;     //该地址存储着BOOTINFO
    init_palette();/* 设定调色板 */
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    putfont8_asc(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "SO1231412*(&^@^^$!*@$");

    char s[20];
    m_sprintf(s, "scrxn: %d * %d", binfo->scrnx, binfo->scrny);
    putfont8_asc(binfo->vram, binfo->scrnx, 8, 24, COL8_FFFFFF, s);

    char mcursor[256];
    int mx = (binfo->scrnx - 16) / 2;
    int my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
    while (1) {
        io_hlt();
    }
}

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

void init_screen(char* vram, int xsize, int ysize) {
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

void putfont8_asc(char* vram, int xsize, int x, int y, char color, char* s) {

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

void init_gdtidt(void) {
    struct SEGMENT_DESCRIPTOR*  gdt = (struct SEGMENT_DESCRIPTOR*)  0x00270000;
    struct GATE_DESCRIPTOR*     idt = (struct GATE_DESCRIPTOR*)     0x0026f800;

    /* GDT的初始化 */
    for (int i = 0; i < 8192; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }

    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x00007fff, 0x00280000, 0x409a);

    /* IDT的初始化 */
    for (int i = 0; i < 256; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x7ff, 0x0026f800);
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR* sd, unsigned int limit, int base, int ar) {
    if (limit > 0xfffff) {
        ar |= 0x8000; /* G_bit = 1 */
        limit /= 0x1000;
    }
    sd->limit_low       = limit & 0xffff;
    sd->base_low        = base  & 0xffff;
    sd->base_mid        = (base >> 16) & 0xff;
    sd->access_right    = ar & 0xff;
    sd->limit_high      = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high       = (base >> 24) & 0xff;
}

void set_gatedesc(struct GATE_DESCRIPTOR* gd, int offset, int selector, int ar) {
    gd->offset_low      = offset & 0xffff;
    gd->selector        = selector;
    gd->dw_count        = (ar >> 8) & 0xff;
    gd->access_right    = ar & 0xff;
    gd->offset_high     = (offset >> 16) & 0xffff;
}

