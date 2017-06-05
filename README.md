&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<img src="http://i.imgur.com/7IPC2Q3.png" width="250" height="200"> <img src="http://i.imgur.com/UL4eeqe.png" width="250" height="200"> <img src="http://i.imgur.com/rWNlyUd.png" width="250" height="200">

A music visualizer written in C++. It uses OpenGL for graphics, libsndfile and portaudio for audio I/O and FFTW for the fourier transform of the audio samples. The application takes in the path to a .wav audio file, plays it and generates animated imagery in real time.<br><br>
# Getting started
Clone this repository using: ```git clone https://github.com/kirankumari04/Music-Visualizer.git```
<br>or Download ZIP and extract it.
<br><br>
The visual studio solution and project files are uploaded here. But, you first would have to setup the dependencies.

Even if you don't have Visual Studio installed, you can run the **Music_Visualization.exe** executable present in the repository, you only need to get the dlls mentioned below.

## Installing dependencies
Create a folder inside the repository folder, named **libraries**.

* Download the fftw for windows from here: [FFTW Download page](http://www.fftw.org/install/windows.html)

Extract the files into a folder named FFTW32 and place it in the libraries folder of the repository directory. 
* Download libsndfile from here: [libsndfile Download page](http://www.mega-nerd.com/libsndfile/#Download)
[[Direct link](http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.28-w32-setup.exe)]

Install it. Go to the installation directory (C:\Program Files (x86)\Mega-Nerd) and copy the libsndfile folder to the libraries folder of the repository folder. (You can uninstall libsndfile now.)
* download portaudio from here: [Portaudio Download page](http://www.portaudio.com/download.html)
[[Direct link](http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz)]

Extract the portaudio folder to the libraries folder in the repository directory.
* Download the prepackaged freeglut library from: [freeglut download page](http://freeglut.sourceforge.net/index.php#download)
[[Direct link](http://files.transmissionzero.co.uk/software/development/GLUT/freeglut-MinGW.zip)]

Extract the freeglut folder into the library folder in the repository directory.
<br><br>
Now, from the libraries folder, copy-paste these dlls to the repository directory (Or, you can just add their paths to the PATH environment variable.):
1. **libfftw3-3.dll** from *FFTW32* folder
2. **freeglut.dll from** *freeglut/bin* folder
3. **libsndfile-1.dll** from *libsndfile/bin* folder
4. **portaudio_x86.dll** from *portaudio/build/msvc/Win32/Debug* folder

Open the solution file (Music_Visualizer.sln) from the repository folder. Done!

## Key controls

| Theme                |Key|
| ---------------------|:--|
| Time domain waveform |<kbd>t</kbd>|
| Frequency bars       |<kbd>f</kbd>|
| misc                 |<kbd>m</kbd>|

I've included a .wav file as an audio sample, you can add more .wav files here and input the relative path as: *music_files/filename.wav*
or you can give a path to any .wav file on your machine.
