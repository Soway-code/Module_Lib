#                        CAN_boot和上位机
首次升级时使用随便一个ID,首次升级ID根据使用第一条指令的ID,后面升级设置好boot里面的CANID_ADDR宏地址后升级CAN-ID根据APP的CAN-ID,
首次波特率使用250K,设置好boot里面的BAUDRATE宏地址后升级波特率根据APP的波特率一样,

//boot 使用内存0x8000000 - 0x08001490
//	0x8003000  1个字节存放升级标志位  0x00已经升级有app,   
//  0x8003001- 0x8003FFF   用来存放APP参数数据
//  #define CANID_ADDR					0x0800305B		//*四个字节存放APP的CANid存放地址,需要根据app修改
/* BAUDRATE APP的波特率 1Mbps 		= 9
				500Kbps		= 8
				125Kbps 	= 6
				100Kbps 	= 5
				50Kbps 		= 4
				20Kbps 		= 3
				10Kbps 		= 2
				5Kbps 		= 1
				250Kbps 	= 其他数值 
*/
//#define	BAUDRATE					0x0800305F		//*一个字节存放APP波特率存放地址,需要根据app修改
//#define SOWAYAPP_START_ADDR         	0x08004000 		//APP启始地址



####                                                                  -----代码维护人：马灿林








