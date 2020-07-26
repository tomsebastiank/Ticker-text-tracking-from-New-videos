// code for text detection in a video with tracking
// Include Standard C/C++ Header Files Here
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// Include OpenCV Header Files Here
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/cxcore.h>
#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/legacy/legacy.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>



// Include CVDRIK Header Files Here
#include "../../CVDRIK/CVDRIKBlurr_edge/CVDRIKBlurr_edge.h"
#include "../../CVDRIK/CVDRIKticker/CVDRIKticker.h"

CvRect initRegion;
bool drawing_box = false;
// A litte subroutine to draw a initRegion onto an image
//
void draw_box( IplImage* img, CvRect rect ) 
{
	cvRectangle (img, cvPoint(initRegion.x,initRegion.y),cvPoint(initRegion.x+initRegion.width,initRegion.y+initRegion.height),cvScalar(0xff,0x00,0x00)); /* red */
}
// This is our mouse callback. If the user
// presses the left button, we start a initRegion.
// when the user releases that button, then we
// add the initRegion to the current image. When the
// mouse is dragged (with the button down) we
// resize the initRegion.
//
void my_mouse_callback( int event, int x, int y, int flags, void* param) 
{
	IplImage* image = (IplImage*) param;
	switch( event ) 
	{
		case CV_EVENT_MOUSEMOVE: 
		{
			if( drawing_box ) 
			{
				initRegion.width = 4*((int)((x-initRegion.x)/4));
				initRegion.height = y-initRegion.y;
			}
		}
		break;
		case CV_EVENT_LBUTTONDOWN: 
		{
			drawing_box = true;
			initRegion = cvRect(x, y, 0, 0);
		}
		break;
		case CV_EVENT_LBUTTONUP: 
		{
			drawing_box = false;
			if(initRegion.width<0) 
			{
				initRegion.x+=initRegion.width;
				initRegion.width *=-1;
			}
			if(initRegion.height<0) 
			{
				initRegion.y+=initRegion.height;
				initRegion.height*=-1;
			}
			draw_box(image, initRegion);
		}
		break;
	}
}




using namespace cv;
int main( int argc , char** argv )
{
	
	// Check Usage
	if( argc != 2 )
	{
		// Show Purpose
		printf( "\n\n Purpose :  video \n\n" );
		
		// Show Usage
		printf( "\n\n Usage As : test1 [ParamFileName] \n\n" );
		
		// Show Sample Command Line
		printf( "\n\n Sample Command Line : ./test1 [videoTD.txt] \n\n" );
		
		return( 0 );
	}
	//open the parameter file
	FILE* fp;
	fp = fopen( argv[1] , "r" );
	printf("\n%s\n",argv[1]);
	// Reading the Parameter File : video Files Information
	char paramNameString[100] , eqString[3] , headerString[100];
	fscanf( fp , "%s" , headerString );
	
	char vidName[100];
	fscanf( fp , "%s%s%s" , paramNameString , eqString , vidName );
	
	char outvideo[100];
	fscanf( fp , "%s%s%s" , paramNameString , eqString , outvideo );
	 
	
	//open video
	
	CvCapture *capture;
	capture= cvCreateFileCapture( vidName );
	if (capture == NULL)
	{
		// Continue to Next video
		printf( "\n\n Can not read video  \n\n" );
		return( 1 );
	}
	CvSize size;
	//determine the video size
	size = cvGetSize( cvQueryFrame( capture ) );
	int totalFrames = 0;
	totalFrames=cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
	double fps = (int)cvGetCaptureProperty(capture,CV_CAP_PROP_FPS);
	// printf( "\n\n*************************************" );
	printf( "\n\n video name %s\ntotal frames %d\n Size %dX%d \n",vidName,totalFrames,size.width,size.height);
	
	cvGrabFrame(capture);//incorporate the check on reading image check output
	//creating temporary images for handling the frames
	IplImage* inImg = NULL;
	inImg = cvCreateImage( size , IPL_DEPTH_8U , 3 );
	cvNamedWindow( "Output" , CV_WINDOW_NORMAL );
	//-----------------------------------------------
	int frameCount = 1; // CvRect rect=cvRect(0,0,200,200);
	inImg= cvQueryFrame( capture );
	cvNamedWindow( "processedData" , CV_WINDOW_NORMAL );
		cvSetMouseCallback( "processedData", my_mouse_callback, (void*) inImg );
		IplImage* temp = cvCloneImage( inImg );

	while( 1 ) 
		{
			cvCopyImage( inImg, temp );
			if( drawing_box ) draw_box( temp, initRegion );
			cvShowImage( "processedData", temp );
			//printf("\nSelected Region Parameters . . . . . .\n");
			//printf("\nx = %d y = %d w = %d h = %d\n", initRegion.x, initRegion.y ,initRegion.width , initRegion.height);
			//printf("\n Press 'Esc' Key If Region Is Selected\n");
			if( cvWaitKey( 15 )==27 ) break;
		}


	CvVideoWriter* video = cvCreateVideoWriter(outvideo, CV_FOURCC('P','I','M','1'),fps , cvSize(initRegion.width,initRegion.height),1 );
	IplImage* newrt= NULL;IplImage* newrt_edge= NULL;IplImage* newrt_mono= NULL;IplImage*copy_newrt_img= NULL;IplImage*newrt_otsu= NULL;
	IplImage*newrt_hist=NULL;IplImage*newrt_part=NULL;IplImage*newrt_dil= NULL;IplImage*temp_img= NULL;IplImage*temp_copy= NULL;


	
	newrt= cvCreateImage( cvSize(initRegion.width,initRegion.height) , IPL_DEPTH_8U , 3 );
	temp_img= cvCreateImage( cvSize(initRegion.width,initRegion.height) , IPL_DEPTH_8U , 3 );
	temp_copy= cvCreateImage( cvSize(initRegion.width,initRegion.height) , IPL_DEPTH_8U , 3 );
	newrt_edge= cvCreateImage( cvSize(initRegion.width,initRegion.height) , IPL_DEPTH_8U , 1 );
	newrt_otsu= cvCreateImage( cvSize(initRegion.width,initRegion.height) , IPL_DEPTH_8U , 1 );
	newrt_dil= cvCreateImage( cvSize(initRegion.width,initRegion.height) , IPL_DEPTH_8U , 1 );			
	newrt_hist= cvCreateImage( cvSize(initRegion.width,initRegion.height) , IPL_DEPTH_8U , 1 );
	newrt_mono= cvCreateImage( cvSize(initRegion.width,initRegion.height) , IPL_DEPTH_8U , 1 );
	newrt_part= cvCreateImage( cvSize(initRegion.width,initRegion.height) , IPL_DEPTH_8U , 3);
	int *  profileArray = (int*)malloc(sizeof(int)*initRegion.width);
	int *  position = (int*)malloc(sizeof(int)*2);
	int *  colour = (int*)malloc(sizeof(int)*3);
	int i,j,k,l=0;
	//starting of box logic


			int *  arr_box = (int*)malloc(sizeof(int)*20);
			for(i=0;i<20;i++)
			*(arr_box+i)=0;
			CvRect * wordarrprev= (CvRect*)malloc(sizeof(CvRect)*20);
			CvRect * wordarrcurr= (CvRect*)malloc(sizeof(CvRect)*20);
			CvRect * wordarrtrack= (CvRect*)malloc(sizeof(CvRect)*20);
			


	//	end of box logic
	int*start;int*end;int background;
	int box_height;int space_width;
	box_height = initRegion.height;

if(box_height<50)
space_width=9;
if(box_height<40)
space_width=7;
if(box_height<30)
space_width=5;
if(box_height<15)
space_width=3;

int*arrnumcurr;int *arrnumprev;
int *niramcurr= (int*)malloc(sizeof(int)*20);
int *niramprev= (int*)malloc(sizeof(int)*20);



for(i=0;i<20;i++)
{

*(niramprev+i)=i;
(*(wordarrprev+i)).x      =0;
(*(wordarrprev+i)).y      =0;
(*(wordarrprev+i)).height =0;
(*(wordarrprev+i)).width  =0;

}

*arrnumprev=0;




	while(frameCount<1000)
	{

for(i=0;i<20;i++)
{


(*(wordarrcurr+i)).x=0;
(*(wordarrcurr+i)).y=0;
(*(wordarrcurr+i)).height=0;
(*(wordarrcurr+i)).width=0;
*(niramcurr+i)=0;
}
*arrnumcurr=0;

		
		inImg= cvQueryFrame( capture );//destroy image in loop
		if( !inImg ) break;
		
		cvSetImageROI(inImg,initRegion);
		cvCopy(inImg,newrt);
		//starting current full
		cvCvtColor(newrt,newrt_mono,CV_RGB2GRAY);
		copy_newrt_img=newrt;
		cvThreshold(newrt_mono, newrt_otsu, 128, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		

		//cvCanny(newrt_otsu,newrt_edge, 240, 240, 3);
		//blurimage(newrt_otsu,newrt_dil,0,1);
		histedge(  profileArray , newrt_otsu, newrt_hist,initRegion.width,initRegion.height);
boxdraw(profileArray ,newrt_dil,copy_newrt_img,initRegion.width,initRegion.height,space_width,colour,arr_box,background,wordarrcurr,arrnumcurr);
//ending current full

//exception handling
if(frameCount<5)
{


for(i=0;i<20;i++)
{


(*(wordarrprev+i)).x      =(*(wordarrcurr+i)).x;
(*(wordarrprev+i)).y      =(*(wordarrcurr+i)).y;
(*(wordarrprev+i)).height =(*(wordarrcurr+i)).height;
(*(wordarrprev+i)).width  =(*(wordarrcurr+i)).width;
*(niramprev+i)=i;
}






}
//exception of frame ends here



		
		cvWaitKey(66);
//starting at 1 clock
IplImage* boxmatrix=NULL;
		
		
int row=(*arrnumprev);
int col=(*arrnumcurr);
boxmatrix =cvCreateImage( cvSize(20,20) , IPL_DEPTH_8U , 1 );
for(i=0;i<20*20;i++)
*(boxmatrix-> imageData+i)=0;

//printf("This is %dt frame\n",frameCount);

		for(i=0;i<row;i++)

		{

int widthprev=(*(wordarrprev+i)).width ;
int heightprev=(*(wordarrprev+i)).height ;
int xprev = (int)(*(wordarrprev+i)).x;
int yprev = (int)(*(wordarrprev+i)).y;
//printf("\n checking %d box among prev frame midposition==  (%d) \n",i,(int)(xprev+widthprev/2));
int midprev = (int)(xprev+widthprev/2);
		


		k=0;
		for(j=0;j<col;j++)

		{

int widthcurr=(*(wordarrcurr+j)).width ;
int heightcurr=(*(wordarrcurr+j)).height ;
int xcurr= (int)(*(wordarrcurr+j)).x;
int ycurr = (int)(*(wordarrcurr+j)).y;
int midcurr = (int)(xcurr+widthcurr/2);
//printf("\t %dth midvalue==(%d)\t",j,(int)(xcurr+widthcurr/2));

if( (xcurr <=xprev+widthprev/2 )&& (xprev+widthprev/2<=xcurr+widthcurr) )

{*(boxmatrix-> imageData+i*row+j)=255;
k=k+1;
*(niramcurr+j)=*(niramprev+i);

}
else
*(boxmatrix-> imageData+i*row+j)=0;
		}

//if(k==0)
//printf("tracked it");
}



for(i=0;i<col;i++)
{if(*(niramcurr+i)==0)
*(niramcurr+i)=(  (*(niramcurr+i-1))+1 )%7;
//printf("\nniram of box%d===%d",i,*(niramcurr+i));
}

//ending 1 pm

colourbox(copy_newrt_img,initRegion.width,initRegion.height,wordarrcurr,arrnumcurr,niramcurr);
//copying previous details

for(i=0;i<20;i++)
{


(*(wordarrprev+i)).x      =(*(wordarrcurr+i)).x;
(*(wordarrprev+i)).y      =(*(wordarrcurr+i)).y;
(*(wordarrprev+i)).height =(*(wordarrcurr+i)).height;
(*(wordarrprev+i)).width  =(*(wordarrcurr+i)).width;
*(niramprev+i)=*(niramcurr+i);
}

*arrnumprev=*arrnumcurr;

//end of coppying



		
		cvShowImage("Output",copy_newrt_img);
		cvReleaseImage(&boxmatrix);
		cvCopy(newrt,temp_img);
		cvWriteFrame(video,newrt);
		frameCount += 1;
		cvResetImageROI(inImg);
	}
	//cvReleaseImage(&inImg);
	
       cvReleaseVideoWriter(&video);
	cvReleaseCapture(&capture);
	
	return( 0 );
	
}
		
	
