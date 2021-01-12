#ifndef _BOARD_H
#define _BOARD_H

#include "stm32f10x_sys.h"

/***********����������*************/
#define LCD
#define	LED
#define LED11
#define LED22
// #define LED33
// #define LED44
// #define	KEY
// #define 	KEY11
// #define	NAV
// #define	BELL
/**********************************/

//������
#define BEEP PAout(8)

//LED
#define	LED1 PCout(4)
#define	LED2 PCout(5)
#define	LED3 PCout(6)
#define	LED4 PCout(7)

//����KEY
#define KEY1 PCin(0)
#define KEY2 PCin(1)
#define KEY3 PCin(2)
#define KEY4 PCin(3)

//���򿪹�
#define Nav_UP			PAin(0)
#define Nav_DOWN		PAin(1)
#define Nav_LEFT		PAin(4)
#define Nav_RIGHT		PAin(3)
#define Nav_CONTER	PAin(2)

typedef enum 
{
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3,
	CONTER = 4,
	_uKEY1 = 5,
	_uKEY2 = 6,
	_uKEY3 = 7,
	_uKEY4 = 8
}KeyState;

//����SPIƬѡ
#define M25P_CS		PAout(11)
#define CH376_CS	PAout(12)
#define TP_CS			PCout(14)	

void Board_init(void);//������������

uint8_t Key_check(void);////������⺯�����м�����ʱ�÷���1���޼����·���0
KeyState Key_event(void);//ȡ��ֵ������������ֵͨ��key_value����

#endif
