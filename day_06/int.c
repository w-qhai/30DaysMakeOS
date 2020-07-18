#include "bootpack.h"

void init_pic(void) {
    io_out8(PIC0_IMR, 0xff);    /* 禁止所有中断 */
    io_out8(PIC1_IMR, 0xff);    /* 禁止所有中断 */

    io_out8(PIC0_ICW1, 0x11);   /* 边沿触发模式 （edge trigger mode） */
    io_out8(PIC0_ICW2, 0x20);   /* IRQ0~7由INT20-27接收 */
    io_out8(PIC0_ICW3, 1 << 2); /* PCI1 由IRQ2连接 */
    io_out8(PIC0_ICW4, 0x01);   /* 无缓冲区模式 */

    io_out8(PIC1_ICW1, 0x11);   /* 边沿触发模式 （edge trigger mode） */   
    io_out8(PIC1_ICW2, 0x28);   /* IRQ8~15由INT28-2F接收 */
    io_out8(PIC1_ICW3, 2);      /* PCI1 由IRQ2连接 */
    io_out8(PIC1_ICW4, 0x01);   /* 无缓冲区模式 */

    io_out8(PIC0_IMR, 0xfb);    /* 11111011 PIC1 以外全部禁止 */
    io_out8(PIC1_IMR, 0xff);    /* 11111111 PIC1 禁止所有中断 */
}

void inthandler21(int *esp) {
    struct BOOTINFO* binfo = (struct  BOOTINFO*) ADR_BOOTINFO;
    boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 21 (IRQ-1) : PS/2 keyword");
    while (1) {
        io_hlt();
    }
}

void inthandler27(int *esp) {

}

void inthandler2c(int *esp) {

}