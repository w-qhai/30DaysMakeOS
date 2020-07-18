; haribote-ipl
; TAB=4

CYLS 	EQU 	10				; #define CYLS = 10

		ORG		0x7c00			; 指名程序的装在地址

; 以下的技术用于标准的FAT12格式的软盘

		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; 启动扇区名称（8字节）
		DW		512				; 每个扇区（sector）大小（必须512字节）
		DB		1				; 簇（cluster）大小（必须为1个扇区）
		DW		1				; FAT起始位置（一般为第一个扇区）
		DB		2				; FAT个数（必须为2）
		DW		224				; 根目录大小（一般为224项）
		DW		2880			; 该磁盘大小（必须为2880扇区1440*1024/512）
		DB		0xf0			; 磁盘类型（必须为0xf0）
		DW		9				; FAT的长度（必??9扇区）
		DW		18				; 一个磁道（track）有几个扇区（必须为18）
		DW		2				; 磁头数（必??2）
		DD		0				; 不使用分区，必须是0
		DD		2880			; 重写一次磁盘大小
		DB		0,0,0x29		; 意义不明（固定）
		DD		0xffffffff		; （可能是）卷标号码
		DB		"HARIBOTEOS "	; 磁盘的名称（必须为11字?，不足填空格）
		DB		"FAT12   "		; 磁盘格式名称（必??8字?，不足填空格）
		RESB	18				; 先空出18字节

; 程序本体

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

		; 读取磁盘

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; 柱面0
		MOV		DH,0			; 磁头0
		MOV		CL,2			; 扇区2
readloop:
		MOV		SI,0			; 记录失败次数的寄存器
retry:		
		MOV		AH,0x02			; AH=0x02 : 读入磁盘 INT 0x13 的参数 及调用
		MOV		AL,1			; 1个扇区
		MOV		BX,0
		MOV		DL,0x00			; A驱动器
		INT		0x13			; 调用磁盘BIOS例程
		JNC		next			; 进位标志（AH）为 0 时跳转 "jump if not carry"
		ADD		SI,1			; 失败次数加1
		CMP		SI,5			
		JAE		error			; SI>=5 跳转到error	"jump if above or equal"
		MOV		AH, 0x00		; 恢复状态再读一次
		MOV		DL, 0x00		; A驱动器
		INT		0x13
		JMP		retry			; 循环
next:
		MOV		AX,ES			; 把内存地址后移 0x200
		ADD 	AX,0x0020
		MOV 	ES,AX
		ADD 	CL,1
		CMP 	CL,18
		JBE		readloop		; CL <= 18 跳转到readloop "jump if below or equal"
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop		; DH < 2 跳到readloop "jump if below"
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS			
		JB		readloop		; CH < 2 跳到readloop "jump if below"	


		
		MOV		[0x0ff0],CH		; 柱面数量写入内存，后面程序可以使用
		JMP		0xc200			; 执行完毕 跳转到操作系统开始地址

error:
		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; 给SI加1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 显示一个文字
		MOV		BX,15			; 指定字符颜色
		INT		0x10			; 调用显卡BIOS
		JMP		putloop
fin:
		HLT						; 让CPU停止，等待指令
		JMP		fin				; 无限循环
msg:
		DB		0x0a, 0x0a		; 换行两次
		DB		"load error"
		DB		0x0a			; 换行
		DB		0

		RESB	0x7dfe-$		; 填写0x00直到0x001fe

		DB		0x55, 0xaa
