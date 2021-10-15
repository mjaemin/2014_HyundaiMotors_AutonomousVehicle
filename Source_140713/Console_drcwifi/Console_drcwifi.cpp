// Console_drcwifi.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <process.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>

#pragma warning (disable:4996)

// ���ϰ��� ������ 
WSADATA wsaData;
SOCKADDR_IN servAddr;
SOCKET hSocket;

// CSerialPort Ŭ������ com1 ��ü ����
CSerialPort com1;

PC_TO_CAM 	gPC;	// PC���� CAM���� ������ ������ 
CAM_TO_PC 	gCAM;	// CAM���� PC�� ������ ������ 

#if 0
	int IMG_W	= 320;		// ������ ������ 
	int IMG_H	= 240;		// ������ ������
	int IMGQuality = 50;
#else
	// ������ ���� VGA���� �ӵ����ϵ� . VGA���� ������ ��� ���� 
	int IMG_W	= 640;		// ������ ������ 
	int IMG_H	= 480;		// ������ ������ 
	int IMGQuality = 50;
#endif

BYTE mbuff[640*480*2] ;	//���� ���ſ� ���� 

//�κ��� 
//int JOG_DIR_1, JOG_LEN_1;
//int JOG_DIR_2, JOG_LEN_2;
//int JOG_BTN1;

//�������� �ƽ�Ű���·� ��ȯ�ϴ� �Լ�  
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

//���� ���� �ʱ�ȭ �� ī�޶�� ���� ���� 
int DRC_InitSocket()
{
	//JOG_DIR_1 = JOG_DIR_2 = JOG_LEN_1 = JOG_LEN_2 = JOG_BTN1 = 0;
	
	// �ۼ��� ��Ŷ �ڷᱸ�� �ʱ�ȭ 
	memset((u8 *)&gPC, 	0, sizeof(PC_TO_CAM));
	memset((u8 *)&gCAM, 	0, sizeof(CAM_TO_PC));
	
	// ���� �ʱ�ȭ ..�츰 Ŭ���̾�Ʈ�� �����Ѵ�.
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
	servAddr.sin_addr.s_addr	= inet_addr("192.168.123.10");	// ī�޶��� IP�ּ� ...����
	servAddr.sin_port		= htons(6400); 			// ī�޶��� ��Ʈ��ȣ 
	
	// ī�޶���� ������ �õ��Ѵ� 
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


//ī�޶󿡰� ������ ��û�Ѵ� 
void DRC_RequestImage()
{
	//ī�޶󿡰� �۽��� �����͸� �ۼ��� 
	gPC.HEAD[0]	= 0x55;				// �ϴ� ��� 3����Ʈ �ۼ� 
	gPC.HEAD[1]	= 0x33;
	gPC.HEAD[2]	= 0xAA;
	gPC.CmdType 	= TYPE_GET_REALTIME_IMG;	// ���ʿ� ���� 
	gPC.CmdDataLen	= 11; 				// ���ʿ� ���� 	
	gPC.UartTxLen	= UART_TX_DATA_LEN;		// DRC-WIFI �� ������ UART TX �� ũ�⸦ 25byte �� ���� ����(Max 100byte)
	gPC.TotalLen	= sizeof(PC_TO_CAM);		// ���ʿ� ���� 		
	gPC.aWidth 	= IMG_W/8;			// ���� �ޱ���ϴ� ������ ũ�⸦ 320x240 ���� ����(Max 640x480)
	gPC.aHeight	= IMG_H/8;
	gPC.Quality	= IMGQuality;			// ������ �̹��� ����Ƽ�� ���� (1~90, ���� 60) ��ȭ���ϼ��� �ӵ� ���ϵ� 
	
	/*
	if(UART_TX_DATA_LEN > 0)
	{		//ī�޶��� �ø���� ������ �����Ͱ� �����ϸ� UART TX�ۼ�
		// ���������� �Ŵ��� ���� ��� 
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

	char *p = (char *)&gPC;    // ���� ������ ���� 
	
	int Sumlen =0;	           // �۽� ���� ���� ������
	
	while(Sumlen<gPC.TotalLen) // ��û �����͸� ī�޶�� �۽��Ѵ�
	{ 	 
		int TempLen = send(hSocket, &p[Sumlen], gPC.TotalLen-Sumlen, 0 ); 		
		Sumlen += TempLen; 
		//printf("_TX:%2d", TempLen);
	}
}

//ī�޶�� ���� ������ ���� �Ѵ� 
int DRC_GetImage()
{
	int Sumlen =0;			// ���� ���� ���� ������
	gCAM.TotalLen = 1000000;	//ī�޶� ������ �������� ���̸� �˼��� ������...�ϴ� ����� ū����...
	
	while(Sumlen<gCAM.TotalLen)
	{
		int TempLen = recv(hSocket,(char *)&mbuff[Sumlen], gCAM.TotalLen-Sumlen, 0 );

		//���� �����޴ٸ� ���� 
		if(TempLen>0)
		{
			//printf(" _RX:%5d", TempLen);
			//ó�� �����޴ٸ� ���� 
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

	//������ ��� �����ϸ� �̰����� ...
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

// FB-155BC (Bluetooth Module) Connect ��ɾ�
// RC ���� �ڵ����� Bluetooth Device ID : 00190137FD70
char CONNECT[17] = "ATD00190137FD0A";

// FB-155BC (Bluetooth Module) Reset ��ɾ�
char init[5] = "ATZ";


UINT WINAPI SerialInterface( void* arg )
{
	int i = 0;    
	char Inbuff[1024] = "";
	char * ptr = NULL;

	CONNECT[15] = 0xd;
	init[3] = 0xd; 
	
	// �ø��� ����
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

// ���Ƿ� ����ũ ����
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
	// �ø��� ����� ���� ������ ������
	HANDLE hThread;
    UINT uThreadID[1];
	uThreadID[0] = 0;
	
	// Port, Baud rate, Data bit, Stop bit, Parity ����  
	com1.Open ("COM3", CBR_9600, 8, ONESTOPBIT, NOPARITY);
	com1.SetTimeout (10, 10, 1);

	int R = 0, G = 0, B = 0;
	int Y = 0, Cb = 0, Cr = 0;
	int x = 0, y = 0, row = 0, col = 0;
	int roi_col = 0;


	double edge_darkness = 0, Lane_darkness = 0, mask_darkness = 0;
	
	if(DRC_InitSocket()==0)			//���� ���� �ʱ�ȭ �� ī�޶�� ���� ���� 
		return 0;

	// FB-155BC (Bluetooth Module) Device Search ��ɾ�
	char Outbuff[11] = "AT+BTINQ?";
	Outbuff[9] = 0xd;

	// �ø��� �۽�
	com1.Write (Outbuff, strlen(Outbuff));

	while(1)
	{	
		// �ø��� ����� ���� ������ ����
		hThread = (HANDLE)_beginthreadex( NULL, 0, SerialInterface, NULL, 0, &uThreadID[0] );
		///////////////

		//////////////
		DRC_RequestImage();		//ī�޶󿡰� ������ ��û�Ѵ� 
		int ret = DRC_GetImage();	//ī�޶�� ���� ������ ���� �Ѵ� 
		if(ret == 0)
		{
			printf("Wrong Image\n");
			continue;
		}
		
		CvMat *buf = cvCreateMat(1, IMG_W*IMG_H*2, CV_8UC1 );  

		memcpy((u8 *)(buf->data.ptr), &mbuff[sizeof(CAM_TO_PC)], gCAM.iJpegLen); 	// ������ ������ �����Ѵ� 	
		
		// ���� ���� (srcImage)
		IplImage *srcImage = cvDecodeImage(buf, 1);					// ����� �����̹Ƿ� ������ Ǯ���ش� 
		
		// TODO:  ���⼭ �ʿ��� ����ó���� �����Ѵ�.
		
		// ���� ó���Ŀ� ��� ������ ����� ���Ͽ� ���� ����ü ���� dstImage�� ���� - 3ä�� (�÷� ����)
		IplImage* dstImage = cvCreateImage( cvGetSize(srcImage), IPL_DEPTH_8U, 3);
		
		// ���� ����(srcImage)�� dstImage�� ����
		cvCopy(srcImage, dstImage, 0);
			
		// ��ȣ���� �����ϱ� ���Ͽ� ���ɿ����� �簢������ ǥ��
		//cvRectangle(dstImage, cvPoint(0, 0), cvPoint(srcImage->width-1, (srcImage->height / 2)-1), CV_RGB(0, 255, 0), 2);
		
		// ������ �����ϱ� ���Ͽ� ���ɿ����� �簢������ ǥ��
		//cvRectangle(dstImage, cvPoint(0, srcImage->height/2), cvPoint(srcImage->width-1, srcImage->height-1), CV_RGB(0, 0, 255), 2);
		
		// ������ �����ϱ� ���� ���� ����(roi)�� ����
		CvRect roi = cvRect ( 0, (dstImage->height / 2), dstImage->width, (dstImage->height / 2) );
		
		// dstImage ���� ���ɿ���(roi)�� ����
		cvSetImageROI (dstImage, roi);
		
		// ���� ����(roi)�� ���� ũ�� ��ŭ ���� ����ü ���� roiImage�� ���� - 3ä�� (�÷� ����)
		IplImage* roiImage = cvCreateImage( cvSize(roi.width, roi.height), IPL_DEPTH_8U, 3);
		
		// dstImage�� ���ɿ���(roi)�� roiImage�� ����
		cvCopy (dstImage, roiImage);
		
		// dstImage�� ���ɿ���(roi)�� ����
		cvResetImageROI (dstImage);
		
		// 0 ~ 255 �׷��� �����Ϸ� ��ȯ (�ڷ��� : IPL_DEPTH_8U)
		// roiImage�� ���� ũ�� ��ŭ ���� ����ü ���� grayImage ���� - 1ä�� (Gray ����)
  		IplImage *grayImage = cvCreateImage( cvGetSize(roiImage), IPL_DEPTH_8U, 1);
  		// 3ä��(�÷�) ������ roiImage�� 1ä��(GRAY) �������� ��ȯ�Ͽ� grayImage�� ����
		cvCvtColor( roiImage, grayImage, CV_BGR2GRAY );
		
		// 0. ~ 1. �׷��� �����Ϸ� ��ȯ (�ڷ��� : IPL_DEPTH_32F)
		IplImage *img_32f = cvCreateImage( cvGetSize(grayImage), IPL_DEPTH_32F, 1 );
		// grayImage�� ���� �����Ϳ� 255�� ������ img_32f�� ���� (0. ~ 1. �׷��� �����Ϸ� ��ȯ)
		cvConvertScale(grayImage, img_32f, 1.0 / 255.0, 0);
		// ���͸� �Լ� (cvSmooth)
		cvSmooth(img_32f, img_32f, CV_GAUSSIAN, 5);

		// Sobel �����ڷ� �̹����� x, y �������� �̺� ��, edge ����.
		IplImage *diff_x = cvCreateImage(cvGetSize(img_32f), IPL_DEPTH_32F, 1);
		IplImage *diff_y = cvCreateImage(cvGetSize(img_32f), IPL_DEPTH_32F, 1);
		cvSobel(img_32f, diff_x, 1, 0, 3); // img_32f�� x������ �̺��Ͽ� ����� �����͸� diff_x ����
		cvSobel(img_32f, diff_y, 0, 1, 3); // img_32f�� y������ �̺��Ͽ� ����� �����͸� diff_y ����

		// Edge�� magnitude�� orientation�� ���
		IplImage *mag = cvCreateImage(cvGetSize(img_32f), IPL_DEPTH_32F, 1);
		IplImage *ori = cvCreateImage(cvGetSize(img_32f), IPL_DEPTH_32F, 1);
		// (diff_x, diff_y)�� ���� ��ǥ�� �����͸� ����ǥ (mag, ori)�� ��ȯ�Ͽ� ����
		cvCartToPolar(diff_x, diff_y, mag, ori, 1);
		
		// edgeImage�� cvCanny �Լ��� �̿��Ͽ� Edge�� ����
		IplImage *edgeImage = cvCreateImage(cvGetSize(roiImage), IPL_DEPTH_8U, 1);
		cvCanny(grayImage, edgeImage, 50, 200, 3);
		
		// ��ȣ�� ���� �����ϱ� ���Ͽ� Red, Yellow, Green ����ü ������ ����
		ColorDetect Red, Yellow, Green, Blue, White, White_S;
		CvScalar ColorData;

		Red.Count = 1;		// Red ����� ȭ�� ��
		Red.DrawX = 0;          // Red ����� x ��ǥ
		Red.DrawY = 0;          // Red ����� y ��ǥ
		
		Yellow.Count = 1; 	// Yellow ����� ȭ�� ��
		Yellow.DrawX = 0; 	// Yellow ����� x ��ǥ
		Yellow.DrawY = 0;	// Yellow ����� y ��ǥ
		
		Green.Count = 1;	// Green ����� ȭ�� ��
		Green.DrawX = 0; 	// Green ����� x ��ǥ
		Green.DrawY = 0;	// Green ����� y ��ǥ
///////////////

		Blue.Count = 1;      // Blue ����� ȭ�� ��
		Blue.DrawX = 0;    // Blue ����� x ��ǥ
		Blue.DrawY = 0;      // Blue ����� y ��ǥ

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
		cvPow(img_32f, img_32f, 2);	// img_32f�� ����� �����ϱ� ���Ͽ� ������ ����
		
		// img_32f�� ���� ���콺 Ŀ���� ������ ��(������)�� Ȯ���ϱ� ���� on_mouseEvent�Լ��� ȣ��
		cvSetMouseCallback("srcImage", on_mouseEvent,  (void *)srcImage);

		IplImage *LanegrayImage = cvCreateImage( cvGetSize(roiImage), IPL_DEPTH_8U, 1 );
		// LanegrayImage�� ������ White�� ����
		cvSet(LanegrayImage, cvScalar(255));
		
		IplImage* mask = cvCreateImage( cvGetSize(roiImage), IPL_DEPTH_8U, 1);
		// mask ������ White�� ����
		cvSet(mask, cvScalar(255));
     
 	/////////////////////
      Red_Flag = 0;
      Yellow_Flag = 0;
      Blue_Flag = 0;
      Cross_Flag = 0;
      Stop_Flag = 0;
      ////////////////////

		mask_field(mask, roiImage); // ����ũ ���� �Լ�

		for(y=0; y<dstImage->height/2; y+=40)      // Width x Height �ػ󵵿��� Height/2 �κи� �� ���� 
		{
			for(x=0; x<dstImage->width; x+=40)
			{
				// 40 x 40 �������� ���ϴ� ���� ã��.
				for(col=0; col<40; col++)
				{
					for(row=0; row<40; row++)
					{
						// dstImage �̹����� (x, y) �����͸� CvScalar ����ü ���� ColorData�� ���� (3ä�� ����)
						ColorData = cvGet2D(srcImage, y+col, x+row);
						B = cvRound(ColorData.val[0]); // B ä��
						G = cvRound(ColorData.val[1]); // G ä��
						R = cvRound(ColorData.val[2]); // R ä��	
					
						// RGB�� YCbCr�� ��ȯ
						Y  = cvRound((0.299 * R) + (0.587 * G) + (0.114 * B));
						Cb = cvRound(0.5643 * (B-Y) + 128);
						Cr = cvRound(0.7132 * (R-Y) + 128);
				
						if( (105 < Cb && Cb < 140) && (50 < Cr && Cr < 90) ) { //�ʷϻ�
							Green.Count++;
							Green.DrawX += x+row;
							Green.DrawY += y+col;
						}	
				
						if( (20 <Cb && Cb < 70) && (130 < Cr && Cr < 150) ) {	//�����
							Yellow.Count++;
							Yellow.DrawX += x+row;
							Yellow.DrawY += y+col;
						}
						
						if( (90 <Cb && Cb < 120) && (190 < Cr && Cr < 215) ) { //������	
							Red.Count++;
							Red.DrawX += x+row;
							Red.DrawY += y+col;
						}
						
						if( (180 <Cb && Cb < 230) && (90 < Cr && Cr < 130) ) { //������	
							Blue.Count++;
							Blue.DrawX += x+row;
							Blue.DrawY += y+col;
						}
					}
				}

				// Red, Yellow, Green ���� ���� ����� (x, y) ��ǥ�� ����� ����
				Red.DrawX /= Red.Count;
				Red.DrawY /= Red.Count;

				Yellow.DrawX /= Yellow.Count;
				Yellow.DrawY /= Yellow.Count;

				Green.DrawX /= Green.Count;
				Green.DrawY /= Green.Count;

				Blue.DrawX /= Blue.Count;
				Blue.DrawY /= Blue.Count;
				
				// ���(x, y) ��ǥ�� ���� �׸� (��ȣ�� �� ����)
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
         for(y=dstImage->height/2; y<dstImage->height; y+=40)      // Width x Height �ػ󵵿��� Height/2 �κи� �� ���� 
         {
            for(x=0; x<dstImage->width; x+=40)
            {
               // 40 x 40 �������� ���ϴ� ���� ã��.
               for(col=0; col<40; col++)
               {
                  for(row=0; row<40; row++)
                  {
                     // dstImage �̹����� (x, y) �����͸� CvScalar ����ü ���� ColorData�� ���� (3ä�� ����)
                     ColorData = cvGet2D(srcImage, y+col, x+row);
                     B = cvRound(ColorData.val[0]); // B ä��
                     G = cvRound(ColorData.val[1]); // G ä��
                     R = cvRound(ColorData.val[2]); // R ä��   
               
                     // RGB�� YCbCr�� ��ȯ
                     Y  = cvRound((0.299 * R) + (0.587 * G) + (0.114 * B));
            
                     if(220 < Y) { //�ʷϻ�
                        White.Count++;
                        White.DrawX += x+row;
                        White.DrawY += y+col;
                     }
                  }
               }

               // Red, Yellow, Green ���� ���� ����� (x, y) ��ǥ�� ����� ����
               White.DrawX /= White.Count;
               White.DrawY /= White.Count;
            
               // ���(x, y) ��ǥ�� ���� �׸� (��ȣ�� �� ����)
               if(White.Count > 800)//���� �����غ���
               {
                  cvCircle(dstImage, cvPoint(White.DrawX, White.DrawY), 7, CV_RGB(255,255,255), 2);
                  //��� G
                  Cross_Flag = 1;
                  //cvRectangle(dstImage, cvPoint(x, y), cvPoint(x+row, y+col), CV_RGB(255, 0, 0), 2);
               }
      
               White.Count = 1;
            }
         }   
      }
/////////////STOP LINE DETECT//////////////////////////////////////////////////////////
      ////������ y height ���� (����) ��� ���� ����(����)�� ������ �̻��̸� ���� �����?
      for(y=dstImage->height/2; y<=dstImage->height*0.75; y+=40)      // Width x Height �ػ󵵿��� Height/2 �κи� �� ���� 
      {
         for(x=0; x<dstImage->width; x+=40)
         {
            // 40 x 40 �������� ���ϴ� ���� ã��.
            for(col=0; col<40; col++)
            {
               for(row=0; row<40; row++)
               {
                  // dstImage �̹����� (x, y) �����͸� CvScalar ����ü ���� ColorData�� ���� (3ä�� ����)
                  ColorData = cvGet2D(srcImage, y+col, x+row);
                  B = cvRound(ColorData.val[0]); // B ä��
                  G = cvRound(ColorData.val[1]); // G ä��
                  R = cvRound(ColorData.val[2]); // R ä��   
               
                  // RGB�� YCbCr�� ��ȯ
                  Y  = cvRound((0.299 * R) + (0.587 * G) + (0.114 * B));
            
                  if(220 < Y) { //�ʷϻ�
                     White_S.Count++;
                     White_S.DrawX += x+row;
                     White_S.DrawY += y+col;
                  }   
               }
            }

            // Red, Yellow, Green ���� ���� ����� (x, y) ��ǥ�� ����� ����
            White_S.DrawX /= White_S.Count;
            White_S.DrawY /= White_S.Count;
            
            // ���(x, y) ��ǥ�� ���� �׸� (��ȣ�� �� ����)
            if(White_S.Count > 200)//���� �����غ���
            {
               cvCircle(dstImage, cvPoint(White.DrawX, White.DrawY), 7, CV_RGB(255,255,255), 2);
               //��� G
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
				if(y < dstImage->height / 2) 	// ��ȣ�� ���� �����ϱ� ���� height
				{

				}
				else 	// ���� ���� �����ϱ� ���� height
				{
					roi_col = y - (dstImage->height / 2);
					
					// img_32f �̹����� (x, roi_col) �����͸� Lane_darkness�� ���� (1ä�� ����)
					Lane_darkness = cvGetReal2D(img_32f, roi_col, x);
					
					// mag �̹����� (x, roi_col) �����͸� edge_darkness ���� (1ä�� ����)
					edge_darkness = cvGetReal2D(mag, roi_col, x);
					

					mask_darkness = cvGetReal2D(mask, roi_col, x);

					// Lane_darkness, edge_darkness�� �̿��Ͽ� ������ ����
					if( (Lane_darkness < 0.1) && (edge_darkness > 0.2) && (mask_darkness > 200) )
					{ 
						//cvSet2D(roiImage, roi_col, x, CV_RGB(255,0,255));
						cvSet2D(dstImage, y, x, CV_RGB(255,0,255));
					}
				}

			}
		}
		
		// edgeImage�� �������� ����� cvHoughLines2 �Լ��� ����Ͽ� ������ �����ϴ� ��
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
         ////////////////////////��ֹ� ����ó�� �߰� �ʿ�
         ////////////////////////���ܼ��� �̿��ұ�?
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
         else if(Blue_Flag==1)      //�ʷϺ�
         {
            com1.Write("Y",1);
            Sleep(500);
         }
         else if(Red_Flag==1)
         {
            ///��ֹ��� ���� Red�߰� ó��, ��ġ�� ������ ��
            //���ʵ��� ����
            com1.Write("G",1);
         }
         else
         {
            //����ON?
            if(Parking_Flag==1)
            {
               com1.Write("K",1);
               Parking_Cplt_Flag = 1;
            }
            else
            {
               //���� ���� ����, ���� �ӵ� ���� ó�� �����ϰ� �� ��, 8bit ��ȣ ��ȯ

               if (steer >= -40 && steer <= 40)      //���� �߰��ӵ�
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
      //����, ���� �� �߰� �ʿ�

	  */
      ////////////////CTRL_END
      ////////////////////////////////////////////











		//////////////

		cvNamedWindow("srcImage", CV_WINDOW_AUTOSIZE);	// ������ ����� ������ ���� 
		cvShowImage(  "srcImage", srcImage);  	        // ������ ȭ�鿡 �����

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
		
		// �̹��� ������ Ȱ��ȭ ��Ű�� Ű���带 �Է��ؾ� �ȴ�. 
		// Ű���� �Է°� ���ÿ� OpenCV���� ������ ȭ�鿡 ����� �ð������� �־����.
		int c = cvWaitKey(10);			

		if( c == 27 || c == 'q') // esc == 27
			 break;
		
		// Ư�� Ű�� �ش��ϴ� ��ƾ�� �ҽ� �߰�
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

		cvReleaseImage(&srcImage);	// ������ �޸𸮸� ���� 
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

	cvDestroyAllWindows();	// ��� ������ ����
	return 0;
}

