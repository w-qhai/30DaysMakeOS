#include "bootpack.h"

struct FIFO8 keyfifo, mousefifo;

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
    io_out8(PIC0_OCW2, 0x61); /* 通知PIC“IRQ-01已经受理完毕” */
    unsigned char data;
    data = io_in8(PORT_KEYDAT);
    fifo8_put(&keyfifo, data);
}

void inthandler27(int *esp) {
    io_out8(PIC0_OCW2, 0x67); /* 通知PIC的IRQ-07（参考7-1） */
}

void inthandler2c(int *esp) {
    io_out8(PIC1_OCW2, 0x64); /* 通知PIC“IRQ-01已经受理完毕” */
    io_out8(PIC0_OCW2, 0x62); /* 通知PIC“IRQ-01已经受理完毕” */
    unsigned char data;
    data = io_in8(PORT_KEYDAT);
    fifo8_put(&mousefifo, data);
}
