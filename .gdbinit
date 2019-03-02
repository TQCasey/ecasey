##main function defined for initial the S3C2440 CPU.
target remote localhost:2331
monitor endian little  ##little endian, should be the same as your application.

##copied from JLinkGDBServer Document.
monitor reset            ##reset the S3C2440
monitor reg cpsr = 0xd3  ##setup cpsr register.
monitor speed auto       ##Link Speed.

##translated from VIVI S3C2440 version.
#disable watchdog
monitor MemU32 0x53000000 = 0

#disalbe interrupt --- int-mask register
monitor MemU32 0x4A000008 = 0xFFFFFFFF

#disalbe interrupt --- int-sub-mask register
monitor MemU32 0x4A00001C = 0x7FFF ## vivi set it as 0x7FF, why???

#initialize system clocks --- locktime register
monitor long 0x4C000000 = 0xFFFFFFFF

#initialize system clocks --- clock-divn register
monitor long 0x4C000014 = 0x5            #CLKDVIN_400_148

#initialize system clocks --- mpll register
monitor long 0x4C000004 = 0x5c011   #default clock

#setup memory controller
monitor MemU32 0x48000000 = 0x22111110    #conw
monitor MemU32 0x48000004 = 0x00000700    #bank0
monitor MemU32 0x48000008 = 0x00000700     #bank1
monitor MemU32 0x4800000c = 0x00000700     #bank2
monitor MemU32 0x48000010 = 0x00000700     #bank3
monitor MemU32 0x48000014 = 0x00000700     #bank4
monitor MemU32 0x48000018 = 0x00000700     #bank5
monitor MemU32 0x4800001c = 0x00018009     #bank6
monitor MemU32 0x48000020 = 0x00018009     #bank7
monitor MemU32 0x48000024 = 0x008e04eb     #vREFRESH
monitor MemU32 0x48000028 = 0xB2        #vBANKSIZE -- 128M/128M --- should
monitor MemU32 0x4800002c = 0x30        #vMRSRB6
monitor MemU32 0x48000030 = 0x30        #vMRSRB7       
