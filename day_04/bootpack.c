void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char* rgb);
void boxfill8(unsigned char* vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

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

void HariMain(void) {

    char* vram = (char*) 0xa0000;/* 声明变量vram、用于BYTE [...]地址 */
	int xsize = 320, ysize = 200;

	init_palette();/* 设定调色板 */

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