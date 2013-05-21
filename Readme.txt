This project was finalized for CS467 Social Visualization for the Spring 2013 Semester
Group: Rohan Jyoti (jyoti1), Siraj Nyakhar (nyakhar1), Josh Yomtoob (yomtoob1)


NOTE: Many parts of this project have been purposely left out. You will not be able to compile this project. In fact deliberate mistakes in the project settings have been made. Please do not attempt to reverse engineer the project. Some sample code files have been provided, however, to get an overall feel of the project.

This project follows the MVC protocol
--> Model: xSpectrum.cpp
--> View: xSpectrumView + xSpectrumCocaView
--> Controller: xSpectrumCocoaViewFactory


Overall Process is as follows:
1. Extract floating point values
2. Custom designed Fourrier Transformation
3. Convolution
4. Push results to View


Apple's CoreAudio and AudioUnit frameworks are used.


The purpose of AudioUnits is to make this process realtime at the kernel-level. An AudioUnit Hosting Platform will be responsible for feeding the data into the plugin which will encapsulate user-written code. The project files and overall purpose are as follows:

xSpectrum.cpp/h --> Actual AudioUnit Back-end code (consists of majority of new AudioUnit related code in addition to previous Final1 xProcessor code)

xSpectrumSharedData --> struct containing data used by all the model, view, and controller

xSpectrum.ext --> exports file necessary for audioUnit linkage into an AudioUnit Hosting service. Its how the AUHS knows where to enter your code (since there is no concept of a main function).

xSpectrum.r --> Rez file required for component building

xSpectrumVersion.h --> Info file for AudioUnit Hosting Service Integration

xSpectrumView --> GUI custom view code defining view elements

xSpectrumCocoaViewFactory --> Controller that links the User Interface to the AudioUnit Window provided by the AUHS

xSpectrumCocoaView --> GUI view code responsible for actual display of spectral processing from xSpectrum

xSpectrumCocaView.xib --> GUI design file

xSpectrum-Info.plist, InfoPlist.strings, xSpectrumCocoaFactory-Info.plist --> necessary info files for any Mac registered software.

xSpectrum-Prefix.pch --> Necessary for Kernel Extension building (AudioUnits are very low level and require kernel extensions to be written (in xSpectrum.cpp) )


=======================================================================================================================
Relation to Social Visualization

In our group's opinion, this project's overall scope relates to the social perspective by providing a visualization technique that invokes multiple sense during a music concert. This project is meant to serve as a simple prototype -- a proof of concept -- for something on a much larger scale. As you can see from the demo video, a user can "see" the DNA of music in realtime. Now imagine this on a much larger scale, with blatant exaggerations around the bassline, drums and snares synced with live music.

