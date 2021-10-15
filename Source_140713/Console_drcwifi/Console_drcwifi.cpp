// Console_drcwifi.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <process.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>

#pragma warning (disable:4996)

// 소켓관련 변수들 
WSADATA wsaData;
SOCKADDR_IN servAddr;
SOCKET hSocket;

// CSerialPort 클래스의 com1 객체 생성
CSerialPort com1;

PC_TO_CAM 	gPC;	// PC에서 CAM으로 보내는 데이터 
CAM_TO_PC 	gCAM;	// CAM에서 PC로 보내는 데이터 

#if 0
	int IMG_W	= 320;		// 영상의 가로폭 
	int IMG_H	= 240;		// 영상의 세로폭
	int IMGQuality = 50;
#else
	// 스레드 없이 VGA사용시 속도저하됨 . VGA사용시 스레드 사용 권장 
	int IMG_W	= 640;		// 영상의 가로폭 
	int IMG_H	= 480;		// 영상의 세로폭 
	int IMGQuality = 50;
#endif

BYTE mbuff[640*480*2] ;	//영상 수신용 버퍼 

//로봇용 
//int JOG_DIR_1, JOG_LEN_1;
//int JOG_DIR_2, JOG_LEN_2;
//int JOG_BTN1;

//정수형을 아스키형태로 변환하는 함수  
void IntToASCII(int val , u8 *p)
{

	if(val>=0) 	{p[0] = '+';		}
	else 	 	{p[0] = '-';	val*=-1;}
	
	p[1] = '0' + (u8)(val/100);
	
	int remain = val - (val/100)*100;
	p[2] = '0' + (u8)(remain/10);

	remain = remain - (remain/10)*10;
	p[3] = '0' + (u8)(remain);
}

//관련 변수 초기화 및 카메라로 소켓 연결 
int DRC_InitSocket()
{
	//JOG_DIR_1 = JOG_DIR_2 = JOG_LEN_1 = JOG_LEN_2 = JOG_BTN1 = 0;
	
	// 송수신 패킷 자료구조 초기화 
	memset((u8 *)&gPC, 	0, sizeof(PC_TO_CAM));
	memset((u8 *)&gCAM, 	0, sizeof(CAM_TO_PC));
	
	// 소켓 초기화 ..우린 클라이언트로 동작한다.
	if(WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
	{
		printf("	WSAStartup Error \n");
		return 0;
	}
	else
		printf("	Winsock Init Ok\n");	

	hSocket = socket(PF_INET, SOCK_STREAM,0); // TCP
	if(hSocket==INVALID_SOCKET)
	{
		printf("	socket Error \n");
		return 0;
	}
	else
		printf("	socket Create \n");
	
	memset(&servAddr,0,sizeof(servAddr));

	servAddr.sin_family 		= AF_INET;
	servAddr.sin_addr.s_addr	= inet_addr("192.168.123.10");	// 카메라의 IP주소 ...고정
	servAddr.sin_port		= htons(6400); 			// 카메라의 포트번호 
	
	// 카메라로의 연결을 시도한다 
	if(connect(hSocket, (SOCKADDR *)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		printf("	connect Error \n");
		return 0;
	}
	else 
	{
		printf("	connect OK \n");
		printf("\n----------------------------------------------------------\n\n");
	}
	
	return 1;
}


//카메라에게 영상을 요청한다 
void DRC_RequestImage()
{
	//카메라에게 송신할 데이터를 작성함 
	gPC.HEAD[0]	= 0x55;				// 일단 헤더 3바이트 작성 
	gPC.HEAD[1]	= 0x33;
	gPC.HEAD[2]	= 0xAA;
	gPC.CmdType 	= TYPE_GET_REALTIME_IMG;	// 알필요 없음 
	gPC.CmdDataLen	= 11; 				// 알필요 없음 	
	gPC.UartTxLen	= UART_TX_DATA_LEN;		// DRC-WIFI 에 전송할 UART TX 의 크기를 25byte 로 임의 설정(Max 100byte)
	gPC.TotalLen	= sizeof(PC_TO_CAM);		// 알필요 없음 		
	gPC.aWidth 	= IMG_W/8;			// 수신 받기원하는 영상의 크기를 320x240 으로 설정(Max 640x480)
	gPC.aHeight	= IMG_H/8;
	gPC.Quality	= IMGQuality;			// 영상의 이미지 퀄리티를 설정 (1~90, 보통 60) 고화질일수록 속도 저하됨 
	
	/*
	if(UART_TX_DATA_LEN > 0)
	{		//카메라의 시리얼로 전달할 데이터가 존재하면 UART TX작성
		// 프로토콜을 매뉴얼 참조 요망 
		gPC.UART_TX_DATA[0]		= 'V';	// company code
		gPC.UART_TX_DATA[1]		= 'J';	// GUI type 0	: Jog shuttle	
		gPC.UART_TX_DATA[2]		= 'A';	// GUI type 1 : A series
		gPC.UART_TX_DATA[3]		= ' ';	
		IntToASCII(JOG_DIR_1, 	&gPC.UART_TX_DATA[4]); 	
		IntToASCII(JOG_LEN_1, 	&gPC.UART_TX_DATA[8]); 
		gPC.UART_TX_DATA[12]		= ' ';		
		IntToASCII(JOG_DIR_2, 	&gPC.UART_TX_DATA[13]); 	
		IntToASCII(JOG_LEN_2, 	&gPC.UART_TX_DATA[17]); 
		gPC.UART_TX_DATA[21]		= ' ';
		gPC.UART_TX_DATA[22]		= '0'+(u8)(JOG_BTN1);		
		gPC.UART_TX_DATA[23]		= '\r';
		gPC.UART_TX_DATA[24]		= '\n';
	}
	*/

	char *p = (char *)&gPC;    // 편리한 전송을 위해 
	
	int Sumlen =0;	           // 송신 누적 길이 관리용
	
	while(Sumlen<gPC.TotalLen) // 요청 데이터를 카메라로 송신한다
	{ 	 
		int TempLen = send(hSocket, &p[Sumlen], gPC.TotalLen-Sumlen, 0 ); 		
		Sumlen += TempLen; 
		//printf("_TX:%2d", TempLen);
	}
}

//카메라로 부터 영상을 수신 한다 
int DRC_GetImage()
{
	int Sumlen =0;			// 수신 누적 길이 관리용
	gCAM.TotalLen = 1000000;	//카메라가 보내는 데이터의 길이를 알수는 없으나...일단 충분히 큰수로...
	
	while(Sumlen<gCAM.TotalLen)
	{
		int TempLen = recv(hSocket,(char *)&mbuff[Sumlen], gCAM.TotalLen-Sumlen, 0 );

		//무언가 수신햇다면 진입 
		if(TempLen>0)
		{
			//printf(" _RX:%5d", TempLen);
			//처음 수신햇다면 진입 
			if(Sumlen==0)
			{
				if(mbuff[0] != 0x55 || mbuff[1] != 0x33 || mbuff[2] != 0xAA)
				{
					printf("\r\nNot MATCH HEADER  0x%02X	0x%02X	0x%02X \r\n",mbuff[0],mbuff[1],mbuff[2]);
					break;
				}
				
				memcpy((BYTE *)&gCAM, mbuff,  sizeof(CAM_TO_PC));
				//printf("good head...%d, TotalLen=%d\n",TempLen, CAM_TO_PC.TotalLen);
			}
			Sumlen += TempLen; 
		}
	}

	//영상을 모두 수신하면 이곳으로 ...
	u8 *p = &mbuff[sizeof(CAM_TO_PC)];
	if(p[0]==0xFF && p[1]==0xD8  )
	{
		return 1;
	}
	else
	{
		printf("ERROR JPEG HEADER\n");
		return 0 ;
	}
	
}

void on_mouseEvent (int event, int x, int y, int flags, void* param)
{
	IplImage    *image;
	image = (IplImage *)param;
	
	double	R = 0, G = 0, B = 0;
	double	Y = 0, Cb = 0, Cr = 0;

	CvScalar ColorData;

	switch(event)
	{
 		case CV_EVENT_LBUTTONDOWN:

			ColorData = cvGet2D(image, y, x);
			
			B = ColorData.val[0];
			G = ColorData.val[1];
			R = ColorData.val[2];	

			Y  = ((0.299 * R) + (0.587 * G) + (0.114 * B));
			Cb = (0.5643 * (B-Y) + 128);
			Cr = (0.7132 * (R-Y) + 128);

			printf("x = %3d, y = %3d\n\n", x, y);
			printf("B = %5f, G = %5f, R = %5f\n\n", B, G, R);
			printf("Y = %5f, Cb = %5f, Cr = %5f\n\n", Y, Cb, Cr);
			
			break;
	}
}


int detectflg=0;

// FB-155BC (Bluetooth Module) Connect 명령어
// RC 모형 자동차의 Bluetooth Device ID : 00190137FD70
char CONNECT[17] = "ATD00190137FD0A";

// FB-155BC (Bluetooth Module) Reset 명령어
char init[5] = "ATZ";


UINT WINAPI SerialInterface( void* arg )
{
	int i = 0;    
	char Inbuff[1024] = "";
	char * ptr = NULL;

	CONNECT[15] = 0xd;
	init[3] = 0xd; 
	
	// 시리얼 수신
	int n = com1.Read (Inbuff, 1024);
	Sleep (100);

	if(n > 0)
	{
		
		printf ("%s", Inbuff);
			
		/*
		printf ("%d\n", Inbuff[0]);
		printf ("%d\n", Inbuff[1]);

		for(i=2; i<n-2; i++)
		{
			printf ("%c\n", Inbuff[i]);
		}

		printf ("%d\n", Inbuff[n-2]);
		printf ("%d\n\n", Inbuff[n-1]);
		*/
		
		// Bluetooth Device Connect Request	
		ptr = strstr(Inbuff, "OK");
		if(ptr != NULL && detectflg == 1)
		{
			detectflg = 0;

			com1.Write (CONNECT, strlen(CONNECT));
			ptr = NULL;
		}
		
		// Bluetooth Device ID Detect
		ptr = strstr(Inbuff, "00190137FD0A,FB155");
		if(ptr != NULL)
		{
			detectflg = 1;
			ptr = NULL;
		}
		
		// Bluetooth Command Error -> Init	
		ptr = strstr(Inbuff, "ERROR");
		if(ptr != NULL)
		{
			printf ("init\n");
			com1.Write (init, strlen(init));
			ptr = NULL;
		}

		for(i=0; i<n; i++)
			Inbuff[i] = 0;

		n = 0;
	}

	_endthreadex(0);
    return 0;
}

// 임의로 마스크 지정
void mask_field (IplImage *mask, IplImage *roiImage)
{
	CvPoint pt1, pt2;
		
	pt1.x = roiImage->width / 2;
	pt1.y = 0;

	pt2.x = 0;
	pt2.y = roiImage->height / 4;

	cvLine(mask, pt1, pt2, cvScalar(0), 2, 8 );

	pt1.x = 0;
	pt1.y = 0;

	pt2.x = 0;
	pt2.y = roiImage->height / 4;

	cvLine(mask, pt1, pt2, cvScalar(0), 2, 8 );

	pt1.x = roiImage->width;
	pt1.y = 0;

	pt2.x = roiImage->width;
	pt2.y = roiImage->height / 4;

	cvLine(mask, pt1, pt2, cvScalar(0), 2, 8 );

	pt1.x = 0;
	pt1.y = 0;

	pt2.x = roiImage->width;
	pt2.y = 0;

	cvLine(mask, pt1, pt2, cvScalar(0), 2, 8 );

	pt1.x = roiImage->width / 2;
	pt1.y = 0;

	pt2.x = roiImage->width;
	pt2.y = roiImage->height / 4;

	cvLine(mask, pt1, pt2, cvScalar(0), 2, 8 );


	CvScalar lo_diff   = cvScalarAll(10);
	CvScalar up_diff   = cvScalarAll(10);
	CvConnectedComp comp;
	int floodFlags=4 | CV_FLOODFILL_FIXED_RANGE;
		
	cvFloodFill(mask,  cvPoint(5, 5),  cvScalar(0), lo_diff, up_diff, &comp, floodFlags);
	cvFloodFill(mask,  cvPoint(roiImage->width-5, 5),  cvScalar(0), lo_diff, up_diff, &comp, floodFlags);
}

int _tmain(int argc, _TCHAR* argv[])
{
	// 시리얼 통신을 위한 쓰레드 변수들
	HANDLE hThread;
    UINT uThreadID[1];
	uThreadID[0] = 0;
	
	// Port, Baud rate, Data bit, Stop bit, Parity 설정  
	com1.Open ("COM3", CBR_9600, 8, ONESTOPBIT, NOPARITY);
	com1.SetTimeout (10, 10, 1);

	int R = 0, G = 0, B = 0;
	int Y = 0, Cb = 0, Cr = 0;
	int x = 0, y = 0, row = 0, col = 0;
	int roi_col = 0;


	double edge_darkness = 0, Lane_darkness = 0, mask_darkness = 0;
	
	if(DRC_InitSocket()==0)			//관련 변수 초기화 및 카메라로 소켓 연결 
		return 0;

	// FB-155BC (Bluetooth Module) Device Search 명령어
	char Outbuff[11] = "AT+BTINQ?";
	Outbuff[9] = 0xd;

	// 시리얼 송신
	com1.Write (Outbuff, strlen(Outbuff));

	while(1)
	{	
		// 시리얼 통신을 위한 쓰레드 생성
		hThread = (HANDLE)_beginthreadex( NULL, 0, SerialInterface, NULL, 0, &uThreadID[0] );
		///////////////

		//////////////
		DRC_RequestImage();		//카메라에게 영상을 요청한다 
		int ret = DRC_GetImage();	//카메라로 부터 영상을 수신 한다 
		if(ret == 0)
		{
			printf("Wrong Image\n");
			continue;
		}
		
		CvMat *buf = cvCreateMat(1, IMG_W*IMG_H*2, CV_8UC1 );  

		memcpy((u8 *)(buf->data.ptr), &mbuff[sizeof(CAM_TO_PC)], gCAM.iJpegLen); 	// 수신한 영상을 복사한다 	
		
		// 원본 영상 (srcImage)
		IplImage *srcImage = cvDecodeImage(buf, 1);					// 압축된 영상이므로 압축을 풀어준다 
		
		// TODO:  여기서 필요한 영상처리를 수행한다.
		
		// 영상 처리후에 결과 영상을 만들기 위하여 영상 구조체 변수 dstImage를 생성 - 3채널 (컬러 영상)
		IplImage* dstImage = cvCreateImage( cvGetSize(srcImage), IPL_DEPTH_8U, 3);
		
		// 원본 영상(srcImage)을 dstImage에 복사
		cvCopy(srcImage, dstImage, 0);
			
		// 신호등을 감지하기 위하여 관심영역을 사각형으로 표현
		//cvRectangle(dstImage, cvPoint(0, 0), cvPoint(srcImage->width-1, (srcImage->height / 2)-1), CV_RGB(0, 255, 0), 2);
		
		// 차선을 감지하기 위하여 관심영역을 사각형으로 표현
		//cvRectangle(dstImage, cvPoint(0, srcImage->height/2), cvPoint(srcImage->width-1, srcImage->height-1), CV_RGB(0, 0, 255), 2);
		
		// 차선을 감지하기 위한 관심 영역(roi)을 지정
		CvRect roi = cvRect ( 0, (dstImage->height / 2), dstImage->width, (dstImage->height / 2) );
		
		// dstImage 영상에 관심영역(roi)을 지정
		cvSetImageROI (dstImage, roi);
		
		// 관심 영역(roi)의 영상 크기 만큼 영상 구조체 변수 roiImage를 생성 - 3채널 (컬러 영상)
		IplImage* roiImage = cvCreateImage( cvSize(roi.width, roi.height), IPL_DEPTH_8U, 3);
		
		// dstImage의 관심영역(roi)을 roiImage에 복사
		cvCopy (dstImage, roiImage);
		
		// dstImage의 관심영역(roi)을 해제
		cvResetImageROI (dstImage);
		
		// 0 ~ 255 그래이 스케일로 변환 (자료형 : IPL_DEPTH_8U)
		// roiImage의 영상 크기 만큼 영상 구조체 변수 grayImage 생성 - 1채널 (Gray 영상)
  		IplImage *grayImage = cvCreateImage( cvGetSize(roiImage), IPL_DEPTH_8U, 1);
  		// 3채널(컬러) 영상인 roiImage를 1채널(GRAY) 영상으로 변환하여 grayImage에 복사
		cvCvtColor( roiImage, grayImage, CV_BGR2GRAY );
		
		// 0. ~ 1. 그래이 스케일로 변환 (자료형 : IPL_DEPTH_32F)
		IplImage *img_32f = cvCreateImage( cvGetSize(grayImage), IPL_DEPTH_32F, 1 );
		// grayImage의 영상 데이터에 255를 나누어 img_32f에 저장 (0. ~ 1. 그래이 스케일로 변환)
		cvConvertScale(grayImage, img_32f, 1.0 / 255.0, 0);
		// 필터링 함수 (cvSmooth)
		cvSmooth(img_32f, img_32f, CV_GAUSSIAN, 5);

		// Sobel 연산자로 이미지를 x, y 방향으로 미분 즉, edge 검출.
		IplImage *diff_x = cvCreateImage(cvGetSize(img_32f), IPL_DEPTH_32F, 1);
		IplImage *diff_y = cvCreateImage(cvGetSize(img_32f), IPL_DEPTH_32F, 1);
		cvSobel(img_32f, diff_x, 1, 0, 3); // img_32f를 x축으로 미분하여 검출된 데이터를 diff_x 저장
		cvSobel(img_32f, diff_y, 0, 1, 3); // img_32f를 y축으로 미분하여 검출된 데이터를 diff_y 저장

		// Edge의 magnitude와 orientation을 계산
		IplImage *mag = cvCreateImage(cvGetSize(img_32f), IPL_DEPTH_32F, 1);
		IplImage *ori = cvCreateImage(cvGetSize(img_32f), IPL_DEPTH_32F, 1);
		// (diff_x, diff_y)의 직교 죄표의 데이터를 극좌표 (mag, ori)로 변환하여 저장
		cvCartToPolar(diff_x, diff_y, mag, ori, 1);
		
		// edgeImage는 cvCanny 함수를 이용하여 Edge를 구현
		IplImage *edgeImage = cvCreateImage(cvGetSize(roiImage), IPL_DEPTH_8U, 1);
		cvCanny(grayImage, edgeImage, 50, 200, 3);
		
		// 신호등 색을 검출하기 위하여 Red, Yellow, Green 구조체 변수를 선언
		ColorDetect Red, Yellow, Green, Blue, White, White_S;
		CvScalar ColorData;

		Red.Count = 1;		// Red 검출된 화소 수
		Red.DrawX = 0;          // Red 검출된 x 좌표
		Red.DrawY = 0;          // Red 검출된 y 좌표
		
		Yellow.Count = 1; 	// Yellow 검출된 화소 수
		Yellow.DrawX = 0; 	// Yellow 검출된 x 좌표
		Yellow.DrawY = 0;	// Yellow 검출된 y 좌표
		
		Green.Count = 1;	// Green 검출된 화소 수
		Green.DrawX = 0; 	// Green 검출된 x 좌표
		Green.DrawY = 0;	// Green 검출된 y 좌표
///////////////

		Blue.Count = 1;      // Blue 검출된 화소 수
		Blue.DrawX = 0;    // Blue 검출된 x 좌표
		Blue.DrawY = 0;      // Blue 검출된 y 좌표

		White.Count = 1;
 		White.DrawX = 0;
		White.DrawY = 0;
      
		White_S.Count = 1;
		White_S.DrawX = 0;
		White_S.DrawY = 0;
////////////
		
		
	//	com1.Write("R",1);
	//	com1.Write("V",1);
   	////////
	char Red_Flag;
	char Yellow_Flag;
	char Blue_Flag;
	char Cross_Flag;
	char Stop_Flag;
	char Parking_Flag;

	char Parking_Cplt_Flag=0;
	char Cross_Cplt_Flag=0;
	////////////////////
		cvPow(img_32f, img_32f, 2);	// img_32f의 명암을 강조하기 위하여 제곱을 곱함
		
		// img_32f의 영상에 마우스 커서로 영상의 색(데이터)을 확인하기 위해 on_mouseEvent함수를 호출
		cvSetMouseCallback("srcImage", on_mouseEvent,  (void *)srcImage);

		IplImage *LanegrayImage = cvCreateImage( cvGetSize(roiImage), IPL_DEPTH_8U, 1 );
		// LanegrayImage의 영상을 White로 만듬
		cvSet(LanegrayImage, cvScalar(255));
		
		IplImage* mask = cvCreateImage( cvGetSize(roiImage), IPL_DEPTH_8U, 1);
		// mask 영상을 White로 만듬
		cvSet(mask, cvScalar(255));
     
 	/////////////////////
      Red_Flag = 0;
      Yellow_Flag = 0;
      Blue_Flag = 0;
      Cross_Flag = 0;
      Stop_Flag = 0;
      ////////////////////

		mask_field(mask, roiImage); // 마스크 지정 함수

		for(y=0; y<dstImage->height/2; y+=40)      // Width x Height 해상도에서 Height/2 부분만 색 검출 
		{
			for(x=0; x<dstImage->width; x+=40)
			{
				// 40 x 40 영역으로 원하는 색을 찾음.
				for(col=0; col<40; col++)
				{
					for(row=0; row<40; row++)
					{
						// dstImage 이미지의 (x, y) 데이터를 CvScalar 구조체 변수 ColorData에 저장 (3채널 영상)
						ColorData = cvGet2D(srcImage, y+col, x+row);
						B = cvRound(ColorData.val[0]); // B 채널
						G = cvRound(ColorData.val[1]); // G 채널
						R = cvRound(ColorData.val[2]); // R 채널	
					
						// RGB를 YCbCr로 변환
						Y  = cvRound((0.299 * R) + (0.587 * G) + (0.114 * B));
						Cb = cvRound(0.5643 * (B-Y) + 128);
						Cr = cvRound(0.7132 * (R-Y) + 128);
				
						if( (105 < Cb && Cb < 140) && (50 < Cr && Cr < 90) ) { //초록색
							Green.Count++;
							Green.DrawX += x+row;
							Green.DrawY += y+col;
						}	
				
						if( (20 <Cb && Cb < 70) && (130 < Cr && Cr < 150) ) {	//노란색
							Yellow.Count++;
							Yellow.DrawX += x+row;
							Yellow.DrawY += y+col;
						}
						
						if( (90 <Cb && Cb < 120) && (190 < Cr && Cr < 215) ) { //붉은색	
							Red.Count++;
							Red.DrawX += x+row;
							Red.DrawY += y+col;
						}
						
						if( (180 <Cb && Cb < 230) && (90 < Cr && Cr < 130) ) { //붉은색	
							Blue.Count++;
							Blue.DrawX += x+row;
							Blue.DrawY += y+col;
						}
					}
				}

				// Red, Yellow, Green 색이 각각 검출된 (x, y) 좌표의 평균을 구함
				Red.DrawX /= Red.Count;
				Red.DrawY /= Red.Count;

				Yellow.DrawX /= Yellow.Count;
				Yellow.DrawY /= Yellow.Count;

				Green.DrawX /= Green.Count;
				Green.DrawY /= Green.Count;

				Blue.DrawX /= Blue.Count;
				Blue.DrawY /= Blue.Count;
				
				// 평균(x, y) 좌표에 원을 그림 (신호등 색 검출)
				if(Red.Count > 100)
				{
					cvCircle(dstImage, cvPoint(Red.DrawX, Red.DrawY), 7, CV_RGB(255,0,0), 2);
              		 Red_Flag = 1;
					//cvRectangle(dstImage, cvPoint(x, y), cvPoint(x+row, y+col), CV_RGB(255, 0, 0), 2);
				}

				if(Yellow.Count > 100)
				{
					cvCircle(dstImage, cvPoint(Yellow.DrawX, Yellow.DrawY), 7, CV_RGB(255,255,0), 2);
              		Yellow_Flag = 1;
					//cvRectangle(dstImage, cvPoint(x, y), cvPoint(x+row, y+col), CV_RGB(255, 255, 0), 2);
				}

				if(Green.Count > 80)
				{
					cvCircle(dstImage, cvPoint(Green.DrawX, Green.DrawY), 7, CV_RGB(0,255,0), 2);
               		Blue_Flag = 1;
					//cvRectangle(dstImage, cvPoint(x, y), cvPoint(x+row, y+col), CV_RGB(0, 255, 0), 2);
				}

				if(Blue.Count > 120)
				{
					cvCircle(dstImage, cvPoint(Blue.DrawX, Blue.DrawY), 7, CV_RGB(0,255,0), 2);
					//cvRectangle(dstImage, cvPoint(x, y), cvPoint(x+row, y+col), CV_RGB(0, 255, 0), 2);
 					Parking_Flag = 1;
				}
		
				Red.Count = 1;
				Yellow.Count = 1;
				Green.Count = 1;
				Blue.Count = 1;

			}
		}



///////////////////////////////////////////////////////////////////////White detect
      if(Cross_Cplt_Flag==0)
      {
         for(y=dstImage->height/2; y<dstImage->height; y+=40)      // Width x Height 해상도에서 Height/2 부분만 색 검출 
         {
            for(x=0; x<dstImage->width; x+=40)
            {
               // 40 x 40 영역으로 원하는 색을 찾음.
               for(col=0; col<40; col++)
               {
                  for(row=0; row<40; row++)
                  {
                     // dstImage 이미지의 (x, y) 데이터를 CvScalar 구조체 변수 ColorData에 저장 (3채널 영상)
                     ColorData = cvGet2D(srcImage, y+col, x+row);
                     B = cvRound(ColorData.val[0]); // B 채널
                     G = cvRound(ColorData.val[1]); // G 채널
                     R = cvRound(ColorData.val[2]); // R 채널   
               
                     // RGB를 YCbCr로 변환
                     Y  = cvRound((0.299 * R) + (0.587 * G) + (0.114 * B));
            
                     if(220 < Y) { //초록색
                        White.Count++;
                        White.DrawX += x+row;
                        White.DrawY += y+col;
                     }
                  }
               }

               // Red, Yellow, Green 색이 각각 검출된 (x, y) 좌표의 평균을 구함
               White.DrawX /= White.Count;
               White.DrawY /= White.Count;
            
               // 평균(x, y) 좌표에 원을 그림 (신호등 색 검출)
               if(White.Count > 800)//값은 수정해봐야
               {
                  cvCircle(dstImage, cvPoint(White.DrawX, White.DrawY), 7, CV_RGB(255,255,255), 2);
                  //출력 G
                  Cross_Flag = 1;
                  //cvRectangle(dstImage, cvPoint(x, y), cvPoint(x+row, y+col), CV_RGB(255, 0, 0), 2);
               }
      
               White.Count = 1;
            }
         }   
      }
/////////////STOP LINE DETECT//////////////////////////////////////////////////////////
      ////일정한 y height 에서 (벡터) 흰색 점의 갯수(비율)을 따져서 이상이면 정지 방법도?
      for(y=dstImage->height/2; y<=dstImage->height*0.75; y+=40)      // Width x Height 해상도에서 Height/2 부분만 색 검출 
      {
         for(x=0; x<dstImage->width; x+=40)
         {
            // 40 x 40 영역으로 원하는 색을 찾음.
            for(col=0; col<40; col++)
            {
               for(row=0; row<40; row++)
               {
                  // dstImage 이미지의 (x, y) 데이터를 CvScalar 구조체 변수 ColorData에 저장 (3채널 영상)
                  ColorData = cvGet2D(srcImage, y+col, x+row);
                  B = cvRound(ColorData.val[0]); // B 채널
                  G = cvRound(ColorData.val[1]); // G 채널
                  R = cvRound(ColorData.val[2]); // R 채널   
               
                  // RGB를 YCbCr로 변환
                  Y  = cvRound((0.299 * R) + (0.587 * G) + (0.114 * B));
            
                  if(220 < Y) { //초록색
                     White_S.Count++;
                     White_S.DrawX += x+row;
                     White_S.DrawY += y+col;
                  }   
               }
            }

            // Red, Yellow, Green 색이 각각 검출된 (x, y) 좌표의 평균을 구함
            White_S.DrawX /= White_S.Count;
            White_S.DrawY /= White_S.Count;
            
            // 평균(x, y) 좌표에 원을 그림 (신호등 색 검출)
            if(White_S.Count > 200)//값은 수정해봐야
            {
               cvCircle(dstImage, cvPoint(White.DrawX, White.DrawY), 7, CV_RGB(255,255,255), 2);
               //출력 G
               Stop_Flag = 1;
               //cvRectangle(dstImage, cvPoint(x, y), cvPoint(x+row, y+col), CV_RGB(255, 0, 0), 2);
            }
      
            White_S.Count = 1;
         }
      }
///////////////////////////////////////////////////////////////////////	

		for(y=0; y<(dstImage->height); y++)
		{
			for(x=0; x<(dstImage->width); x++)
			{
				if(y < dstImage->height / 2) 	// 신호등 색을 검출하기 위한 height
				{

				}
				else 	// 차선 색을 검출하기 위한 height
				{
					roi_col = y - (dstImage->height / 2);
					
					// img_32f 이미지의 (x, roi_col) 데이터를 Lane_darkness에 저장 (1채널 영상)
					Lane_darkness = cvGetReal2D(img_32f, roi_col, x);
					
					// mag 이미지의 (x, roi_col) 데이터를 edge_darkness 저장 (1채널 영상)
					edge_darkness = cvGetReal2D(mag, roi_col, x);
					

					mask_darkness = cvGetReal2D(mask, roi_col, x);

					// Lane_darkness, edge_darkness를 이용하여 차선을 검출
					if( (Lane_darkness < 0.1) && (edge_darkness > 0.2) && (mask_darkness > 200) )
					{ 
						//cvSet2D(roiImage, roi_col, x, CV_RGB(255,0,255));
						cvSet2D(dstImage, y, x, CV_RGB(255,0,255));
					}
				}

			}
		}
		
		// edgeImage을 직선으로 만드는 cvHoughLines2 함수를 사용하여 차선을 검출하는 법
		// CV_HOUGH_STANDARD MODE

		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* seqLines;

		seqLines = cvHoughLines2(edgeImage, storage, CV_HOUGH_STANDARD, 1, CV_PI/180, 100, 0, 0 );
		
		for( int k = 0; k < MIN(seqLines->total,100); k++ )
		{
			float* line;
			float rho, theta;
			float c, s;
			float x0, y0;

			line = (float*)cvGetSeqElem(seqLines, k);
	
			rho   = line[0];
			theta = line[1];

            		// drawing line
			c = cos(theta);
			s = sin(theta);
			x0 = rho*c;
			y0 = rho*s;
	 		CvPoint pt1, pt2;
			pt1.x = cvRound(x0 + 1000*(-s));
			pt1.y = cvRound(y0 + 1000*(c));

			pt2.x = cvRound(x0 - 1000*(-s));
			pt2.y = cvRound(y0 - 1000*(c));
			
			cvLine(LanegrayImage, pt1, pt2, CV_RGB(255,255,255), 3, 8 );
		}

		//CV_HOUGH_PROBABILISTIC MODE
	
		seqLines = cvHoughLines2(edgeImage, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI/180, 80, 30, 3);

		for(int k=0; k<seqLines->total; k++)
		{
			CvPoint* line = (CvPoint*)cvGetSeqElem(seqLines, k);
			cvLine(LanegrayImage, line[0], line[1], cvScalar(0), 3, 8);
		}



		/////////////
		

      ////////////////////////////////////////////
      ////////////////CTRL
      /*
      if(Cross_Flag==1)
      {
         ////////////////////////장애물 영상처리 추가 필요
         ////////////////////////적외선을 이용할까?
         com1.Write("G",1);
         Sleep(3000);
         com1.Write("Y",1);
         Sleep(500);
         Cross_Cplt_Flag=1;
      }
      else
      {
         if(Yellow_Flag==1)
         {
            if(Stop_Flag==1)
            {
               com1.Write("G",1);
            }
            else
            {
               //break;?
            }
         }
         else if(Blue_Flag==1)      //초록불
         {
            com1.Write("Y",1);
            Sleep(500);
         }
         else if(Red_Flag==1)
         {
            ///장애물을 따로 Red추가 처리, 위치로 구분할 것
            //몇초동안 정지
            com1.Write("G",1);
         }
         else
         {
            //주차ON?
            if(Parking_Flag==1)
            {
               com1.Write("K",1);
               Parking_Cplt_Flag = 1;
            }
            else
            {
               //차량 보통 주행, 각도 속도 따라서 처리 가능하게 할 것, 8bit 신호 변환

               if (steer >= -40 && steer <= 40)      //직진 중간속도
               {
                  com1.Write("Y",1);
               }

               else if (steer >= -80 && steer < -40)   //Low left
               {
                  com1.Write("L",1);
               }
      
               else if (steer >= -120 && steer < -100)   //Mid left
               {
                  com1.Write("E",1);
               }

               else if (steer >= -160 && steer < -100)   //Ext left
               {
                  com1.Write("V",1);
               }

               else if (steer > 40 && steer <= 80)      //Low right
               {
                  com1.Write("H",1);
               }

               else if (steer > 80 && steer <= 100)   //Mid right
               {
                  com1.Write("B",1);
               }

               else if (steer > 100 && steer <= 160)   //Ext right
               {
                  com1.Write("R",1);
               }

               else
               {

               }
            }
         }
      }
      //주차, 주차 후 추가 필요

	  */
      ////////////////CTRL_END
      ////////////////////////////////////////////











		//////////////

		cvNamedWindow("srcImage", CV_WINDOW_AUTOSIZE);	// 영상을 출력할 윈도우 생성 
		cvShowImage(  "srcImage", srcImage);  	        // 영상을 화면에 출력함

		//cvNamedWindow("grayImage", CV_WINDOW_AUTOSIZE);
		//cvShowImage ( "grayImage", grayImage);
		
		//cvNamedWindow("img_32f", CV_WINDOW_AUTOSIZE);
		//cvShowImage ( "img_32f", img_32f);
		
		//cvNamedWindow("diff_x", CV_WINDOW_AUTOSIZE);
		//cvShowImage ( "diff_x", diff_x);
		
		//cvNamedWindow("diff_y", CV_WINDOW_AUTOSIZE);
		//cvShowImage ( "diff_y", diff_y);
		
		//cvNamedWindow("mag", CV_WINDOW_AUTOSIZE);
		//cvShowImage ( "mag", mag);

		//cvNamedWindow("ori", CV_WINDOW_AUTOSIZE);
		//cvShowImage ( "ori", ori);

		cvNamedWindow("dstImage", CV_WINDOW_AUTOSIZE);
		cvShowImage ( "dstImage", dstImage);
		
		//cvNamedWindow("roiImage", CV_WINDOW_AUTOSIZE);
		//cvShowImage ( "roiImage", roiImage);

		//cvNamedWindow("edgeImage", CV_WINDOW_AUTOSIZE);
		//cvShowImage ( "edgeImage", edgeImage);

		//cvNamedWindow("LanegrayImage", CV_WINDOW_AUTOSIZE);
		//cvShowImage ( "LanegrayImage", LanegrayImage);

		//cvNamedWindow("mask", CV_WINDOW_AUTOSIZE);
		//cvShowImage ( "mask", mask);
		
		// 이미지 영상을 활성화 시키고 키보드를 입력해야 된다. 
		// 키보드 입력과 동시에 OpenCV에서 영상을 화면에 출력할 시간여유를 주어야함.
		int c = cvWaitKey(10);			

		if( c == 27 || c == 'q') // esc == 27
			 break;
		
		// 특정 키에 해당하는 루틴에 소스 추가
		switch( c )
		{
			case (VK_UP<<16):
				printf("Press	UP\n");
				com1.Write ("a", 1);
				break;

			case (VK_DOWN<<16):
				printf("Press	DOWN\n");
				com1.Write ("b", 1);
				break;

			case (VK_RIGHT<<16):
				printf("Press	RIGHT\n");
				com1.Write ("f", 1);
				break;

			case (VK_LEFT<<16):
				printf("Press	LEFT\n");
				com1.Write ("d", 1);
				break;
				
			case 'c':
				printf("Press   STOP\n");
				com1.Write ("c", 1);
				break;
				
			case 'e':
				printf("Press   STRAIGHT\n");
				com1.Write ("e", 1);
				break;
				
			case '+':
				printf("+");
				com1.Write ("+", 1);
				break;
		}
		

		cvReleaseMemStorage(&storage );

		cvReleaseImage(&srcImage);	// 영상의 메모리를 해제 
		cvReleaseImage(&grayImage);
		cvReleaseImage(&img_32f);
		cvReleaseImage(&diff_x);
		cvReleaseImage(&diff_y);
		cvReleaseImage(&mag);
		cvReleaseImage(&ori);
		cvReleaseImage(&dstImage);
		cvReleaseImage(&roiImage);
		cvReleaseImage(&LanegrayImage);
		cvReleaseImage(&edgeImage);
		cvReleaseImage(&mask);

	}

	cvDestroyAllWindows();	// 모든 영상을 종료
	return 0;
}

