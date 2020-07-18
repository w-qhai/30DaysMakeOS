#include "bootpack.h"
#include <stdio.h>

extern struct FIFO8 keyfifo, mousefifo;

struct  MOUSE_DES {
	unsigned char buf[3], phase;
	int x, y, btn;
};

void wait_KBC_sendready(void);
void init_keyboard(void);
void enable_mouse(struct MOUSE_DES* des);
int mouse_decode(struct MOUSE_DES* des, unsigned char dat);

void HariMain(void) {
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct MOUSE_DES mdes;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	unsigned char  buf[3], mouse_phase;
	int mx, my;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC的初始化已经完成，于是开放CPU的中断 */

	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* 开放PIC1和键盘中断(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) */

	init_keyboard();

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2; /* 计算画面中心坐标 */
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	enable_mouse(&mdes);
	mouse_phase = 0;


    while (1) {
        io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		}
		else {
			if (fifo8_status(&keyfifo)) {
				int data = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", data);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo)) {
				int data = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdes, data)) {

					sprintf(s, "[lcr %4d %4d]", mdes.x, mdes.y);
					if ((mdes.btn & 0x01) != 0) {
						s[3] = 'L';
					}
					else if ((mdes.btn & 0x02) != 0) {
						s[3] = 'R';
					} 
					else if ((mdes.btn & 0x04) != 0) {
						s[3] = 'C';
					} 
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 15, COL8_FFFFFF, s);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15);
					mx += mdes.x;
					my += mdes.y;

					mx = mx < 0 ? 0 : mx;
					mx = mx > binfo->scrnx - 16 ? binfo->scrnx - 16 : mx;
					my = my < 0 ? 0 : my;
					my = my > binfo->scrny - 16 ? binfo->scrny - 16 : my;

					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
				}
			}
		}
    }
}

#define PORT_KEYDAT             0x0060
#define PORT_KEYSTA             0x0064
#define PORT_KEYCMD             0x0064
#define KEYSTA_SEND_NOTREADY    0x02
#define KEYCMD_WRITE_MODE       0x60
#define KBC_MODE                0x47

void wait_KBC_sendready(void) {
	while (1) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			 break;
		}
	}
}

void init_keyboard(void) {
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);

	struct BOOTINFO* binfo = (struct BOOTINFO*) ADR_BOOTINFO;
	putfonts8_asc(binfo->vram, binfo->scrnx, 200, 14, COL8_C6C6C6, "init_keyboard");
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(struct MOUSE_DES* des) {
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	des->phase = 0;
}

int mouse_decode(struct MOUSE_DES* des, unsigned char dat) {
	if (des->phase == 0) {
		if (dat == 0xfa) {
			des->phase = 1;
		}
	}
	else if (des->phase == 1) {
		/*
		滑动：		1000
		左键：		1001
		右键：		1010
		滑轮按下：	1100
		*/
		if ((dat & 0xc8) == 0x08) { /* dat & 11001000 == 00001000 */
			des-> buf[0] = dat;
			des->phase++;
		}
	}
	else if (des->phase == 2) {
		des-> buf[1] = dat;
		des->phase++;
	}
	else if (des->phase == 3) {
		des-> buf[2] 	= dat;
		des->phase 		= 1;
		des->btn   		= des->buf[0] & 0x07; /* dat & 00000111 取后三位*/
		des->x			= des->buf[1];
		des->y 			= des->buf[2];

		if ((des->buf[0] & 0x10) != 0) {	 /* des->buf[0] & 00010000 */
			des->x |= 0xffffff00;
		}
		if ((des->buf[0] & 0x20) != 0) {	/* des->buf[0] & 00100000 */
			des->y |= 0xffffff00;
		}
		des->y = -des->y;
		return 1;
	}
	return 0;
}
