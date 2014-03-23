// Header Includes
#include <opencv2/opencv.hpp>
#include <OpenNI.h>
#include <iostream>
#include <time.h>

#define DEFAULT_FRAME_LIMIT 9000

#define WIDTH 640
#define HEIGHT 480
#define FOV_H	1.022600
#define FOV_V	0.796616

// Namespaces
using namespace std;

openni::RGB888Pixel* colorImgRaw;
openni::DepthPixel* depthImgRaw ;


int main( const int argc, const char* argv[] )
{
	// Generate output filenames using current date/time
	char *FileName = (char*)malloc(sizeof(char) * 200);
	time_t RawTime;
	struct tm * CurrentDateTime;
	char CurrentDateTimeString [100];

	time(&RawTime);
	CurrentDateTime = localtime(&RawTime);
	strftime(CurrentDateTimeString, 20, "%Y-%m-%d_%H%M%S", CurrentDateTime);
	sprintf(FileName, "Output/data_%s.dat", CurrentDateTimeString);

	// Output depth map to file
	FILE *DataFile = fopen(filename, "wb");

	// Determine the frame limit. If none was specified via command argument, use the default defined above
	int FrameLimit;	
	if(argc == 2)
	{
		FrameLimit = atoi(argv[1]);
	}		
	else
	{
		FrameLimit = DEFAULT_FRAME_LIMIT;
	}	


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

	/*Set Depth and Color Synchronization*/
	ret = device.setDepthColorSyncEnabled(TRUE);
	if (~( ret == openni::STATUS_OK )){
				cout << "Can't sync depth and color" << endl;
	}


	// Create Depth Image
	ret = depth.create( device, openni::SENSOR_DEPTH );
	openni::VideoMode dMode = depth.getVideoMode();	
	dMode.setResolution(WIDTH, HEIGHT);
	depth.setVideoMode(dMode);
	depth.setMirroringEnabled(!depth.getMirroringEnabled());
	if ( ret == openni::STATUS_OK )
	{
		// Start Depth
		depth.start();
	}

	// Create Color Image
	ret = color.create( device, openni::SENSOR_COLOR );	
	openni::VideoMode cMode = color.getVideoMode();	
	cMode.setResolution(WIDTH, HEIGHT);
	color.setVideoMode(cMode);
	color.setMirroringEnabled(!color.getMirroringEnabled());

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

	// Get FPS Information
	cout << "Color : " << color.getVideoMode().getFps() << "(fps) | Depth : " << depth.getVideoMode().getFps() << "(fps)" << endl;

	
	//Set Image Registration Mode (Depth to color)
	ret = device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	if (~( ret == openni::STATUS_OK )){
		cout << "Can't set depth to color registration" << endl;
	}

	int startRecording = 0;

	// Loop
	for (int i=0; i <FrameLimit;i++)
	{		
		// Read a Frame from VideoStream
		color.readFrame( &colorFrame );
		depth.readFrame( &depthFrame );

		colorImgRaw = (openni::RGB888Pixel*)colorFrame.getData();
		depthImgRaw = (openni::DepthPixel*)depthFrame.getData();

		fwrite(colorImgRaw, 3, cImgWidth * cImgHeight, DataFile);
		fwrite(&depthImgRaw[0], sizeof(short), cImgWidth * cImgHeight, DataFile);
		printf("Recording frame #%d\r", i);
			
	}
	printf("\nDone");

	// Close File streams
	fclose(DataFile);

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
