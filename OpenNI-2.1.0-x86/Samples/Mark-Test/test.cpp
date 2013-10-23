// Header Includes
#include <opencv2/opencv.hpp>
#include <OpenNI.h>
#include <iostream>

#define DEPTH_FILE_NAME "depthOut.dat"
#define IMAGE_FILE_NAME "imageOut.dat"

#define RES_X 640
#define RES_Y 480

// Namespaces
using namespace std;

int main( const int argc, const char* argv[] )
{
	// Device
	openni::Device device;

	// VideoStream
	openni::VideoStream color;
	openni::VideoStream depth;

	// Target Device URI
	const char* device_uri = openni::ANY_DEVICE;

	// Initialize OpenNI Module
	openni::Status ret = openni::OpenNI::initialize();
	cout << "OpenNI Initialization Error : " << openni::OpenNI::getExtendedError() << endl;

	// Device Counts & Informations
	openni::Array< openni::DeviceInfo > devicesInfo;
	openni::OpenNI::enumerateDevices( &devicesInfo );
	cout << "Device Counts : " << devicesInfo.getSize() << endl;
	for ( int i = 0 ; i < devicesInfo.getSize() ; i++ )
	{
		cout << "Device Info [ " << i << " ] : " << devicesInfo[i].getName() << endl;
	}

	// Open
	ret =device.open( device_uri );
	if ( ret != openni::STATUS_OK )
	{
		cerr << "Device Open Failed" << endl;
		// Shutdown
		openni::OpenNI::shutdown();
		return EXIT_FAILURE;
	}

	/*Set Image Registration Mode (Depth to color)
	openni::ImageRegistrationMode imgRegMode = openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR;
	ret = device.setImageRegistrationMode(imgRegMode);
	if (~( ret == openni::STATUS_OK )){
		cout << "Can't set depth to color registration" << endl;
	}
	*/
	// Create Depth Image
	ret = depth.create( device, openni::SENSOR_DEPTH );
	openni::VideoMode dMode = depth.getVideoMode();	
	dMode.setResolution(RES_X, RES_Y);
	depth.setVideoMode(dMode);
	if ( ret == openni::STATUS_OK )
	{
		// Start Depth
		depth.start();
	}

	// Create Color Image
	ret = color.create( device, openni::SENSOR_COLOR );	
	openni::VideoMode cMode = color.getVideoMode();	
	cMode.setResolution(RES_X, RES_Y);
	color.setVideoMode(cMode);

	if ( ret == openni::STATUS_OK )
	{
		// Start Color
		color.start();
	}
	

	// Check Valid State
	if ( !color.isValid() || !depth.isValid() )
	{
		cout << "Image Invalid" << endl;
		openni::OpenNI::shutdown();
		return EXIT_FAILURE;
	}

	// Get Color Stream Min-Max Value
	int minColorValue = color.getMinPixelValue();
	int maxColorValue = color.getMaxPixelValue();
	cout << "Color min-Max Value : " << minColorValue << "-" << maxColorValue << endl;

	// Get Depth Stream Min-Max Value
	int minDepthValue = depth.getMinPixelValue();
	int maxDepthValue = depth.getMaxPixelValue();
	cout << "Depth min-Max Value : " << minDepthValue << "-" << maxDepthValue << endl;

	// Get Sensor Resolution Information
	int dImgWidth = depth.getVideoMode().getResolutionX();
	int dImgHeight = depth.getVideoMode().getResolutionY();
	int cImgWidth = color.getVideoMode().getResolutionX();
	int cImgHeight = color.getVideoMode().getResolutionY();
	cout << "Color Resolution : " << cImgWidth << "x" << cImgHeight << endl;
	cout << "Depth Resolution : " << dImgWidth << "x" << dImgHeight << endl;

	// Frame Information Reference
	openni::VideoFrameRef colorFrame;
	openni::VideoFrameRef depthFrame;

	// Color Image & Depth Image Matrix
	cv::Mat cImg = cv::Mat( cImgHeight, cImgWidth, CV_8UC3 );
	cv::Mat dImg = cv::Mat( dImgHeight, dImgWidth, CV_8UC3 );
	cv::Mat dRaw = cv::Mat (dImgHeight, dImgWidth, CV_16UC1 );

	// Get FPS Information
	cout << "Color : " << color.getVideoMode().getFps() << "(fps) | Depth : " << depth.getVideoMode().getFps() << "(fps)" << endl;

	
	// Output depth map to file
	FILE *DepthFile = fopen(DEPTH_FILE_NAME, "wb");
	// Output depth map to file
	FILE *ImageFile = fopen(IMAGE_FILE_NAME, "wb");
	// Loop
	for(;;)
	{
		// Read a Frame from VideoStream
		color.readFrame( &colorFrame );
		depth.readFrame( &depthFrame );

		// Copy To Mat
		openni::RGB888Pixel* colorImgRaw = (openni::RGB888Pixel*)colorFrame.getData();
		for ( int i = 0 ; i < ( colorFrame.getDataSize() / sizeof( openni::RGB888Pixel ) ) ; i++ )
		{
			int idx = i * 3; // cv::Mat is BGR
			unsigned char* data = &cImg.data[idx];
			data[0] = (unsigned char)colorImgRaw[i].b;
			data[1] = (unsigned char)colorImgRaw[i].g;
			data[2] = (unsigned char)colorImgRaw[i].r;
		}

		fwrite(colorImgRaw, 3, cImgWidth * cImgHeight, ImageFile);
		
		openni::DepthPixel* depthImgRaw = (openni::DepthPixel*)depthFrame.getData();
		int lb, ub;
		for ( int i = 0 ; i < ( depthFrame.getDataSize() / sizeof( openni::DepthPixel ) ) ; i++ )
		{
			int idx = i * 3; // Grayscale
			unsigned char* data = &dImg.data[idx];
			
			lb = (depthImgRaw[i]/5) % 256;
			ub = depthImgRaw[i]/5 / 256;

			switch (ub) {
				case 0:
					data[2] = 255;
					data[1] = 255-lb;
					data[0] = 255-lb;
					break;
				case 1:
					data[2] = 255;
					data[1] = lb;
					data[0] = 0;
					break;
				case 2:
					data[2] = 255-lb;
					data[1] = 255;
					data[0] = 0;
					break;
				case 3:
					data[2] = 0;
					data[1] = 255;
					data[0] = lb;
					break;
				case 4:
					data[2] = 0;
					data[1] = 255-lb;
					data[0] = 255;
					break;
				case 5:
					data[2] = 0;
					data[1] = 0;
					data[0] = 255-lb;
					break;
				default:
					data[2] = 0;
					data[1] = 0;
					data[0] = 0;
					break;
			}

			/*
			int gray_scale = ( ( depthImgRaw[i] * 255 ) / ( maxDepthValue - minDepthValue ) );
			data[0] = (unsigned char)~gray_scale;
			data[1] = (unsigned char)~gray_scale;
			data[2] = (unsigned char)~gray_scale;
			*/
		}
		
		fwrite(depthImgRaw, sizeof(openni::DepthPixel), cImgWidth * cImgHeight, DepthFile);

		// Show Images
		cv::imshow( "depth", dImg );
		cv::imshow( "color", cImg );

		FILE *pcl = fopen("data.pcl", "wb");
		int numPoints  = cImgWidth * cImgHeight;
		fwrite(&numPoints, sizeof(int), 1, pcl);
		fwrite(&cImgWidth, sizeof(int), 1, pcl);
		fwrite(&cImgHeight, sizeof(int), 1, pcl);
		/*
		float *worldPoints;
		worldPoints = (float*) malloc(3*dImgWidth*dImgHeight*sizeof(float));
		
		for (int v = 0; v < cImgHeight; v++) {
			for (int u = 0; u < cImgWidth; u++) {
				int x, y;
			
				openni::CoordinateConverter::convertDepthToWorld(depth,u,v,depthImgRaw[v*dImgWidth+u], &worldPoints[v*dImgWidth+u+0], &worldPoints[v*dImgWidth+u+1], &worldPoints[v*dImgWidth+u+2]);
				openni::CoordinateConverter::convertDepthToColor(depth, color,u,v,depthImgRaw[v*dImgWidth+u],&x,&y);
				fwrite(&worldPoints[v*dImgWidth+u], sizeof(float), 3, pcl);
				fwrite(&colorImgRaw[y*dImgWidth+x], sizeof(char), 3, pcl);
				//fwrite(colorImgRaw, 3, cImgWidth * cImgHeight, pcl);
	
			}
		}
		
		fclose(pcl);
		*/
		int k = cvWaitKey( 30 );		// About 30fps
		if ( k == 0x1b )					// Exit By ESC
			break;
	}

	// Close File streams
	fclose(ImageFile);
	fclose(DepthFile);

	// Destroy Streams
	color.destroy();
	depth.destroy();
	// Close Device
	device.close();
	// Shutdown OpenNI
	openni::OpenNI::shutdown();

	// Return
	return EXIT_SUCCESS;
}
