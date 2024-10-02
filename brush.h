# include "NiraiLoader.h"

struct Brush {

	uint8_t color = 1;
	uint8_t size = 1;
	uint8_t type = 0;

	uint16_t colorValue = 0;

	int DebugCounter = 0;

	SDL_Rect cursor;

	///////////////////
	//// FUNCTIONS ////
	///////////////////

	// PEN FUNCTION

	void Pen(std::vector<uint8_t>& image, std::vector<uint16_t>& texture, int x, int y, int offsetX, int offsetY, int w, int h) {

		for (int i = 0; i < size; ++i) {
			for (int j = 0; j < size; ++j) {
				if (x - offsetX + i < w && x - offsetX + i >= 0 && y - offsetY + j < h && y - offsetY + i >= 0 && x - offsetX + ((y + j - offsetY) * w) + i < image.size()) {
					image[x - offsetX + ((y + j - offsetY) * w) + i] = color;
					texture[x - offsetX + ((y + j - offsetY) * w) + i] = colorValue;
				}
			}
		}
	}

	void Spray(std::vector<uint8_t>& image, std::vector<uint16_t>& texture, short x, short y, short offsetX, short offsetY, short w, short h){
		for(short i = 0; i < 3 + 1.5 * size; ++i){
			short jitterX = (rand() % (size + 3)) - 2;
			short jitterY = (rand() % (size + 3)) - 2;
			if (x - offsetX + jitterX < w && x - offsetX + jitterX >= 0 && y - offsetY + jitterY < h && y - offsetY + jitterY >= 0 && x - offsetX + ((y + jitterY - offsetY) * w) + jitterX < image.size()) {
				image[x - offsetX + jitterX + (y - offsetY + jitterY) * w] = color;
				texture[x - offsetX + jitterX + (y - offsetY + jitterY) * w] = colorValue;
			}
		}
	}

	// LINE FUNCTION

	void DrawLine( std::vector<uint8_t>& image, std::vector<uint16_t>& texture, int imageWidth, int imageHeight, int x0, int y0, int offsetX, int offsetY, int x1, int y1) {

		// variable to store non-absolute value of ∆
		// and calculate ∆x
		int d = x1 - x0;

		// store ∆x as an absolute value
		uint16_t dx = (d < 0) ? -d : d;

		// calculate ∆y and store it as an absolute value
		d = y1 - y0;
		uint16_t dy = (d < 0) ? -d : d;

		// check if x0 is to the left of x1
		// and if it is, set the horizontal stepping to positive 1,
		// else set the horizontal stepping to negative 1
		int8_t sx = (x0 < x1) ? 1 : -1;

		// check if y0 is to the left of y1
		// and if it is, set the vertical stepping to positive 1,
		// else set the vertical stepping to negative 1
		int8_t sy = (y0 < y1) ? 1 : -1;

		// calculate initial slope error corection
		int16_t err = dx - dy;

		// variable to later store the slope error *2
		int16_t e2 = 0;

		// start of the line drawing loop
		_startLineDraw:

		// draw at the current location
		Pen(image, texture, x0, y0, offsetX, offsetY, imageWidth, imageHeight);

		// if the current location is the same as the far end of the line,
		// exit the function
		if (x0 == x1 && y0 == y1) {
			return;
		}

		// calculate the error *2
		e2 = 2 * err;

		// if the error correction *2 is greater than or equal to the inverse of ∆y
		if (e2 >= -dy) {

			// subtract ∆y from the error correction
			err -= dy;

			// and step horizontally
			x0 += sx;
		}

		// if the error correction *2 is less than or equal to ∆x
		if (e2 <= dx) {

			// add ∆x to the error correction
			err += dx;

			// and step vertically
			y0 += sy;
		}

		// jump to the start of the loop
		jmp _startLineDraw;

		return;
	}

	// BUCKET FUNCTION

	void Fill ( std::vector<uint8_t>& image,							// 1-dimensional vector containing the image data
			   int x,													// x coordinate of clicked area
			   int y,													// y coordinate of clicked area
			   int offsetX,												// offset of x coordinate to accommodate for scroll
			   int offsetY,												// offset of y coordinate to accommodate for scroll
			   int w,													// width of image in pixels
			   int h,													// height of image in pixels
			   uint8_t oldColor )										// color to be filled with new color
	{

		// the variable "color" is defined in the brush struct,
		// and is used by the program as the currently used color.

		// creating a variable with the size of the image,
        // so that a subroutine isn't called every time you
        // need to access that data, as doing so would slow
        // down the algorithm substantially.
		int imageSize = image.size ();

		// creating a vector of std::array which will contain
        // x and y coordinates of valid pixels.
		std::vector<std::array<int, 2> > flood;

		// reserving the size of the image in memory
		// times the size of one unit of the coordinate vector
		// so that the vector doesn't need to be moved if the
		// push_back() call requires more memory space than is
		// available.
		flood.reserve(imageSize);

		// creating a variable to contain the absolute location
		// in the image vector of the pixel to be processed.
		int loc = 0;

		// creating an array of integers to modify the x and y
		// coordinates of the pixel, which will then be used to
		// add a new seed at the modified x and y coordinates if
		// the pixel at that location is OK to be changed.
		// the pixels will be checked in the order of above,
		// beside (left), below, and beside (right) the current
		// pixel seed.
		constexpr char xmod[4] = { 0, -1, 0, 1 };
		constexpr char ymod[4] = { -1, 0, 1, 0 };

		// setting the location for the first pixel. in this case,
		// the location is the values that were passed into the
		// function by the program when the mouse was clicked.
		loc = (x - offsetX) + ((y - offsetY) * w);

		// rst is defined as "return;" (Return from SubrouTine).

		// if the pixel at the clicked location is the same color
		// as the color currently used by the brush, return from
		// the subroutine.
		if (image[loc] == color) rst

		// otherwise, if the pixel is valid, set its value to that
		// of the new color.
		else if (loc >= 0 && loc < imageSize) image[loc] = color;

		// create an array in which to store the coordinates, and
		// store the coordinates of the newly validated pixel.
		// This is done after the value of the pixel is changed,
		// because otherwise, the algorithm would validate the
		// same pixels too many times and it would take far
		// longer than necessary.
		std::array<int, 2> fl = { x - offsetX, y - offsetY };

		// now push the coordinate array into the flood vector.
		flood.push_back(fl);

		// while the iterator is less than the size of the
		// flood vector, or in other words, while there are still
		// available pixels to be processed, loop.
		for (int i = 0; i < flood.size(); ++i) {

			// in another loop, cycle through the x and y modifier
			// arrays, and check the pixels at the modified
			// coordinates.
			for (int j = 0; j < 4; ++j) {

				// this line sets the absolute location to the
				// current modified coordinates, the pixel at
				// which, if valid, will be changed to the
				// new color and then seeded.
				loc = flood[i][0] + xmod[j] + (flood[i][1] + ymod[j]) * w;

				// the if statement checks if the location is
				// greater than zero, and less than the size of
				// the image; if the color of the pixel at that
				// location is the color to be filled in, and if
				// the modified coordinate doesn't exceed the
				// edges of the image.
				if (loc >= 0 && loc < imageSize && image[loc] == oldColor && flood[i][0] + xmod[j] >= 0 && flood[i][0] + xmod[j] < w) {

					// change the newly validated pixel to the
					// new color.
					image[loc] = color;


					// set the coordinate array to the coordinates
					// of the validated and changed pixel,
					fl[0] = flood[i][0] + xmod[j];
					fl[1] = flood[i][1] + ymod[j];

					// and push the value of the coordinate array
					// into the flood vector.
					flood.push_back(fl);
				}
			}
		}

		// after the loops are finished (there are no more
		// seeded pixels to check), return from the subroutine.
		rst
	}

};
