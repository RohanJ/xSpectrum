//
//  xSpectrum.cpp
//  xSpectrum
//
//  Created by Rohan Jyoti on 04/20/13.
//  Copyright (c) 2013 Rohan Jyoti. All rights reserved.
//

#include "xSpectrum.h"

//AudioUnit Entry-Point definition for integration with AudioUnit Hosting Platform
//It serves as the virtual main function call
AUDIOCOMPONENT_ENTRY(AUBaseFactory, xSpectrum)


/* General Acronyms:
 
 AU        =        AudioUnit
 AUHS      =        AudioUnit Hosting Service
 #pragma   =        Additional GCC optimization directives
                    These directives are more efficient than C/C++
                    preprocessor counterpart
 CA        =        CoreAudio
 CI        =        CoreImage
 
*/

/* Programmer Notes
 
 The functions in this file MUST be in this file. For AudioUnit processing to work
 correctly, these function cannot be dispersed throughout various classes. A majority
 of the functions are NECESSARY AudioUnit overrides with defined entrypoints. 
 Although wrapper functions can be used pointing to functions in other classes, it would defeat the purpose.
 
*/



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrum::xSpectrum                *
 * @parameters:     AudioUnit                           *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        default constructor                 *
 *******************************************************/

xSpectrum::xSpectrum(AudioUnit component) : AUEffectBase(component, true) //def. constructor
{
	mSpectrumBuffer = NULL;
	mFetchingBufferList = NULL;
	mSpectralDataBufferList = NULL;
	mMinAmp = NULL;
	mMaxAmp = NULL;
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrum::~xSpectrum               *
 * @parameters:     none                                *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        virtual destructor                  *
 *******************************************************/

xSpectrum::~xSpectrum() //virtual destructor
{
	mSpectralProcessor.free();
	if(mSpectrumBuffer)
	{
		delete(mSpectrumBuffer);
		mSpectrumBuffer = NULL;
	}
	
	if(mFetchingBufferList)
	{
		delete(mFetchingBufferList);
		mFetchingBufferList = NULL;
	}
	
	if(mSpectralDataBufferList)
	{
		delete(mSpectralDataBufferList);
		mSpectralDataBufferList = NULL;
	}
	
	if(mMinAmp)
	{
		free(mMinAmp);
		mMinAmp = NULL;
	}
	
	if(mMaxAmp)
	{
		free(mMaxAmp);
		mMaxAmp = NULL;
	}
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrum::init                     *
 * @parameters:     none                                *
 * @return:         OSStatus                            *
 * @type:           public                              *
 * @purpose:        initializes audio unit              *
 *******************************************************/

OSStatus xSpectrum::init()
{
	OSStatus result = AUEffectBase::Initialize();
	
	if(result == noErr) //recall that noErr is a CoreAudio macro
	{
		allocateBuffers();
	}
	
	return result;
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrum::GetParameterValueStrings *
 * @parameters:     AudioUnitScope,                     * 
 *                  AudioUnitParameterID                *
 *                  CFArrayRef *                        *
 * @return:         OSSatus                             *
 * @type:           public                              *
 * @purpose:        default AU Value Strings            *
 *******************************************************/

OSStatus xSpectrum::GetParameterValueStrings(AudioUnitScope			inScope,
											 AudioUnitParameterID	inParameterID,
											 CFArrayRef *			outStrings)
{
	return kAudioUnitErr_InvalidProperty;
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrum::GetParameterInfo         *
 * @parameters:     AudioUnitScope,                     * 
 *                  AudioUnitParameterID                *
 *                  AudioUnitParameterInfo              *
 * @return:         OSStatus                            *
 * @type:           public                              *
 * @purpose:        default AU Parameter Info           *
 *******************************************************/

OSStatus xSpectrum::GetParameterInfo(AudioUnitScope				inScope,
									 AudioUnitParameterID		inParameterID,
									 AudioUnitParameterInfo		&outParameterInfo )
{
	OSStatus result = noErr;
	
	outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable
					 |		 kAudioUnitParameterFlag_IsReadable;
    
    if (inScope == kAudioUnitScope_Global)
	{
        switch(inParameterID)
        {
				
            default:
                result = kAudioUnitErr_InvalidParameter;
                break;
		}
	} 
	else
	{
        result = kAudioUnitErr_InvalidParameter;
    }
	
	return result;
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrum::GetPropertyInfo          *
 * @parameters:     AudioUnitPropertyID                 * 
 *                  AudioUnitScope                      *
 *                  AudioUnitElement                    *
 *                  UInt32 &                            *
 *                  Boolean &                           *
 * @return:         OSStatus                            *
 * @type:           public                              *
 * @purpose:        default AU Property Info            *
 *******************************************************/

OSStatus xSpectrum::GetPropertyInfo (AudioUnitPropertyID		inID,
									 AudioUnitScope				inScope,
									 AudioUnitElement			inElement,
									 UInt32 &					outDataSize,
									 Boolean &					outWritable)
{
	if (inScope == kAudioUnitScope_Global)
	{
		switch (inID) 
		{
			case kAudioUnitProperty_CocoaUI:
				outWritable = false;
				outDataSize = sizeof (AudioUnitCocoaViewInfo);
				return noErr;
				
			case kAudioUnitProperty_SampleTimeStamp: //value from xSpectrumSharedData
				outWritable = false;
				outDataSize = sizeof(Float64*);
				return noErr;
				
			case kAudioUnitProperty_xSpectrumData: //value from xSpectrumSharedData
				outWritable = true;
				outDataSize = sizeof(xSpectrumData);
				return noErr;
		}
	}
	return AUEffectBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrum::GetProperty              *
 * @parameters:     AudioUnitPropertyID                 * 
 *                  AudioUnitScope                      *
 *                  AudioUnitElement                    *
 *                  void *                              *
 * @return:         OSStatus                            *
 * @type:           public                              *
 * @purpose:        AU Property used for AU ID in AUHS  *
 *******************************************************/

OSStatus xSpectrum::GetProperty(AudioUnitPropertyID		inID,
								AudioUnitScope			inScope,
								AudioUnitElement		inElement,
								void *					outData)
{
	if (inScope == kAudioUnitScope_Global) 
	{
		switch (inID) 
		{
			case kAudioUnitProperty_CocoaUI:
			{
				// Look for a resource in the main bundle by name and type.
				CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("com.audiounit.xSpectrum") );
				
				if (bundle == NULL) return fnfErr;
                
				CFURLRef bundleURL = CFBundleCopyResourceURL( bundle, 
                    CFSTR("xSpectrumCocoaViewFactory"), 
                    CFSTR("bundle"), 
                    NULL);
                
                if (bundleURL == NULL) return fnfErr;

				AudioUnitCocoaViewInfo cocoaInfo;
				cocoaInfo.mCocoaAUViewBundleLocation = bundleURL;
				cocoaInfo.mCocoaAUViewClass[0] = CFStringCreateWithCString(NULL, "xSpectrumCocoaViewFactory", kCFStringEncodingUTF8);
				
				*((AudioUnitCocoaViewInfo *)outData) = cocoaInfo;
				
				return noErr;
			}
			case kAudioUnitProperty_xSpectrumData:
			{
				xSpectrumData *data = (xSpectrumData*)outData;
				return getxSpectrumData(data);
			}
			case kAudioUnitProperty_SampleTimeStamp:
			{
				*(static_cast<Float64*>(outData)) = mRenderStamp.mSampleTime;
				return noErr;
			}
		}
	}

	return AUEffectBase::GetProperty (inID, inScope, inElement, outData);
}

#pragma mark CommunicationWithView



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrum::allocateBuffers          *
 * @parameters:     none                                *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        Allocates neces. xSpectrum buffers  *
 *******************************************************/

void xSpectrum::allocateBuffers()
{
	//First we will define blocksize and number of bins
	mBlockSize = 1024;
	mNumBins = mBlockSize>>1; //bit-wise right shift equivalent to divide by 2
	
	
	//mSpectrumBuffer
	if(mSpectrumBuffer) delete(mSpectrumBuffer);
	mSpectrumBuffer = new CARingBuffer();
	mSpectrumBuffer->Allocate(GetNumberOfChannels(), 
							  mNumBins*sizeof(Float32), 
							  kMaxSonogramLatency); 
	//first parameter is from AUEffectsBase, 2nd&3rd internal
	
	
	//mFetchingBufferList
	CAStreamBasicDescription clientDescription;
	clientDescription.SetCanonical(GetNumberOfChannels(), false);
	clientDescription.mSampleRate = GetSampleRate(); 
		//parameter function calls from AUEffectsBase
	UInt32 frameLength = kDefaultValue_BufferSize*sizeof(Float32); //512*512*4=1MB
	if(mFetchingBufferList)
	{
		//it is necessary to deallocate before deletion
		mFetchingBufferList->DeallocateBuffers();
		delete(mFetchingBufferList);
	}
	mFetchingBufferList = CABufferList::New("FetchingBuffer", clientDescription);
	mFetchingBufferList->AllocateBuffers(frameLength);
	
	
	//mSpectralDataBufferList
	if(mSpectralDataBufferList)
	{
		mSpectralDataBufferList->DeallocateBuffers();
		delete(mSpectralDataBufferList);
	}
	mSpectralDataBufferList = CABufferList::New("IntermediateBuffer", clientDescription);
	mSpectralDataBufferList->AllocateBuffers(frameLength);
	
	
	
	//Next we must initialize the audio time stamp (Defined in header) to 0
	//Note that we must use memset because mRenderStamp is a pointer
	memset(&mRenderStamp, 0, sizeof(AudioTimeStamp));
	mRenderStamp.mFlags = kAudioTimeStampHostTimeValid;
	
	
	//mSpectralProcessor
	mSpectralProcessor.free();
		/*SpectralProcessor Constructor
			SpectralProcessor (
				UInt32 inFFTSize,        // The size of the FFT you want to perform
				UInt32 inHopSize,        // The overlap between frames
				UInt32 inNumChannels,    // The number of audio channels to process
				UInt32 inMaxFrames       // the maximum number of frames received per slice
			);
		 */
	mSpectralProcessor = new SpectralProcessor(mBlockSize, mNumBins, GetNumberOfChannels(), GetMaxFramesPerSlice());
	
	if(mMinAmp) free(mMinAmp); //recall that mMinAmp is the lowest frequency in sample frame
	mMinAmp = (Float32*) calloc(GetNumberOfChannels(), sizeof(Float32));
	
	if(mMaxAmp) free(mMaxAmp); //recall that mMaxAmp is the highest frequency in sample frame
	mMaxAmp = (Float32*) calloc(GetNumberOfChannels(), sizeof(Float32));
	
	
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrum::changeStreamFormat       *
 * @parameters:     AudioUnitScope                      *
 *                  AudioUnitElement                    *
 *                  const CAStreamBasicDescription &    *
 *                  const CAStreamBasicDescription &    *
 * @return:         OSStatus                            *
 * @type:           public                              *
 * @purpose:        Inovkes AU stream handler           *
 *                  Allows for variation in sample      *
 *                  rate processing                     *
 *******************************************************/

OSStatus xSpectrum::changeStreamFormat(AudioUnitScope					inScope, 
									   AudioUnitElement					inElement, 
									   const CAStreamBasicDescription &	inPrevFormat, 
									   const CAStreamBasicDescription &	inNewFormat)
{
	return AUBase::ChangeStreamFormat(inScope, inElement, inPrevFormat, inNewFormat);
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrum::getxSpectrumData         *
 * @parameters:     xSpectrumData *                     *
 * @return:         OSStatus                            *
 * @type:           public                              *
 * @purpose:        store data into xSpectrumData       *
 *******************************************************/

OSStatus xSpectrum::getxSpectrumData(xSpectrumData *data)
{
	//here we will extract and store the data elements from the current sample buffer input
	data->mNumBins = mNumBins; 
	data->mMinAmp = mMinAmp[data->mChannel];
	data->mMaxAmp = mMaxAmp[data->mChannel];
	
	//Ensure near-zero latency (else condition would be to upsample/downsample accordingly)
	//however, CoreAudio is much too efficient for this to ever happen; This is just
	//for debugging purposes in the worst-case scenerio
	UInt32 currentNumSlices = data->mNumSlices;
	if(currentNumSlices > kMaxSonogramLatency) 
		return kAudioUnitErr_TooManyFramesToProcess;
	
	//Get the current buffer whilst fetching mNumBins * mNumSlices of data
	AudioBufferList *currentBufferList = &mFetchingBufferList->GetModifiableBufferList();
	SampleTime time = (SampleTime) data->mFetchStamp.mSampleTime;
	mSpectrumBuffer->Fetch(currentBufferList, currentNumSlices, time);
	
	//Extract and set current sample float value
	Float32* currentFloat = (Float32*) currentBufferList->mBuffers[data->mChannel].mData;
	memcpy(data->mData, currentFloat, currentNumSlices*mNumBins*sizeof(Float32));
		//update timeStamp to reflect number of slices (aka frames) processed.
	data->mFetchStamp.mSampleTime += currentNumSlices; 
	return noErr;
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrum::Render                   *
 * @parameters:     AudioUnitRenderActionFlags          *
 *                  const AudioTimeStamp                *
 *                  UInt32                              *
 * @return:         OSStatus                            *
 * @type:           public                              *
 * @purpose:        Invokes SpectralProcessor           *
 *******************************************************/

OSStatus xSpectrum::Render(AudioUnitRenderActionFlags	&ioActionFlags, 
						   const AudioTimeStamp			&inTimeStamp, 
						   UInt32						inFramesToProcess)
{
	//This is where main spectral processing happens;
	
	//In AudioUnits, data is pulled from the Hosting Service
	UInt32 actionFlags = 0;
	OSStatus failedPull = PullInput(0, actionFlags, inTimeStamp, inFramesToProcess);
	if(failedPull) return failedPull; //PullInput returns true on failure (idk why?)
	
	//setup input/output busses, initialized to 0
	AUInputElement* inputBus = GetInput(0); //GetInput with value 0 --> pseudo-init function
	AUOutputElement* outputBus = GetOutput(0); 
	
	//prepare output buffer bus
	outputBus->PrepareBuffer(inFramesToProcess);
	
	//prepae bufferlist for spectral processing
	AudioBufferList& inputBufferList = inputBus->GetBufferList();
	
	//Invoke Spectral Processing
		/*	Since we are not actually performing any digital signal processing, we
			only want to make use for a forward fast fourier transformation;
			that is: Time-Domain to Frequency Domain
		 */
	
		//Prototype: bool ProcessForwards( UInt32 inNumFrames. AudioBufferList* inInput)
	bool validProcessing = mSpectralProcessor->ProcessForwards(inFramesToProcess, &inputBufferList);
	
	if(validProcessing)
	{
		//if fft was successful, we want to perform convolution to get max and min freqs
		AudioBufferList* temp = &mSpectralDataBufferList->GetModifiableBufferList();
		mSpectralProcessor->GetMagnitude(temp, mMinAmp, mMaxAmp); 
		//GetMagnitude will write the min and max freqs into the pointers provided above
		
		SampleTime time2 = (SampleTime)(mRenderStamp.mSampleTime);
			//prototype: mSpectrumBuffer->Store(const AudioBufferList *abl, UInt32 nFrames, SampleTime frameNumber)
		mSpectrumBuffer->Store(temp, 1, time2); //store 1 frame at a time based on timestamp
		
		mRenderStamp.mSampleTime += 1;
	}
	
	return AUEffectBase::Render(ioActionFlags, inTimeStamp, inFramesToProcess);
}

#pragma mark ____xSpectrumEffectKernel



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumKernel::Reset              *
 * @parameters:     none                                *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        Kernel reset override               *
 *******************************************************/

void xSpectrumKernel::Reset()
{
}



/********************************************************
 * @author:         Rohan Jyoti                         *
 * @name:           xSpectrumKernel::Process            *
 * @parameters:     const Float32                       *
 *                  Float32                             *
 *                  UInt32                              *
 *                  UInt32                              *
 *                  bool                                *
 * @return:         none                                *
 * @type:           public                              *
 * @purpose:        Kernel-level DSP callback           *
 *******************************************************/

void xSpectrumKernel::Process(const Float32 	*inSourceP,
							  Float32		 	*inDestP,
							  UInt32 			inFramesToProcess,
							  UInt32			inNumChannels,
							  bool				&ioSilence )
{

	//This code will pass-thru the audio data.
	//This is where you want to process data to produce an effect; aka 
	//the core of Digital Signal Processing (DSP)

	//TESTING
	/*
	UInt32 nSampleFrames = inFramesToProcess;
	const Float32 *sourceP = inSourceP;
	Float32 *destP = inDestP;
	Float32 gain = GetParameter( kParam_One );
	
	while (nSampleFrames-- > 0) {
		Float32 inputSample = *sourceP;
		
		//The current (version 2) AudioUnit specification *requires* 
	    //non-interleaved format for all inputs and outputs. Therefore inNumChannels is always 1
		
		sourceP += inNumChannels;	// advance to next frame (e.g. if stereo, we're advancing 2 samples);
									// we're only processing one of an arbitrary number of interleaved channels

			// here's where you do your DSP work
                Float32 outputSample = inputSample * gain;
		
		*destP = outputSample;
		destP += inNumChannels;
	}*/
}
