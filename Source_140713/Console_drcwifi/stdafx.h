// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include "SerialPort.h"
#include <assert.h>
// �Ʒ� afx.h ����� TRACE()�� ����ϱ� ���� �߰���.
#define _AFXDLL
#include <afx.h>
#include <windows.h>

#include <winsock2.h>
#include "Console_drcwifi.h"

#define UART_TX_DATA_LEN	25		// ī�޶� ������ ������ ���� 

typedef signed char 	s8;
typedef unsigned char 	u8;
typedef unsigned short 	u16;
typedef signed short 	s16;
typedef unsigned int 	u32;

#pragma pack(1)

// ������ ��û�ϴ� ��� ī�޶�� �۽��ϴ� ������
typedef struct 
{
	u8 	HEAD[3];		// �� �ʿ���� 
	u32	TotalLen;		// �� �ʿ���� 
	u16	CmdType;		// �� �ʿ���� 
	u32	CmdDataLen;	 	// �� �ʿ���� 
	u32	UartTxLen;		// �� �ʿ���� 
	
	u8	aWidth;			// ��û�ϴ� ���󼼷�ũ��/8
	u8	aHeight;		// ��û�ϴ� ���󰡷�ũ��/8
	u8 	Quality;		// ��û�ϴ� ����ȭ�� 0~90 ���� 60
	u8	reserved1;		
	u8	reserved2;
	u8	reserved3;
	u8	reserved4;
	u8	reserved5;
	u8	reserved6;
	u8	reserved7;
	u8	reserved8;	
	BYTE UART_TX_DATA[UART_TX_DATA_LEN];// ī�޶� ������ ������  
}PC_TO_CAM;

// ī�޶�κ��� ���Ź��� ����������...������ ������ ���̾� ���ŵ�  
typedef struct 
{
	u8 	HEAD[3];		// �� �ʿ���� 
	u32	TotalLen;		// �� �ʿ���� 
	u16	AckType;		// �� �ʿ���� 
	u32	AckDataLen;		// �� �ʿ���� 	
	u32	UartRxLen;		// �� �ʿ���� 
	
	u8	iWidth;			// ���ŵ� ���󼼷�ũ��/8
	u8	iHeight;		// ���ŵ� ���󰡷�ũ��/8
	u32	reserved1;	
	u32	reserved2;		
	u32	iJpegLen;		// ���� ��ü�� ũ�� 
	u8	reserved3;	
	u8	reserved4;	
}CAM_TO_PC;

#pragma pack()

#pragma comment(lib,"opencv_video230d.lib")
#pragma comment(lib,"opencv_objdetect230d.lib")
#pragma comment(lib,"opencv_ml230d.lib")
#pragma comment(lib,"opencv_legacy230d.lib")
#pragma comment(lib,"opencv_imgproc230d.lib")
#pragma comment(lib,"opencv_highgui230d.lib")
#pragma comment(lib,"opencv_haartraining_engine.lib")
#pragma comment(lib,"opencv_gpu230d.lib")
#pragma comment(lib,"opencv_flann230d.lib")
#pragma comment(lib,"opencv_features2d230d.lib")
#pragma comment(lib,"opencv_core230d.lib")
#pragma comment(lib,"opencv_contrib230d.lib")
#pragma comment(lib,"opencv_calib3d230d.lib")

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#define TYPE_GET_REALTIME_IMG		1

// TODO: reference additional headers your program requires here
