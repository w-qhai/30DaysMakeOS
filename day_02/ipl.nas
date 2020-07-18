; hello-os
; TAB=4

		ORG		0x7c00			; 指名程序的装在地址

; 以下的技?用于?准的FAT12格式的??

		JMP		entry
		DB		0x90
		DB		"HELLOIPL"		; ??扇区名称（8字?）
		DW		512				; ?个扇区（sector）大小（必?512字?）
		DB		1				; 簇（cluster）大小（必??1个扇区）
		DW		1				; FAT起始位置（一般?第一个扇区）
		DB		2				; FAT个数（必??2）
		DW		224				; 根目?大小（一般?224?）
		DW		2880			; ?磁?大小（必??2880扇区1440*1024/512）
		DB		0xf0			; 磁??型（必??0xf0）
		DW		9				; FAT的?度（必??9扇区）
		DW		18				; 一个磁道（track）有几个扇区（必??18）
		DW		2				; 磁?数（必??2）
		DD		0				; 不使用分区，必?是0
		DD		2880			; 重写一次磁?大小
		DB		0,0,0x29		; 意?不明（固定）
		DD		0xffffffff		; （可能是）卷?号?
		DB		"HELLO-OS   "	; 磁?的名称（必??11字?，不足填空格）
		DB		"FAT12   "		; 磁?格式名称（必??8字?，不足填空格）
		RESB	18				; 先空出18字?

; プログラム本体

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX
		MOV		ES,AX

		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; ?SI加1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; ?示一个文字
		MOV		BX,15			; 指定字符?色
		INT		0x10			; ?用??BIOS
		JMP		putloop
fin:
		HLT						; ?CPU停止，等待指令
		JMP		fin				; 无限循?

msg:
		DB		0x0a, 0x0a		; ?行2次
		DB		"hello, world"
		DB		0x0a			; ?行
		DB		0

		RESB	0x7dfe-$		; 填写0x00直到0x001fe

		DB		0x55, 0xaa