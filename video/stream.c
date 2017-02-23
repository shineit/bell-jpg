#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "param.h"
#include "debug.h"

int GetMotionLevel()
{
	int nLevel;

	switch(bparam.stBell.alarm_level)
	{
		case 1:
			nLevel = 10;
			break;
		case 2:
			nLevel = 8;
			break;
		case 3:
			nLevel = 6;
			break;
		case 4:
			nLevel = 4;
			break;
		case 5:
		default:
			nLevel = 2;
			break;

	}

    return nLevel;
}

int GetMotionCheckDiff()
{
    int diff = 100;

	switch(bparam.stBell.alarm_level)
    {
        case 1:
        case 2:
            diff = 200;
            break;
        case 3:
            diff = 150;
            break;
        case 4:
        case 5:
		default:
            diff = 100;
            break;
    }

    return diff;
}

int MotionDetect( int* motionarea )
{
    int 	result = 0;

    int		iRet = 0;

#ifdef SENSOR_3894
	iRet = H264_GetMotionResult( (unsigned char*)&result );
	//Textout("H264_GetMotionResult status=%d",iRet);
	return iRet;
#else
    iRet = H264GetMotionResult( (unsigned char*)&result );
#endif
    if ( result > 0 )
    {
    	//Textout("H264GetMotionResult is %d",iRet);
        *motionarea = result;
#ifdef PREFIX_8433_PPCS
		//if(iRet >1)//至少两个区域以上发生移动侦测才算触发报警
		if(iRet >0)
		{
			iRet = 0x01;
		}
		else
		{
			iRet = 0;
		}
#else
		iRet = 0x01;
#endif
        
    }

    return iRet;
}



void* CameraCheckProc( void* arg )
{
    int iRet;

    while ( 1 )
    {
        iRet = access( "/tmp/uvcvideo.bin", R_OK );
        if ( iRet == 0x00 )
        {
            Textout( "check video is error" );
            #ifdef INSMOD_UVC_DRIVER
            Textout("rmmod uvcvideo");
            DoSystem("rm /tmp/uvcvideo.bin");
            StopVideoCaptureEx();
            sleep(1);
            RmmodUvcDriver();
            sleep(1);
            InsmodUvcDriver();
            sleep(1);
            StartVideoCaptureEx();
            #else
			Textout("software Reboot....");
            SetRebootCgi();
            #endif
        }

        sleep( 10 );
    }
}

void VCodecStart( void )
{
    pthread_t videothread1;
    pthread_create( &videothread1, 0, CameraCheckProc, NULL );
	pthread_detach(videothread1);
}

