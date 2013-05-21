#include <AudioUnit/AudioUnit.r>

#include "xSpectrumVersion.h"

// Note that resource IDs must be spaced 2 apart for the 'STR ' name and description
#define kAudioUnitResID_xSpectrum				1000



#define RES_ID			kAudioUnitResID_xSpectrum
#define COMP_TYPE		kAudioUnitType_Effect
#define COMP_SUBTYPE	xSpectrum_COMP_SUBTYPE
#define COMP_MANUF		xSpectrum_COMP_MANF	

#define VERSION			kxSpectrumVersion
#define NAME			"Apple Demo: xSpectrum"
#define DESCRIPTION		"xSpectrum AU"
#define ENTRY_POINT		"xSpectrumEntry"

// #include "AUResources.r"
//For above error: http://www.rawmaterialsoftware.com/viewtopic.php?f=8&t=6123