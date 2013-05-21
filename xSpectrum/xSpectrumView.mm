//
//  xSpectrumView.mm
//  xSpectrum
//
//  Created by Rohan Jyoti on 04/20/13.
//  Copyright (c) 2013 Rohan Jyoti. All rights reserved.
//

#import "xSpectrumView.h"

//Macro for NS object converion to a Core Image Graph object
#define NSRectToCGRect(orig) CGRectMake(orig.origin.x, orig.origin.y, orig.size.width, orig.size.height)

typedef struct
{
	float red;
	float green;
	float blue;
} mRGB;


@implementation xSpectrumView


/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::createColorMap       *
 * @parameters:     UInt32                              *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        creates GUI                         *
 *******************************************************/

#pragma mark __Coloring__
- (void) createColorMap: (UInt32) mapSize
{
	//Instantiate the colormap
	xColorMapSize = mapSize;
	if( xColorMap)
		free(xColorMap);
	xColorMap = (Float32*)calloc(3*mapSize, sizeof(Float32));
	
	//Initialize the colormap
		//Background
	Float32 mRed = [xBackgroundColor redComponent];
	Float32 mGreen = [xBackgroundColor greenComponent];
	Float32 mBlue = [xBackgroundColor blueComponent];
	
		//Line
	Float32 lRed = [xLineColor redComponent];
	Float32 lGreen = [xLineColor greenComponent];
	Float32 lBlue = [xLineColor blueComponent];
	
		//Increments
	Float32 incrementRed = (lRed - mRed)/mapSize;
	Float32 incrementGreen = (lGreen - mGreen)/mapSize;
	Float32 incrementBlue = (lBlue - mBlue)/mapSize;
	
	UInt32 mapIndex;
	for(UInt32 i=0; i<mapSize; i++)
	{
		mapIndex = i*3; //we mulitply by 3, cuz each index represents color component
		xColorMap[mapIndex] = mRed;
		xColorMap[mapIndex] = mGreen;
		xColorMap[mapIndex] = mBlue;
		
		//increment color map components
		mRed += incrementRed;
		mGreen += incrementGreen;
		mBlue += incrementBlue;
	}
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::skewColor            *
 * @parameters:     Float32                             *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        color manipulation                  *
 *******************************************************/

- (UInt32) skewColor: (Float32) skewFactor
{
	UInt32 position = (UInt32)(skewFactor * (xColorMapSize-1));
	return(position);
}



/********************************************************************
 * @author:         Rohan Jyoti                                     *
 * @name:           xSpectrumView::getIndex withMin andMax andScale *
 * @parameters:     Float32, Float32, Float32, Float32              *
 * @return:         UInt32                                          *
 * @type:           public                                          *
 * @purpose:        retrieves index from suppled frame attr         *
 *******************************************************************/

- (UInt32)	getIndex: (Float32) value
			withMin: (Float32) minVal
			andMax: (Float32) maxVal
			andScale: (Float32) scaleFactor
{
	if(maxVal < 0.001) return 0;
	
	value = value / scaleFactor;
	
	Float32 mMax = 20.0 * log10(1.0 + maxVal);
	Float32 mMin = 20.0 * log10(1.0 + minVal);
	Float32 mIndex = 20.0 * log10(1.0 + value);
	
	Float32 db;
	db = (mIndex <= mMin) ? mMin : mIndex;
	db = (mIndex >= mMax) ? mMax : mIndex;
	
	Float32 ratio;
	ratio = db / mMax;
	UInt32 skewRatio = [self skewColor: ratio];
	
	return skewRatio;
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::getColorWithIndex    *
 * @parameters:     UInt32                              *
 * @return:         mRGB struct                         *
 * @type:           public                              *
 * @purpose:        gets color component                *
 *******************************************************/

- (mRGB) getColorWithIndex: (UInt32) index
{
	mRGB returnStruct;
	UInt32 returnIndex = index*3; //recall that we *3 cuz each triplet indexes --> components
	returnStruct.red = xColorMap[returnIndex];
	returnStruct.green = xColorMap[returnIndex];
	returnStruct.blue = xColorMap[returnIndex];
	return returnStruct;
}


#pragma mark __Initialize__

/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::isStaticView         *
 * @parameters:     BOOL                                *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        verifies stream play/pause          *
 *******************************************************/

- (void) setStaticView:(BOOL)isStatic
{
	isStaticView = isStatic;
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::initializeBuffers    *
 * @parameters:     none                                *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        initialize necessary buffers        *
 *******************************************************/

- (void) initializeBuffers
{
	//colorMap represents the background and foreground colors
	//we init to mNumBins because we want to be able to specify a different color
	//for every sample frequency (optional). This will be necessary for creating
	//a more aesthetically pleasing GUI/visualization
	[self createColorMap:mNumBins];
	
	UInt32 numBytesPerFrame = mNumBins * 4 * sizeof(unsigned char); //3 for rgb, 1 for frame
	
	//mRingBuffer
	if(mRingBuffer) delete(mRingBuffer);
	mRingBuffer = new CARingBuffer();
	mRingBuffer->Allocate(1, numBytesPerFrame, mNumSlicesTotal);
	mRingBufferCounter = 0;
	mRenderCounter = 0;
	
	UInt32 numTotalBytes = numBytesPerFrame*mNumSlicesTotal;
	
	//clientDescrption is used for both fetching and storing
	CAStreamBasicDescription bufferClientDescription;
	bufferClientDescription.SetCanonical(1, false);
	
	//mFetchingBufferList
	if(mFetchingBufferList)
	{
		mFetchingBufferList->DeallocateBuffers();
		delete(mFetchingBufferList);
	}
	mFetchingBufferList = CABufferList::New("FetchingBufferList", bufferClientDescription);
	mFetchingBufferList->AllocateBuffers(numTotalBytes);
	
	//mStoringBufferList
	if(mStoringBufferList)
	{
		mStoringBufferList->DeallocateBuffers();
		delete(mStoringBufferList);
	}
	mStoringBufferList = CABufferList::New("StoringBufferList", bufferClientDescription);
	mStoringBufferList->AllocateBuffers(numTotalBytes);
	
	storing = false; //becasue we have not actually stored any sample frames yet
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::Initialize           *
 * @parameters:     none                                *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        Initialize GUI elements             *
 *******************************************************/

- (void) Initialize //Cocoa override
{
	xBackgroundColor = [[NSColor colorWithCalibratedRed: 0 green: 0 blue: 0 alpha:1]retain];
	xLineColor  = [[NSColor colorWithCalibratedRed: 1 green: 1 blue: 1 alpha: 1]retain];
	
	// recall that well is an iboutlet that actualy controls the gui colors per timeframe
	[xBackgroundWell setColor:xBackgroundColor];
	[xLineWell setColor:xLineColor];
	
	// recall that slices is essentially a frame and that a bin is each division of freq
	mNumSlicesTotal = (UInt32) ([self frame].size.width);
	mNumBinsTotal = (UInt32) ([self frame].size.height);
	
	mRingBufferCounter = 0;
	mRenderCounter = 0;
	
	[self initializeBuffers];
	isStaticView = false;
	
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::initWithFrame        *
 * @parameters:     NSRect                              *
 * @return:         id                                  *
 * @type:           public                              *
 * @purpose:        Initialize GUI elements             *
 *******************************************************/

- (id)initWithFrame:(NSRect)frame //cocoa required
{
    self = [super initWithFrame:frame]; //call to cocoa lib
    if (self) 
	{
        [self Initialize]; 
    }
    
    return self;
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::dealloc              *
 * @parameters:     none                                *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        frees memory used by gui            *
 *******************************************************/

- (void) dealloc //cocoa deallocation overide
{
	if (xBackgroundColor) [xBackgroundColor release]; //release is the same as delete, 
	//but we use release for NS cocoa objects
	if (xLineColor) [xLineColor release];
	
	if(xColorMap) free(xColorMap);; //double semi here indicates, end entire func if this condition is met
	
	if(mRingBuffer) delete(mRingBuffer);
	if(mFetchingBufferList) delete(mFetchingBufferList);
	if(mStoringBufferList) delete(mStoringBufferList);
	
	[super dealloc];
}

#pragma mark __IBActions__

/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::setLineColor         *
 * @parameters:     NSColor*                            *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        sets the line color                 *
 *******************************************************/

- (void) setLineColor:(NSColor*) color;
{
	if(color != xLineColor)
	{
		[color retain];
		[xLineColor release];
		xLineColor = color;
	}
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::setBackgroundColor   *
 * @parameters:     NSColor*                            *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        sets the background color           *
 *******************************************************/

- (void) setBackgroundColor:(NSColor*) color
{
	if(color != xBackgroundColor)
	{
		[color retain];
		[xBackgroundColor release];
		xBackgroundColor = color;
	}
}



/**********************************************************
 * @author:         Rohan Jyoti                           *
 * @name:           xSpectrumView::changeBackgroundColor  *
 * @parameters:     id                                    *
 * @return:         IBAction                              *
 * @type:           public                                *
 * @purpose:        setBackgroundColor wrapper            *
 *********************************************************/

- (IBAction) changeBackgroundColor:(id)sender
{
	[self setBackgroundColor: [sender color]];
	[self createColorMap: mNumBins];
	[self setNeedsDisplay:YES];
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::changeLineColor      *
 * @parameters:     id                                  *
 * @return:         IBAction                            *
 * @type:           public                              *
 * @purpose:        setLineColor wrapper                *
 *******************************************************/

- (IBAction) changeLineColor:(id)sender
{
	[self setLineColor: [sender color]];
	[self createColorMap: mNumBins];
	[self setNeedsDisplay:YES];
}


#pragma mark __Reading__


/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::storing              *
 * @parameters:     none                                *
 * @return:         bool                                *
 * @type:           public                              *
 * @purpose:        returns sotring value               *
 *******************************************************/

- (bool) storing
{
	return storing;
}


#pragma mark __Drawing__

/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::storeSlices          *
 * @parameters:     xSpectrumData*                      *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        stores data incase of pause         *
 *******************************************************/

- (void) storeSlices:(xSpectrumData *)data
{
	//this function is for when audio stream stops, we want to store the current data
	//so that in the future we can implement a gradual fade away effect
	storing = true;
	
	if(mNumBins != data->mNumBins)
	{
		mNumBins = data->mNumBins;
		[self initializeBuffers];
	}
	
	mNumSlices = data->mNumSlices;
	
	Float32 xMinAmp = data->mMinAmp;
	Float32 xMaxAmp = data->mMaxAmp;
	
	UInt32 numBytesPerRow = mNumBins * 4 * sizeof(unsigned char);
	
	AudioBufferList* xStoringList = &mStoringBufferList->GetModifiableBufferList();
	unsigned char* bitmapImageSliceBits = (unsigned char*) xStoringList->mBuffers[data->mChannel].mData;
	
	Float32 xMag;
	UInt32 colorIndex, binIndex, bmpSliceIndex;
	mRGB colors;
	
	for(UInt32 j = 0; j < mNumSlices; j++) //for each frame
	{
		for(UInt32 i =0; i < mNumBins; i++) //for each frequency
		{
			binIndex = i + j*mNumBins;
			xMag = data->mData[binIndex];
			
			if(isnan(xMag)) xMag = 0.0;
			
			colorIndex = [self getIndex: xMag withMin: xMinAmp andMax: xMaxAmp andScale: 1.0];
			colors = [self getColorWithIndex: colorIndex];
			
			bmpSliceIndex = i*4 + j*numBytesPerRow;
			
			bitmapImageSliceBits[bmpSliceIndex  ] = 255;
			bitmapImageSliceBits[bmpSliceIndex+1] = (unsigned char) (255*colors.red);
			bitmapImageSliceBits[bmpSliceIndex+2] = (unsigned char) (255*colors.green);
			bitmapImageSliceBits[bmpSliceIndex+3] = (unsigned char) (255*colors.blue);
		}
	}
	
	//store the data
	mRingBuffer->Store(xStoringList, mNumSlices, mRingBufferCounter);
	if(!isStaticView)
	{
		mRingBufferCounter += mNumSlices;
	}
	
	[self setNeedsDisplay:YES];
	
	storing = false;
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumView::drawRect             *
 * @parameters:     NSRect                              *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        does actual drawing of GUI          *
 *******************************************************/

- (void)drawRect:(NSRect)rect //cocoa required
{
    // Drawing code goes here
	[super drawRect: rect];
	
	if(!mRingBuffer) return; //meaning there is no data to draw
	AudioBufferList* fetchBufferList = &mFetchingBufferList->GetModifiableBufferList();
	
	SInt64 renderNum = mRingBufferCounter - mRenderCounter;
	mRingBuffer->Fetch(fetchBufferList, mNumSlicesTotal, mRingBufferCounter);
	if(!isStaticView)
		mRenderCounter += renderNum;
	
	unsigned char* bitmapImageBits = (unsigned char*) fetchBufferList->mBuffers[0].mData;
	
	UInt32 numBytesPerRow = mNumBins * 4 * sizeof(unsigned char);
	UInt32 numTotalBytes = mNumSlicesTotal*numBytesPerRow;
	
	NSData* bitmapData = [[NSData alloc] initWithBytesNoCopy:bitmapImageBits length: numTotalBytes freeWhenDone:NO];
	
	CIImage* xSpectrumImage = [[CIImage alloc] initWithBitmapData : bitmapData
													  bytesPerRow : numBytesPerRow 
															 size :  CGSizeMake( (Float32) mNumSlicesTotal, (Float32) mNumBins)
														   format : kCIFormatARGB8
													   colorSpace : CGColorSpaceCreateDeviceRGB()
							   ];
	
	NSRect drawRect = rect;
	
	NSRect xImageRect = NSMakeRect(0, 0, mNumSlicesTotal, mNumBins);
	
	//CoreImage Implementation
	NSAffineTransform* yFlip = [NSAffineTransform transform];
	[yFlip rotateByDegrees:90];
	[yFlip translateXBy: 0.0 yBy:-xImageRect.size.height];
	
	CIFilter* flipYTransform = [CIFilter filterWithName:@"CIAffineTransform"];
	[flipYTransform setValue: yFlip forKey: @"inputTransform"];
	[flipYTransform setValue: xSpectrumImage forKey:@"inputImage"];
	CIImage* affineOutputImage = [flipYTransform valueForKey: @"outputImage"];
	
	//default write out
	[affineOutputImage drawInRect: drawRect 
					   fromRect: xImageRect 
					   operation:NSCompositeCopy 
					   fraction:1.0];
	
	[bitmapData release];
	[xSpectrumImage release];
	
	
}

@end
