//
//  xSpectrumCocoaView.h
//  xSpectrum
//
//  Created by Rohan Jyoti on 04/20/13.
//  Copyright (c) 2013 Rohan Jyoti. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>

#import "xSpectrum.h"
#import "xSpectrumView.h"

/*
 Apple Developer's NOTE: 
 It is important to rename ALL ui classes when using the XCode Audio Unit with Cocoa View
 template. Cocoa has a flat namespace, and if you use the default filenames, it is possible
 that you will get a namespace collision with classes from the cocoa view of a previously 
 loaded audio unit.	We recommend that you use a unique prefix that includes the manufacturer 
 name and unit name on all objective-C source files. You may use an underscore in your name, 
 but please refrain from starting your class name with an undescore as these names are 
 reserved for Apple.					
 */

@interface xSpectrumCocoaView : NSView
{
    // IB Members
    IBOutlet xSpectrumView*	uixSpectrumView; 
	// if drawing multichannel, would have several
	
    AudioUnit mAU;
	AUEventListenerRef mAUEventListener;
	xSpectrumData *mData;	
	NSTimer* mFetchTimer;
}

//the following functions are required CoreAudio/AudioUnit functions
#pragma mark ___REDRAW___

- (void) updateSpectrum: (NSTimer*) time;

#pragma mark ____ PUBLIC FUNCTIONS ____
- (void)setAU:(AudioUnit)inAU;

#pragma mark ____ PRIVATE FUNCTIONS
- (void)priv_initBuffers;
- (void)priv_synchronizeUIWithParameterValues;
- (void)priv_addListeners;
- (void)priv_removeListeners;

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
- (void)priv_eventListener:(void *) inObject event:(const AudioUnitEvent *)inEvent value:(Float32)inValue;


@end
