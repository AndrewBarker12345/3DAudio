# 3DAudio
An audio effects plugin that simulates moving surround sound audio over headphones.

To compile this code you will also need the JUCE library(www.juce.com).  I have most recently built this with JUCE 5.4.3 (and VST SDK 3.6.12) on Mac and JUCE 4.3.0 (with VST3 SDK 3.6.0) on Windows.  Once you have JUCE installed, you can use the Introjucer/Projucer to set up an audio plugin application project and copy all these files into it.  From there you will be able to configure Xcode/Visual Studio projects or Linux makefiles to compile on whatever platform you have.  With JUCE, you can compile the code into a variety of plugin formats:  Audio Unit, VST, VST3, RTAS, or AAX.  In order to use the plugin to process audio you will need to have the binary data file that contains all the spatial impulse responses.  The data file can be obtained by purchasing a copy of the software from www.freedomaudioplugins.com.
