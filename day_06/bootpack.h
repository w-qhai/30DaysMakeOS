/* asmhead.nas */
#include <stdarg.h>
#include <stdio.h>
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
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

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
void init_screen8(char* vram, int xsize, int ysize);
void putfont8(char* varm, int xsize, int x, int y, char color, char* font);
void putfonts8_asc(char* varm, int xsize, int x, int y, char color, char* s);
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

#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e

struct SEGMENT_DESCRIPTOR {
    short limit_low, base_low;
    char base_mid, access_right;    /* 访问权*/
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

/* int.c */

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

void init_pic(void);
void inthandler21(int *esp);
void inthandler27(int *esp);
void inthandler2c(int *esp);
