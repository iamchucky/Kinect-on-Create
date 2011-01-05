#include "camera\udpCameraBroadcaster.h"

#include <opencv\cv.h>
#include <opencv\cxcore.h>
#include <opencv\highgui.h>
#include <CLNUIDevice.h>

//camera configuration
UDPCameraBroadcaster* broadcastCam;

void InitCommon ()
{	
	broadcastCam = new UDPCameraBroadcaster ();								// Sensor broadcast
	
}


int main()
{
	InitCommon ();
	CLNUICamera cam;
	CLNUIMotor motor;

	PCHAR serial;
	bool status = false;
	PDWORD dImage = (PDWORD) malloc(640*480*4);
	PBYTE dImageBW = (PBYTE) malloc(640*480);
	int frame_count = 0;
	int mode = 1;
	bool running = true;
	short x,y,z;

	IplImage * colorImage = cvCreateImage(cvSize(640,480), IPL_DEPTH_8U, 4);
	IplImage * depthImage = cvCreateImage(cvSize(640,480), IPL_DEPTH_8U, 4);
	IplImage * depthImageBW = cvCreateImage(cvSize(640,480), IPL_DEPTH_8U, 1);
	IplImage * depthImageBWresize = cvCreateImage(cvSize(160,120), IPL_DEPTH_8U, 1);

	int camera_count = GetNUIDeviceCount();
	serial = GetNUIDeviceSerial(0);
	cam = CreateNUICamera(serial);
	motor = CreateNUIMotor(serial);
	status = StartNUICamera(cam);
	
	if (status)
	{
		status = SetNUIMotorPosition(motor, 0);
		while(running)
		{
			GetNUIMotorAccelerometer(motor, x, y, z);
			printf("x: %d, y: %d, z: %d\n", x, y, z);
			if (mode == 1)
			{
				if (GetNUICameraDepthFrameCorrected8(cam, (PBYTE)dImageBW))
				{
					frame_count++; 
					memcpy(depthImageBW->imageData, dImageBW, 640*480);
					cvResize(depthImageBW, depthImageBWresize);
					cvShowImage( "Kinect", depthImageBW);
					broadcastCam->SendImage((unsigned char*)depthImageBWresize->imageData,depthImageBWresize->widthStep, 160, 120,0,0,1);
				}
			}
			else if (mode == 2)
			{
				if (GetNUICameraDepthFrameRGB32(cam, (PDWORD)dImage))
				{
					frame_count++; 
					memcpy(depthImage->imageData, dImage, 640*480*4);
					cvShowImage( "Kinect", depthImage);
				}
			}
			else
			{
				if (GetNUICameraColorFrameRGB32(cam, (PDWORD)dImage))
				{
					frame_count++; 
					memcpy(colorImage->imageData, dImage, 640*480*4);
					cvShowImage( "Kinect", colorImage);
				}
			}
			switch(cvWaitKey(1))
			{
				case '1':
					mode = 1;
					break;
				case '2':
					mode = 2;
					break;
				case '3':
					mode = 3;
					break;
				case 27:
					running = false;
					break;
				default:
					break;
			}
			
		}
		cvReleaseImage(&colorImage);
		cvReleaseImage(&depthImage);
		cvReleaseImage(&depthImageBW);
		cvReleaseImage(&depthImageBWresize);
		
		status = DestroyNUIMotor(motor);
	}
	/*status = StopNUICamera(cam);
	status = DestroyNUICamera(cam);*/
	
	return 0;
}