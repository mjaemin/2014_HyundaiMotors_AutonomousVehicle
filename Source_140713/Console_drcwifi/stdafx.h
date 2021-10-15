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
// 아래 afx.h 헤더는 TRACE()를 사용하기 위해 추가함.
#define _AFXDLL
#include <afx.h>
#include <windows.h>

#include <winsock2.h>
#include "Console_drcwifi.h"

#define UART_TX_DATA_LEN	25		// 카메라에 보내는 데이터 길이 

typedef signed char 	s8;
typedef unsigned char 	u8;
typedef unsigned short 	u16;
typedef signed short 	s16;
typedef unsigned int 	u32;

#pragma pack(1)

// 영상을 요청하는 경우 카메라로 송신하는 데이터
typedef struct 
{
	u8 	HEAD[3];		// 알 필요없음 
	u32	TotalLen;		// 알 필요없음 
	u16	CmdType;		// 알 필요없음 
	u32	CmdDataLen;	 	// 알 필요없음 
	u32	UartTxLen;		// 알 필요없음 
	
	u8	aWidth;			// 요청하는 영상세로크기/8
	u8	aHeight;		// 요청하는 영상가로크기/8
	u8 	Quality;		// 요청하는 영상화질 0~90 보통 60
	u8	reserved1;		
	u8	reserved2;
	u8	reserved3;
	u8	reserved4;
	u8	reserved5;
	u8	reserved6;
	u8	reserved7;
	u8	reserved8;	
	BYTE UART_TX_DATA[UART_TX_DATA_LEN];// 카메라에 보내는 데이터  
}PC_TO_CAM;

// 카메라로부터 수신받은 정보데이터...영상은 정보에 연이어 수신됨  
typedef struct 
{
	u8 	HEAD[3];		// 알 필요없음 
	u32	TotalLen;		// 알 필요없음 
	u16	AckType;		// 알 필요없음 
	u32	AckDataLen;		// 알 필요없음 	
	u32	UartRxLen;		// 알 필요없음 
	
	u8	iWidth;			// 수신된 영상세로크기/8
	u8	iHeight;		// 수신된 영상가로크기/8
	u32	reserved1;	
	u32	reserved2;		
	u32	iJpegLen;		// 영상 자체의 크기 
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
