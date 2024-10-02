#ifndef NIRAILOADER_H
#define NIRAILOADER_H
#include<iostream>
#include<fstream>
#include<vector>
#include<array>
#include<stack>
#include<deque>
#include<map>
#include<math.h>
#include<random>
#include<ctime>
#include<string>
#include<string.h>
#include<SDL2/SDL.h>
#include<stdlib.h>

#include"tinyfiledialogs.h"

#define rst return;
#define ret return;
#define jmp goto

namespace nl{

	struct Palette {

		Palette(){}

		uint16_t size = 0;
		uint16_t transparencyColor = 0b1000001000010000;

		std::vector<uint16_t> color;

	};


	struct Image {

		uint8_t w = 0;
		uint8_t h = 0;

		uint16_t widthInPixels = 0;
		uint16_t heightInPixels = 0;
		uint32_t size = 0;

		bool compress = 0;

		std::vector <uint8_t> bytes;

		std::deque< std::vector<uint8_t> > actions;

	};

	Palette	LoadPalette(std::string filename) {

		Palette palette;

		std::ifstream file;
		file.open(filename.c_str(), std::ios::in | std::ios::binary);

		if (!file) {
			printf("error: %s: file not found.\n", filename.c_str());
			palette.size = 0;
			return palette;
		}

		file >> std::noskipws >> palette.size;

		uint8_t		byte[2];
		uint16_t	color;

		for (int i = 0; i < palette.size; ++i) {

			file >> std::noskipws >> byte[0];
			file >> std::noskipws >> byte[1];

			color = byte[0] << 8;
			color |= byte[1] & 0xff;

			palette.color.push_back(color);

		}

		for (int i = 0; i < palette.size; ++i) {
			printf("Color %d:\n", i);
			printf("	r: %d\n", (palette.color[i] & 0b0111110000000000) >> 10);
			printf("	g: %d\n", (palette.color[i] & 0b0000001111100000) >> 5);
			printf("	b: %d\n", palette.color[i] & 0b0000000000011111);
		}

		file.close();

		return palette;
	}

	Palette LoadHexPalette(std::string filename){

		printf("opening file: %s\n", filename.c_str());
		std::ifstream file;
		file.open(filename.c_str(), std::ios::in);
		if(!file)
			printf("Could not open file: %s\n", filename.c_str());

		std::string hex;
		uint32_t hexcode;
		uint8_t rgb[3];
		uint16_t hex16;

		Palette palette;
		palette.color.reserve(16);

		for(short i = 0; i < 16; ++i)
			palette.color.push_back(0);

		palette.size = 1;

		bool inc = 0;

		for(short i = 1; i < 16; ++i){
			hex16 = 0;

			if(!std::getline(file,hex))
				break;

			hexcode = std::stoi(hex, 0, 16);

			rgb[0] = (hexcode & 0xFF0000) >> 16;
			rgb[1] = (hexcode & 0xFF00) >> 8;
			rgb[2] = (hexcode & 0xFF);

			for(short i = 0; i < 3; ++i){
				inc = (rgb[i] % 8 >= 4);
				rgb[i] /= 8;
				rgb[i] += inc;
			}

			hex16 = rgb[0] << 10;
			hex16 |= rgb[1] << 5;
			hex16 |= rgb[2];

			hex16 ^= 0b1000000000000000;

			palette.color[i] = hex16;

			if(hex16 == 0)
				i = 16;
			else ++palette.size;

		}

		file.close();

		return palette;
	}

	std::vector<uint8_t> DecompressRLE(std::vector<uint8_t> bytes) {

		printf("Compressed size is %zu\n", bytes.size());

		std::vector<uint8_t> newBytes;
		printf("Initialized new byte vector\n");

		int consecutiveBytes = 0;
		uint8_t currentByte = 0;

		for (int i = 0; i < bytes.size(); ++i) {

			consecutiveBytes = (bytes[i] >> 4) + 1;
			currentByte = bytes[i] & 0b00001111;

			for (int j = 1; j < consecutiveBytes; ++j) {
				newBytes.push_back(currentByte);
			}

		}

		printf("Returning new bytes\n");

		return newBytes;
	}

	Image	LoadImage(std::string filename) {

		printf("Loading image\n");

		Image image;

		printf("initialized image struct\n");

		std::ifstream file;
		file.open(filename.c_str(), std::ios::in | std::ios::binary);

		if (!file) {
			printf("error: %s: file not found.\n", filename.c_str());
			image.w = 0;
			return image;
		}

		printf("Opened file\n");

		file >> std::noskipws >> image.w;
		file >> std::noskipws >> image.h;

		if (image.h & 0b10000000) {
			image.h ^= 0b10000000;
			image.compress = 1;
		}

		++image.w;
		++image.h;
		image.widthInPixels = image.w * 8;

		printf("Read width: %d pixels.\n", image.widthInPixels);

		image.heightInPixels = image.h * 8;

		printf("Read height: %d pixels.\n", image.heightInPixels);

		uint8_t byte;
		uint8_t parsedByte = 0;

		while (file >> std::noskipws >> byte) {
			if(!image.compress){
				parsedByte = byte >> 4;
				image.bytes.push_back(parsedByte);
				parsedByte = byte & 0x0f;
				image.bytes.push_back(parsedByte);
			}else image.bytes.push_back(byte);
		}
		if(image.compress){
			printf("Decompressing image\n");
			image.bytes = DecompressRLE(image.bytes);
			printf("Image decompressed\n");
		}

		printf("Width is %d\n", image.w);
		printf("Height is %d\n", image.h);

		printf("Total size: %zu\n", image.bytes.size());

		file.close();
		return image;
	}

	std::vector<uint16_t> CreatePixelBuffer(Image image, Palette palette) {

		//printf("init vector\n");
		std::vector<uint16_t> data;

		uint32_t size = image.widthInPixels * image.heightInPixels;
		//printf("total size is %d bytes.\n", size * 2);

		//printf("creating texturable vector\n");
		for (uint32_t i = 0; i < size; ++i) {
			if (image.bytes[i] < palette.size)
				data.push_back(palette.color[image.bytes[i]]);
			else {
				//printf("\nERROR:	One (1) or more pixels in this image request a color\n	index greater than the size of the provided palette. Aborting.\n\n");
				while (data.size() != 0) {
					data.pop_back();
				}
				//printf("Data size reduced to %zd\n", data.size());
				return data;
			}

		}
		//printf("vector created\n");

		return data;
	}

	SDL_Texture* CreateSDLTextureFromFiles(SDL_Renderer* renderer, std::string image_filename, std::string palette_filename){

		Image image = LoadImage(image_filename);
		Palette palette = LoadPalette(palette_filename);
		SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STATIC, image.widthInPixels, image.heightInPixels);

		std::vector data = CreatePixelBuffer(image, palette);

		SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);

	return texture;
	}

	SDL_Surface* CreateSDLSurfaceFromFiles(SDL_Renderer* renderer, std::string image_filename, std::string palette_filename){

		Image image = LoadImage(image_filename);
		Palette palette = LoadPalette(palette_filename);
		std::vector data = CreatePixelBuffer(image, palette);

		SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(&data[0], image.widthInPixels, image.heightInPixels, 16, 2*image.widthInPixels, SDL_PIXELFORMAT_ARGB1555);


	return surface;
	}
}

#endif // NIRAILOADER_H
