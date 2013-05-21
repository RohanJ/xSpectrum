//
//  xSpectrumView.h
//  xSpectrum
//
//  Created by Rohan Jyoti on 04/20/13.
//  Copyright (c) 2013 Rohan Jyoti. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

#import "CARingBuffer.h"
#import "CABufferList.h"

#import "xSpectrumSharedData.h"

@interface xSpectrumView : NSView
{
	//interface is Obj-C/C++ equivalent to C++ class
	//Difference is that within brackets, you only declare variables,
	//functions come after; in essence, entire file is treated as 
	//virtual class
	
	//Wells are container objects for gui elements
	IBOutlet NSColorWell* xBackgroundWell;
	IBOutlet NSColorWell* xLineWell;
	
	NSColor* xBackgroundColor;
	NSColor* xLineColor;
	Float32* xColorMap;
	UInt32 xColorMapSize;
	
	CARingBuffer* mRingBuffer;
	SInt64 mRingBufferCounter;
	
	//Current and Total bins slices/frames written
	UInt32 mNumBins;
	UInt32 mNumSlices;
	UInt32 mNumBinsTotal;
	UInt32 mNumSlicesTotal;
	
	SInt64 mRenderCounter;
	
	CABufferList* mFetchingBufferList;
	CABufferList* mStoringBufferList;
	
	bool storing;
	bool isStaticView;
}

//function declrations: Obj-C/C++ prototype --> -/+ (return) name: (type) param.
- (bool) storing;

#pragma mark INTERFACE ACTIONS
- (IBAction) changeBackgroundColor: (id) sender;
- (IBAction) changeLineColor: (id)sender;

#pragma mark EVENTS
- (void) setStaticView: (BOOL) isStatic;
- (void) storeSlices: (xSpectrumData*) data;

@end
