# What is NiraiSprite?

NiraiSprite is a program to draw sprites for retro-style games.  
The specification is inspired by the SNES, with support for 15 ARGB1555 colors + 1 transparent color per palette.  
  
256 color mode to be added soon.

# Installation

## Installation on Windows

Head over to releases and download the latest NiraiSprite.zip, and extract it anywhere on your computer.  
You can run it from its folder.

## Installation on Linux

Head over to releases and download the latest NiraiSprite.AppImage.  
Make sure it's executable with
```bash
chmod +x NiraiSprite.AppImage
```
and then you can run the file.

# Building

## Prerequisites

Before you try building NiraiSprite all by yourself, make sure you have the correct dependencies installed.  
These are:  

[SDL2](https://github.com/libsdl-org/SDL/releases) version >= 2.23 NOT 3.x  
[git](https://github.com/)  
[make](https://www.gnu.org/software/make/) or [Make for Windows](https://gnuwin32.sourceforge.net/packages/make.htm) on Windows  
[tinyfiledialogs](https://github.com/native-toolkit/libtinyfiledialogs)

## Directory Preparation

This is a platform-agnostic way to prepare the NiraiSprite directory so you can build the program.  
  
Clone and cd into the directory, and initialize submodules
```bash
git clone https://github.com/Niratomica/NiraiSprite.git --recurse-submodules
cd NiraiSprite
```
Now you can proceed to building!

## Building It Yourself on Linux

Run make
```bash
make
```

If all dependencies were installed correctly, then it should work.

## Building It Yourself on Windows [WIP]

This guide uses the PowerShell command line.  
If you don't know how to do that, just download the release.

In the NiraiSprite directory, run make, passing the win32 makefile.  
If you have not added [Make For Windows](https://gnuwin32.sourceforge.net/packages/make.htm) to your path, or installed make in some other way, do so before running this command.
```bash
make -f Makefile.win32
```
Then you can run the executable.
