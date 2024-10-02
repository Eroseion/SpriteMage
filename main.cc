# include "brush.h"
#include<cstring>

#ifdef __APPLE__
#define LMOD SDLK_LGUI
#define RMOD SDLK_RGUI
#else
#define LMOD SDLK_LCTRL
#define RMOD SDLK_RCTRL
#endif

nl::Palette	EditPalette(nl::Palette palette = nl::Palette());
nl::Palette RandomPalette(unsigned short size);
void	SavePalette(nl::Palette palette, const char* filename);

nl::Image	NewImage();
void	SaveImage(nl::Image image, char* filename);
void	SaveImageToBMP(std::vector<uint16_t> data, uint16_t width, uint16_t height);

std::vector<uint8_t> CompressRLE(std::vector<uint8_t> bytes);

struct Button : SDL_Rect{

	~Button(){
		SDL_DestroyTexture(icon);
	}

	void SetIcon(SDL_Texture image);
	SDL_Texture* GetIcon(){
		return icon;
	}

private:
	SDL_Texture* icon;
};

Button* PointIsOnThisButton(int x, int y, Button* button){
	if(x >= button->x && x <= button->x + button->w && y >= button->y && y <= button->y + button->h)
		return button;
	else return nullptr;
}

int main(int argc, char** argv) {

printf("Entered Main Method\n");

#ifdef _WIN32

std::string cursor_plt = "cursor.plt";
std::string default_plt = "default.plt";
std::string erase_nir = "erase.nir";
std::string fill_nir = "fill.nir";
std::string pen_nir = "pen.nir";
std::string sidebar_nir = "sidebar.nir";
std::string spray_nir = "spray.nir";
/*
#elif __linux__

std::string cursor_plt = getenv("APPDIR"); cursor_plt += "/cursor.plt";
std::string default_plt = getenv("APPDIR"); default_plt += "/default.plt";
std::string erase_nir = getenv("APPDIR"); erase_nir += "/erase.nir";
std::string fill_nir = getenv("APPDIR"); fill_nir += "/fill.nir";
std::string pen_nir = getenv("APPDIR"); pen_nir += "/pen.nir";
std::string sidebar_nir = getenv("APPDIR"); sidebar_nir += "/sidebar.nir";
std::string spray_nir = getenv("APPDIR"); spray_nir += "/spray.nir";
*/
#endif

_start:

srand(time(0));

nl::Palette palette;
nl::Image image;
std::vector<uint16_t> data;

	if (argv[1] == NULL) {
		printf("No image specified. Using blank 64x64 pixel image.\n\n");
		image.w = 8;
		image.h = 8;
		image.widthInPixels = 64;
		image.heightInPixels = 64;
		for(short i = 0; i < 4096; ++i){
			image.bytes.push_back(0);
		}
		printf("\nNo palette specified. Using default palette.\n\n");
		palette = nl::LoadPalette("default.plt");
		if (palette.size <= 1) { printf("Exiting.\n"); return 0; }
	}
	else if (argv[1] != NULL && argv[2] == NULL) {
		if ((image = nl::LoadImage(argv[1])).w == 0) {
			printf("Using blank 64x64 pixel image.\n");
			image.w = 8;
			image.h = 8;
			image.widthInPixels = 64;
			image.heightInPixels = 64;
			for(short i = 0; i < 4096; ++i){
				image.bytes.push_back(0);
			}
		}
		printf("\nNo palette specified. Using default palette.\n\n");
		palette = nl::LoadPalette("default.plt");
		if (palette.size <= 1) { printf("Exiting.\n"); return 0; }
	}
	else {
		if (!strcmp(argv[2], "--random")) {
			if(argv[3])
				palette = RandomPalette(std::stoi(argv[3]));
			else{
				printf("You must specify a palette size.\n");
				return 1;
			}
		}
		else if (!strcmp(argv[2], "--hex")) {
			if(argv[3]) palette = nl::LoadHexPalette(argv[3]);
			else{
				printf("You must specify the .hex palette file.\n");
				return 1;
			}
		}
		else if ((palette = nl::LoadPalette(argv[2])).size <= 1) {
			printf("Creating new palette.\n");
			palette = EditPalette();
			if (palette.size <= 1) { printf("Exiting.\n"); return 0; }

		}
		if ((image = nl::LoadImage(argv[1])).w == 0) {
			printf("Using blank 64x64 pixel image.\n");
			image.w = 8;
			image.h = 8;
			image.widthInPixels = 64;
			image.heightInPixels = 64;
			for(short i = 0; i < 4096; ++i){
				image.bytes.push_back(0);
			}
		}

	}

	printf("Creating texture vector.\n");
	data = nl::CreatePixelBuffer(image, palette);

	if (data.size() == 0) {
		printf("Texture size returned null. Exit.\n");
		return 1;
	}

	if (image.compress) {
		printf("Image gets compressed.\n");
	}
	else printf("Image does not get compressed.\n");

	image.size = image.bytes.size();

	Brush brush;
	brush.colorValue = palette.color[brush.color];
	brush.cursor.w = 1;
	brush.cursor.h = 1;
	brush.cursor.x = 0;
	brush.cursor.y = 0;

	int lastX = 0, lastY = 0;
	SDL_Point lastClick;
	std::deque< SDL_Point > lastClicks;
	lastClick.x = 160;
	lastClick.y = 0;
	lastClicks.push_back(lastClick);

	SDL_Init(SDL_INIT_EVERYTHING);

	uint32_t renderLogicalWidth = image.widthInPixels + image.widthInPixels / 3;
	uint32_t renderLogicalHeight = image.heightInPixels;

	uint16_t winW = image.widthInPixels>480?image.widthInPixels+image.widthInPixels/3:640;
	uint16_t winH = image.heightInPixels>320?image.heightInPixels+image.heightInPixels:480;
	int winX, winY;

	SDL_Window* window = SDL_CreateWindow("NiraiSprite", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winW, winH, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	SDL_RenderSetLogicalSize(renderer, renderLogicalWidth, renderLogicalHeight);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);

	SDL_GetWindowPosition(window, &winX, &winY);

	SDL_Window* paletteWindow = SDL_CreateWindow("NiraiSprite palette", winX + 640, winY, 256, 256, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_SKIP_TASKBAR | SDL_WINDOW_ALWAYS_ON_TOP);
	SDL_Renderer* paletteRenderer = SDL_CreateRenderer(paletteWindow, -1, 0);
	SDL_RenderSetLogicalSize(paletteRenderer, 16, 16);
	SDL_SetRenderDrawColor(paletteRenderer, 0, 0, 0, 0xFF);

	bool focus = 1;

	SDL_Texture* paletteTex = SDL_CreateTexture(paletteRenderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STATIC, palette.size > 16 ? 16 : palette.size - 1, palette.size > 16 ? 16 : 1);
	SDL_UpdateTexture(paletteTex, NULL, &palette.color[1], (palette.size - 1) * 2);

	SDL_Texture* pEditTex = nl::CreateSDLTextureFromFiles(paletteRenderer, "pen.nir", "cursor.plt");

	SDL_Rect pEditRect;
	pEditRect.x = 240;
	pEditRect.y = 240;
	pEditRect.w = 16;
	pEditRect.h = 16;

	SDL_Rect paletteRect;
	paletteRect.x = 0;
	paletteRect.y = 0;
	paletteRect.w = palette.size > 16 ? 16 : palette.size - 1;
	paletteRect.h = palette.size > 16 ? 16 : 1;


	short clickedColor = 1;

	SDL_Surface* eraseSurf = nl::CreateSDLSurfaceFromFiles(renderer, "erase.nir", "cursor.plt");
	SDL_Cursor* eraseCur = SDL_CreateColorCursor(eraseSurf, 6, 13);
	SDL_Surface* penSurf = nl::CreateSDLSurfaceFromFiles(renderer, "pen.nir", "cursor.plt");
	SDL_Cursor* penCur = SDL_CreateColorCursor(penSurf, 0, 15);
	SDL_Surface* fillSurf = nl::CreateSDLSurfaceFromFiles(renderer, "fill.nir", "cursor.plt");
	SDL_Cursor* fillCur = SDL_CreateColorCursor(fillSurf, 2, 14);
	SDL_Surface* spraySurf = nl::CreateSDLSurfaceFromFiles(renderer, "spray.nir", "cursor.plt");
	SDL_Cursor* sprayCur = SDL_CreateColorCursor(spraySurf, 0, 3);

	SDL_Cursor* curCur = penCur;

	bool curIsDefault = 0;

	SDL_Texture* sidebarTex = nl::CreateSDLTextureFromFiles(renderer, "sidebar.nir", "default.plt");

	uint8_t optionSize = 80;

	Button penButton, fillButton, sprayButton, eraseButton, weightButton, paletteToggleButton, undoButton, newButton, saveButton, openButton;
	Button* optionButtons[] = {&penButton, &fillButton, &sprayButton, &eraseButton, &weightButton, &paletteToggleButton, &undoButton, &newButton, &saveButton, &openButton};

	penButton.w = optionSize;
	penButton.h = optionSize;
	fillButton.w = optionSize;
	fillButton.h = optionSize;
	sprayButton.w = optionSize;
	sprayButton.h = optionSize;
	eraseButton.w = optionSize;
	eraseButton.h = optionSize;
	weightButton.w = optionSize;
	weightButton.h = optionSize;
	paletteToggleButton.w = optionSize;
	paletteToggleButton.h = optionSize;
	undoButton.w = optionSize;
	undoButton.h = optionSize;
	newButton.w = optionSize;
	newButton.h = optionSize;
	saveButton.w = optionSize;
	saveButton.h = optionSize;
	openButton.w = optionSize;
	openButton.h = optionSize;

	penButton.x = 0;
	penButton.y = 0;
	fillButton.x = 80;
	fillButton.y = 0;
	sprayButton.x = 0;
	sprayButton.y = 80;
	eraseButton.x = 80;
	eraseButton.y = 80;
	weightButton.x = 0;
	weightButton.y = 160;
	paletteToggleButton.x = 80;
	paletteToggleButton.y = 160;
	undoButton.x = 0;
	undoButton.y = 240;
	newButton.x = 80;
	newButton.y = 320;
	saveButton.x = 0;
	saveButton.y = 400;
	openButton.x = 80;
	openButton.y = 400;

	SDL_Rect sidebarRect;
	sidebarRect.x = 0;
	sidebarRect.y = 0;
	sidebarRect.w = 160;
	sidebarRect.h = 480;

	SDL_SetCursor(curCur);
	brush.cursor.x = renderLogicalWidth / 4;

//	SDL_ShowCursor(SDL_DISABLE);

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STATIC, image.widthInPixels, image.heightInPixels);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);

	SDL_Texture* cursorTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STATIC, 1, 1);
	SDL_SetTextureBlendMode(cursorTex, SDL_BLENDMODE_NONE);
	SDL_UpdateTexture(cursorTex, NULL, &palette.color[brush.color], 1);

	SDL_Rect rect;

	rect.w = image.widthInPixels;
	rect.h = image.heightInPixels;

	SDL_Texture* alphaTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1, 1);
	SDL_SetRenderDrawColor(renderer, 0xee, 0xee, 0xee, 0xff);
	SDL_SetRenderTarget(renderer, alphaTex);
	SDL_RenderClear(renderer);
	SDL_SetRenderTarget(renderer,NULL);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);

	image.actions.push_back(image.bytes);

	bool running = 1;
	SDL_Event event;

	bool leftClick = 0;
	int mouseX = 0;
	int mouseY = 0;


	bool ctrl = 0;
	bool shift = 0;

	int quit = 0;
	const char* saveFilterPatterns[1] = { "*.nir" };
	const char* paletteFilterPatterns[1] = { "*.plt" };

	uint16_t currentWidth = renderLogicalWidth;
	uint16_t currentHeight = renderLogicalHeight;

	SDL_Point lastDraw, lastDrawingPos;
	lastDrawingPos.x = 160;
	lastDrawingPos.y = 0;
	bool weight = 0;

	bool drewYet = 0;
	bool shouldRender = 1;

	char * filename;

	const char* helpMessage = "Ctrl + Right = next color\n\nCtrl + Left = previous color\n\nLeft click = select color at cursor\n\nP = pen tool\n\nE = eraser\n\nF = fill tool\n\nW = weight/pixel perfect mode\n\nCtrl + P = edit palette\n\nCtrl + Z = undo\n\n Ctrl + S = save\n\nCtrl + B save as Windows .bmp bitmap\n\n Ctrl + N = new image (will erase unsaved progress)\n\nCtrl + O = open NiraiSprite image file (will erase unsaved progress)\n\nCtrl + Up = increase pen size\n\n Ctrl + Down = decrease pen size";

	if(rect.w > rect.h){
		renderLogicalHeight = (renderLogicalWidth / 4) * 3;
		rect.y = (renderLogicalHeight / 2) - (rect.h / 2);
	}
	else if(rect.h > rect.w){
		renderLogicalWidth = (renderLogicalHeight / 3) * 4;
		rect.x = (renderLogicalWidth / 4) + (renderLogicalHeight / 2) - (rect.w / 2);
	}
	else if(rect.h == rect.w){
		renderLogicalWidth = (renderLogicalHeight / 3) * 4;
		rect.x = image.widthInPixels / 3 - 1;
		rect.y = 0;
	}
	currentWidth = renderLogicalWidth;
	currentHeight = renderLogicalHeight;

	printf("Entering Program Loop\n");

	while (running) {

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_WINDOWEVENT:
				if(event.window.event == SDL_WINDOWEVENT_CLOSE){
					if(event.window.windowID == SDL_GetWindowID(window)){
						quit = tinyfd_messageBox("Save image?", "Would you like to save the image before quitting?", "yesnocancel", "question", 0);
						if(quit == 2)
							running = 0;
						else if (quit == 1) {
							SaveImage(image, tinyfd_saveFileDialog("Save Image", NULL, 1, saveFilterPatterns, "NiraiSprite image files"));
							running = 0;
						}
					}else if(event.window.windowID == SDL_GetWindowID(paletteWindow)){
						SDL_HideWindow(paletteWindow);
					}
				}
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_w:
					weight = weight == 0 ? 1 : 0;
					break;
				case SDLK_e:
					brush.color = 0;
					brush.type = 0;
					brush.colorValue = palette.color[brush.color];
					SDL_UpdateTexture(cursorTex, NULL, &palette.color[brush.color], 1);
					SDL_SetCursor(eraseCur);
					curCur = eraseCur;
				break;
				case SDLK_f:
					brush.type = 1;
					brush.cursor.w = 1;
					brush.cursor.h = 1;
					SDL_SetCursor(fillCur);
					curCur = fillCur;
					if(!brush.color){
						++brush.color;
						brush.colorValue = palette.color[brush.color];
						SDL_UpdateTexture(cursorTex, NULL, &palette.color[brush.color], 1);
					}
					break;
				case SDLK_h:
					SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Keybinds", helpMessage, window);
				break;
				case SDLK_n:
					if(ctrl){
						nl::Image newImg = NewImage();
						if(newImg.h){
							image = newImg;
							rect.x = image.widthInPixels/3;
							rect.y = 0;
							rect.w = image.widthInPixels;
							rect.h = image.heightInPixels;
							renderLogicalWidth = rect.w + rect.w / 3;
							renderLogicalHeight = rect.h;
							currentWidth = renderLogicalWidth;
							currentHeight = renderLogicalHeight;
							SDL_RenderSetLogicalSize(renderer, renderLogicalWidth, renderLogicalHeight);
							data = nl::CreatePixelBuffer(image, palette);
							SDL_DestroyTexture(texture);
							texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STATIC, image.widthInPixels, image.heightInPixels);
							SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
							SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
							while(image.actions.size())
								image.actions.pop_back();
							image.actions.push_back(image.bytes);
							drewYet = 0;
							while(lastClicks.size()){
								lastClicks.pop_back();
							}
							lastClick.x = 160;
							lastClick.y = 0;
							lastClicks.push_back(lastClick);
							if(rect.w > rect.h){
								renderLogicalHeight = (renderLogicalWidth / 4) * 3;
								rect.y = (renderLogicalHeight / 2) - (rect.h / 2);
							}
							else if(rect.h > rect.w){
								renderLogicalWidth = (renderLogicalHeight / 3) * 4;
								rect.x = (renderLogicalWidth / 4) + (renderLogicalHeight / 2) - (rect.w / 2);
							}
							else if(rect.h == rect.w){
								renderLogicalWidth = (renderLogicalHeight / 3) * 4;
								rect.x = image.widthInPixels / 3 - 1;
								rect.y = 0;
							}
							currentWidth = renderLogicalWidth;
							currentHeight = renderLogicalHeight;
						}
					}
				break;
				case SDLK_o:
					if(ctrl){
						filename = tinyfd_openFileDialog("Open Image File", "", 1, saveFilterPatterns, "NiraiSprite image files", 0);
						if(filename) image = nl::LoadImage(filename);
						rect.x = image.widthInPixels/3;
						rect.y = 0;
						rect.w = image.widthInPixels;
						rect.h = image.heightInPixels;
						renderLogicalWidth = rect.w + rect.w / 3;
						renderLogicalHeight = rect.h;
						currentWidth = renderLogicalWidth;
						currentHeight = renderLogicalHeight;
						SDL_RenderSetLogicalSize(renderer, renderLogicalWidth, renderLogicalHeight);
						palette = EditPalette(palette);
						SDL_DestroyTexture(paletteTex);
						paletteTex = SDL_CreateTexture(paletteRenderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STATIC, palette.size > 16 ? 16 : palette.size - 1, palette.size > 16 ? 16 : 1);
						SDL_UpdateTexture(paletteTex, NULL, &palette.color[1], (palette.size - 1) * 2);
						paletteRect.w = palette.size > 16 ? 16 : palette.size - 1;
						paletteRect.h = palette.size > 16 ? 16 : 1;
						data = nl::CreatePixelBuffer(image, palette);
						SDL_DestroyTexture(texture);
						texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STATIC, image.widthInPixels, image.heightInPixels);
						SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
						SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
						while(image.actions.size())
							image.actions.pop_back();
						image.actions.push_back(image.bytes);
						drewYet = 0;
						while(lastClicks.size()){
							lastClicks.pop_back();
						}
						lastClick.x = 160;
						lastClick.y = 0;
						lastClicks.push_back(lastClick);
						if(rect.w > rect.h){
							renderLogicalHeight = (renderLogicalWidth / 4) * 3;
							rect.y = (renderLogicalHeight / 2) - (rect.h / 2);
						}
						else if(rect.h > rect.w){
							renderLogicalWidth = (renderLogicalHeight / 3) * 4;
							rect.x = (renderLogicalWidth / 4) + (renderLogicalHeight / 2) - (rect.w / 2);
						}
						else if(rect.h == rect.w){
							renderLogicalWidth = (renderLogicalHeight / 3) * 4;
							rect.x = image.widthInPixels / 3 - 1;
							rect.y = 0;
						}
						currentWidth = renderLogicalWidth;
						currentHeight = renderLogicalHeight;
					}
				break;
				case SDLK_p:
					if (!ctrl && !shift) {
						brush.type = 0;
						brush.cursor.w = brush.size;
						brush.cursor.h = brush.size;
						if(brush.color == 0){
							brush.color = 1;
							brush.colorValue = palette.color[1];
							SDL_UpdateTexture(cursorTex, NULL, &brush.colorValue, 1);
						}
						SDL_SetCursor(penCur);
						curCur = penCur;
					}
					else if(!shift){
						palette = EditPalette(palette);
						data = nl::CreatePixelBuffer(image, palette);
						SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
						brush.colorValue = palette.color[brush.color];
						SDL_UpdateTexture(cursorTex, NULL, &brush.colorValue, 1);
						ctrl = 0;
						SDL_DestroyTexture(paletteTex);
						paletteTex = SDL_CreateTexture(paletteRenderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STATIC, palette.size > 16 ? 16 : palette.size - 1, palette.size > 16 ? 16 : 1);
						SDL_UpdateTexture(paletteTex, NULL, &palette.color[1], (palette.size - 1) * 2);
						paletteRect.w = palette.size > 16 ? 16 : palette.size - 1;
						paletteRect.h = palette.size > 16 ? 16 : 1;
					}
					else if(shift){
						SDL_ShowWindow(paletteWindow);
					}
					break;
				case SDLK_s:
					if(ctrl) SaveImage(image, tinyfd_saveFileDialog("Save Image", NULL, 1, saveFilterPatterns, "NiraiSprite image files"));
					else{
						brush.type = 2;
						brush.cursor.w = brush.size;
						brush.cursor.h = brush.size;
						SDL_SetCursor(sprayCur);
						curCur = sprayCur;
						if(!brush.color){
							++brush.color;
							brush.colorValue = palette.color[brush.color];
						}
					}
					break;
				case SDLK_b:
					if(ctrl) SaveImageToBMP(data,image.widthInPixels,image.heightInPixels);
					break;
				case SDLK_z:
					if (ctrl && image.actions.size() > 1) {
						image.actions.pop_back();
						image.bytes = image.actions.back();
						data = nl::CreatePixelBuffer(image, palette);
						SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
						if(lastClicks.size() > 1){
							lastClicks.pop_back();
							lastClick = lastClicks.back();
						}
						renderLogicalWidth = currentWidth;
						renderLogicalHeight = currentHeight;
						SDL_RenderClear(renderer);
						SDL_RenderSetLogicalSize(renderer, renderLogicalWidth, renderLogicalHeight);
						SDL_RenderCopy(renderer, alphaTex, NULL, &rect);
						SDL_RenderCopy(renderer, texture, NULL, &rect);
						renderLogicalWidth = 640;
						renderLogicalHeight = 480;
						SDL_RenderSetLogicalSize(renderer, 640, 480);
						SDL_RenderCopy(renderer, sidebarTex, NULL, &sidebarRect);
						SDL_RenderPresent(renderer);
						if(shouldRender){
							renderLogicalWidth = currentWidth;
							renderLogicalHeight = currentHeight;
							SDL_RenderSetLogicalSize(renderer, renderLogicalWidth, renderLogicalHeight);
						}
					}
					break;
					case SDLK_SLASH:
					SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Keybinds", helpMessage, window);
					break;
					case SDLK_RIGHT:
					if (brush.color < palette.size - 1 && ctrl) {
						if(!brush.type){
							SDL_SetCursor(penCur);
							curCur = penCur;
						}
						++brush.color;
						brush.colorValue = palette.color[brush.color];
						SDL_UpdateTexture(cursorTex, NULL, &palette.color[brush.color], 1);
					}
					else if ( rect.x + rect.w - 5 > renderLogicalWidth / 4) rect.x -= 4;
					break;
					case SDLK_PERIOD:
						if (brush.color < palette.size - 1 && ctrl) {
							if(!brush.type){
								SDL_SetCursor(penCur);
								curCur = penCur;
							}
							++brush.color;
							brush.colorValue = palette.color[brush.color];
							SDL_UpdateTexture(cursorTex, NULL, &palette.color[brush.color], 1);
						}
						else if ( 0 - rect.x + renderLogicalWidth < image.widthInPixels) rect.x -= 4;
						break;
						case SDLK_LEFT:
						if (brush.color > 0 && ctrl) {
							if(!brush.type || brush.color > 1)
								--brush.color;
							brush.colorValue = palette.color[brush.color];

							if(brush.color == 0){
								if (! (data[brush.color] & 0b0000000000000000)) {
										SDL_UpdateTexture(cursorTex, NULL, &brush.colorValue, 1);
								}
								SDL_SetCursor(eraseCur);
								curCur = eraseCur;
							}
							else SDL_UpdateTexture(cursorTex, NULL, &brush.colorValue, 1);
						}
						else rect.x += 4;
						if(rect.x + 5 > renderLogicalWidth && rect.x + 5 > 0){
							rect.x = renderLogicalWidth - 4;
						}
						break;
						case SDLK_COMMA:
						if (brush.color > 0 && ctrl) {
							if(!brush.type || brush.color > 1)
								--brush.color;
							brush.colorValue = palette.color[brush.color];

							if(brush.color == 0){
								if (! (data[brush.color] & 0b0000000000000000)) {
										SDL_UpdateTexture(cursorTex, NULL, &brush.colorValue, 1);
								}
								SDL_SetCursor(eraseCur);
								curCur = eraseCur;
							}
							else SDL_UpdateTexture(cursorTex, NULL, &brush.colorValue, 1);

						}
						else if (rect.x < 0) rect.x += 4;
						break;
						case SDLK_UP:
							if (ctrl && brush.size < 0xff && (!brush.type || brush.type == 2)) {
							++brush.size;
							++brush.cursor.w;
							++brush.cursor.h;
						}else
						if( rect.y + 5 < renderLogicalHeight || rect.y < 0 ) rect.y += 4;
						break;
						case SDLK_EQUALS:
							if (ctrl && brush.size < 0xff && (!brush.type || brush.type == 2)) {
							++brush.size;
							++brush.cursor.w;
							++brush.cursor.h;
						}else
						if( rect.y < 0 ) rect.y += 4;
						break;
						case SDLK_DOWN:
							if (ctrl && brush.size > 1 && (!brush.type || brush.type == 2)) {
								--brush.size;
							--brush.cursor.w;
							--brush.cursor.h;
						} else
						if( rect.y + rect.h - 5 > 0) rect.y -= 4;
						break;
						case SDLK_MINUS:
							if (ctrl && brush.size > 1 && (!brush.type || brush.type == 2)) {
								--brush.size;
							--brush.cursor.w;
							--brush.cursor.h;
						} else
						if( 0 - rect.y + renderLogicalHeight < image.heightInPixels ) rect.y -= 4;
						break;
				case LMOD:
					ctrl = 1;
					break;
				case RMOD:
					ctrl = 1;
					break;
				case SDLK_LSHIFT:
					shift = 1;
					break;
				case SDLK_RSHIFT:
					shift = 1;
					break;
				}
				break;

			case SDL_KEYUP:
				switch(event.key.keysym.sym){
					case LMOD:
						ctrl = 0;
						break;
					case RMOD:
						ctrl = 0;
						break;
					case SDLK_LSHIFT:
						shift = 0;
						break;
					case SDLK_RSHIFT:
						shift = 0;
						break;
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button) {
				case SDL_BUTTON_LEFT:
					if(event.window.windowID == SDL_GetWindowID(window)){
						if(brush.cursor.x >= renderLogicalWidth / 4 - 1){
							brush.cursor.x = event.button.x - brush.cursor.w / 2;
							brush.cursor.y = event.button.y - brush.cursor.h / 2;
							lastX = brush.cursor.x;
							lastY = brush.cursor.y;

							leftClick = 1;
							if(!brush.type && brush.cursor.x >= renderLogicalWidth / 4){
								if (shift && drewYet){
									brush.DrawLine(image.bytes, data, image.widthInPixels, image.heightInPixels, lastClicks.back().x, lastClicks.back().y, rect.x, rect.y, brush.cursor.x, brush.cursor.y);
								}
								else drewYet = 1;
								brush.Pen(image.bytes, data, brush.cursor.x, brush.cursor.y, rect.x, rect.y, image.widthInPixels, image.heightInPixels);
								lastDrawingPos.x = brush.cursor.x;
								lastDrawingPos.y = brush.cursor.y;

								SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);

							}
							else if(brush.type == 1){

								brush.Fill(image.bytes, brush.cursor.x, brush.cursor.y, rect.x, rect.y, image.widthInPixels, image.heightInPixels, image.bytes[event.button.x - rect.x + ((event.button.y - rect.y) * image.widthInPixels)]);
								data = nl::CreatePixelBuffer(image, palette);
								SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
							}
							else if(brush.type == 2){
								brush.Spray(image.bytes, data, brush.cursor.x, brush.cursor.y, rect.x, rect.y, image.widthInPixels, image.heightInPixels);
								lastDrawingPos.x = brush.cursor.x;
								lastDrawingPos.y = brush.cursor.y;
								SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
							}
							lastClick.x = brush.cursor.x;
							lastClick.y = brush.cursor.y;
							lastClicks.push_back(lastClick);
							if(lastClicks.size() > 11) lastClicks.pop_front();
						}
						else{
							Button* button;
							for(short i = 0; i < 10; ++i){
								button = PointIsOnThisButton(event.button.x, event.button.y, optionButtons[i]);
									if(button == &penButton){
										brush.type = 0;
										brush.cursor.w = brush.size;
										brush.cursor.h = brush.size;
										if(brush.color == 0){
											brush.color = 1;
											brush.colorValue = palette.color[1];
											SDL_UpdateTexture(cursorTex, NULL, &brush.colorValue, 1);
										}
										SDL_SetCursor(penCur);
										curCur = penCur;
									}
									else if(button == &fillButton){
										brush.type = 1;
										brush.cursor.w = 1;
										brush.cursor.h = 1;
										SDL_SetCursor(fillCur);
										curCur = fillCur;
										if(!brush.color){
											++brush.color;
											brush.colorValue = palette.color[brush.color];
											SDL_UpdateTexture(cursorTex, NULL, &palette.color[brush.color], 1);
										}
									}
									else if(button == &sprayButton){
										brush.type = 2;
										brush.cursor.w = brush.size;
										brush.cursor.h = brush.size;
										SDL_SetCursor(sprayCur);
										curCur = sprayCur;
										if(!brush.color){
											++brush.color;
											brush.colorValue = palette.color[brush.color];
										}
									}
									else if(button == &eraseButton){
										brush.color = 0;
										brush.type = 0;
										brush.colorValue = palette.color[brush.color];
										SDL_UpdateTexture(cursorTex, NULL, &palette.color[brush.color], 1);
										SDL_SetCursor(eraseCur);
										curCur = eraseCur;
									}
									else if(button == &weightButton){
										weight = weight == 0 ? 1 : 0;
									}
									else if(button == &paletteToggleButton){
										if(SDL_GetWindowFlags(paletteWindow) & SDL_WINDOW_HIDDEN){
											SDL_ShowWindow(paletteWindow);
										}
										else{
											SDL_HideWindow(paletteWindow);
										}
									}
									else if(button == &undoButton){
										image.actions.pop_back();
										image.bytes = image.actions.back();
										data = nl::CreatePixelBuffer(image, palette);
										SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
										if(lastClicks.size() > 1){
											lastClicks.pop_back();
											lastClick = lastClicks.back();
										}
										renderLogicalWidth = currentWidth;
										renderLogicalHeight = currentHeight;
										SDL_RenderClear(renderer);
										SDL_RenderSetLogicalSize(renderer, renderLogicalWidth, renderLogicalHeight);
										SDL_RenderCopy(renderer, alphaTex, NULL, &rect);
										SDL_RenderCopy(renderer, texture, NULL, &rect);
										renderLogicalWidth = 640;
										renderLogicalHeight = 480;
										SDL_RenderSetLogicalSize(renderer, 640, 480);
										SDL_RenderCopy(renderer, sidebarTex, NULL, &sidebarRect);
										SDL_RenderPresent(renderer);
									}
									else if(button == &newButton){
										nl::Image newImg = NewImage();
										if(newImg.h){
											image = newImg;
											rect.x = image.widthInPixels/3;
											rect.y = 0;
											rect.w = image.widthInPixels;
											rect.h = image.heightInPixels;
											renderLogicalWidth = rect.w + rect.w / 3;
											renderLogicalHeight = rect.h;
											currentWidth = renderLogicalWidth;
											currentHeight = renderLogicalHeight;
											SDL_RenderSetLogicalSize(renderer, renderLogicalWidth, renderLogicalHeight);
											data = nl::CreatePixelBuffer(image, palette);
											SDL_DestroyTexture(texture);
											texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STATIC, image.widthInPixels, image.heightInPixels);
											SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
											SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
											while(image.actions.size())
												image.actions.pop_back();
											image.actions.push_back(image.bytes);
											drewYet = 0;
											while(lastClicks.size()){
												lastClicks.pop_back();
											}
											lastClick.x = 160;
											lastClick.y = 0;
											lastClicks.push_back(lastClick);
											if(rect.w > rect.h){
												renderLogicalHeight = (renderLogicalWidth / 4) * 3;
												rect.y = (renderLogicalHeight / 2) - (rect.h / 2);
											}
											else if(rect.h > rect.w){
												renderLogicalWidth = (renderLogicalHeight / 3) * 4;
												rect.x = (renderLogicalWidth / 4) + (renderLogicalHeight / 2) - (rect.w / 2);
											}
											else if(rect.h == rect.w){
												renderLogicalWidth = (renderLogicalHeight / 3) * 4;
												rect.x = image.widthInPixels / 3 - 1;
												rect.y = 0;
											}
											currentWidth = renderLogicalWidth;
											currentHeight = renderLogicalHeight;
										}
									}
									else if(button == &saveButton){
										SaveImage(image, tinyfd_saveFileDialog("Save Image", NULL, 1, saveFilterPatterns, "NiraiSprite image files"));
									}
									else if(button == &openButton){
										filename = tinyfd_openFileDialog("Open Image File", "", 1, saveFilterPatterns, "NiraiSprite image files", 0);
										if(filename) image = nl::LoadImage(filename);
										rect.x = image.widthInPixels/3;
										rect.y = 0;
										rect.w = image.widthInPixels;
										rect.h = image.heightInPixels;
										renderLogicalWidth = rect.w + rect.w / 3;
										renderLogicalHeight = rect.h;
										currentWidth = renderLogicalWidth;
										currentHeight = renderLogicalHeight;
										SDL_RenderSetLogicalSize(renderer, renderLogicalWidth, renderLogicalHeight);
										palette = EditPalette(palette);
										SDL_DestroyTexture(paletteTex);
										paletteTex = SDL_CreateTexture(paletteRenderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STATIC, palette.size > 16 ? 16 : palette.size - 1, palette.size > 16 ? 16 : 1);
										SDL_UpdateTexture(paletteTex, NULL, &palette.color[1], (palette.size - 1) * 2);
										paletteRect.w = palette.size > 16 ? 16 : palette.size - 1;
										paletteRect.h = palette.size > 16 ? 16 : 1;
										data = nl::CreatePixelBuffer(image, palette);
										SDL_DestroyTexture(texture);
										texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STATIC, image.widthInPixels, image.heightInPixels);
										SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
										SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
										while(image.actions.size())
											image.actions.pop_back();
										image.actions.push_back(image.bytes);
										drewYet = 0;
										while(lastClicks.size()){
											lastClicks.pop_back();
										}
										lastClick.x = 160;
										lastClick.y = 0;
										lastClicks.push_back(lastClick);
										if(rect.w > rect.h){
											renderLogicalHeight = (renderLogicalWidth / 4) * 3;
											rect.y = (renderLogicalHeight / 2) - (rect.h / 2);
										}
										else if(rect.h > rect.w){
											renderLogicalWidth = (renderLogicalHeight / 3) * 4;
											rect.x = (renderLogicalWidth / 4) + (renderLogicalHeight / 2) - (rect.w / 2);
										}
										else if(rect.h == rect.w){
											renderLogicalWidth = (renderLogicalHeight / 3) * 4;
											rect.x = image.widthInPixels / 3 - 1;
											rect.y = 0;
										}
										currentWidth = renderLogicalWidth;
										currentHeight = renderLogicalHeight;
									}
							}
						}
					}
					else if(event.window.windowID == SDL_GetWindowID(paletteWindow)){

						switch(brush.type){
							case 0:
								curCur = penCur;
								break;
							case 1:
								curCur = fillCur;
								break;
							case 2:
								curCur = sprayCur;
								break;
						}
						SDL_SetCursor(curCur);

						clickedColor = event.button.x + 1 + event.button.y * 16;
						if(clickedColor < palette.size && clickedColor > 0 && clickedColor != 256){
							brush.color = clickedColor;
							brush.colorValue = palette.color[brush.color];
							SDL_UpdateTexture(cursorTex, NULL, &brush.colorValue, 1);
						}
						else if(clickedColor == 256){
							palette = EditPalette(palette);
							data = nl::CreatePixelBuffer(image, palette);
							SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
							brush.colorValue = palette.color[brush.color];
							SDL_UpdateTexture(cursorTex, NULL, &brush.colorValue, 1);
							ctrl = 0;
							SDL_DestroyTexture(paletteTex);
							paletteTex = SDL_CreateTexture(paletteRenderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STATIC, palette.size > 16 ? 16 : palette.size - 1, palette.size > 16 ? 16 : 1);
							SDL_UpdateTexture(paletteTex, NULL, &palette.color[1], (palette.size - 1) * 2);
							paletteRect.w = palette.size > 16 ? 16 : palette.size - 1;
							paletteRect.h = palette.size > 16 ? 16 : 1;
						}
					}
					break;
				case SDL_BUTTON_RIGHT:
					brush.color = image.bytes[event.button.x - rect.x + ( ( event.button.y - rect.y ) * image.widthInPixels)];
					brush.colorValue = palette.color[brush.color];
					if(!brush.type){
						curCur = brush.color > 0 ? penCur : eraseCur;
						SDL_SetCursor(curCur);
					}
					SDL_UpdateTexture(cursorTex, NULL, &brush.colorValue, 1);
					break;
				}
				break;

			case SDL_MOUSEBUTTONUP:
				switch (event.button.button) {
				case SDL_BUTTON_LEFT:
				leftClick = 0;
				if(event.window.windowID == SDL_GetWindowID(window) && brush.cursor.x >= renderLogicalWidth / 4){
						lastClicks.back().x = brush.cursor.x;
						lastClicks.back().y = brush.cursor.y;

						if(weight && brush.cursor.x >= renderLogicalWidth / 4 && brush.type == 0){
							brush.DrawLine(image.bytes, data, image.widthInPixels, image.heightInPixels, lastX, lastY, rect.x, rect.y, lastDrawingPos.x, lastDrawingPos.y);
							SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
							lastX = brush.cursor.x;
							lastY = brush.cursor.y;
							lastDrawingPos.x = brush.cursor.x;
							lastDrawingPos.y = brush.cursor.y;
						}

						image.actions.push_back(image.bytes);
						if (image.actions.size() > 11) image.actions.pop_front();
					}
					break;
					case SDL_BUTTON_RIGHT:
					break;
				}
				break;

			case SDL_MOUSEMOTION:

			if(event.window.windowID == SDL_GetWindowID(window)){

					if(!focus){
						SDL_RaiseWindow(window);
						focus = 1;
					}

					brush.cursor.x = event.motion.x - brush.cursor.w / 2;
					brush.cursor.y = event.motion.y - brush.cursor.h / 2;

					if(brush.cursor.x < renderLogicalWidth / 4 - 1 && renderLogicalWidth != 640 && renderLogicalHeight != 480 && !leftClick){
						brush.cursor.x = 0;
						currentWidth = renderLogicalWidth;
						currentHeight = renderLogicalHeight;
						renderLogicalWidth = 640;
						renderLogicalHeight = 480;
						SDL_RenderSetLogicalSize(renderer, 640, 480);
						SDL_SetCursor(SDL_GetDefaultCursor());
						shouldRender = 0;
					}
					else if(brush.cursor.x >= renderLogicalWidth / 4 && !shouldRender){
						renderLogicalWidth = currentWidth;
						renderLogicalHeight = currentHeight;
						SDL_RenderSetLogicalSize(renderer, renderLogicalWidth, renderLogicalHeight);
						shouldRender = 1;
						SDL_SetCursor(curCur);
					}
					else if(brush.cursor.x >= renderLogicalWidth / 4 && shouldRender && curIsDefault){
						SDL_SetCursor(curCur);
						curIsDefault = 0;
					}

					if (weight && (brush.cursor.x > lastX + 1 || brush.cursor.x < lastX - 1 || brush.cursor.y > lastY + 1 || brush.cursor.y < lastY - 1) && leftClick && (!brush.type)) {
						brush.DrawLine(image.bytes, data, image.widthInPixels, image.heightInPixels, lastX, lastY, rect.x, rect.y, lastDrawingPos.x, lastDrawingPos.y);
						lastX = lastDrawingPos.x;
						lastY = lastDrawingPos.y;
						SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
					}
					else if((!weight) && (brush.cursor.x > lastX + 1 || brush.cursor.x < lastX - 1 || brush.cursor.y > lastY + 1 || brush.cursor.y < lastY - 1) && leftClick && (!brush.type)){
						brush.DrawLine(image.bytes, data, image.widthInPixels, image.heightInPixels, lastX, lastY, rect.x, rect.y, brush.cursor.x, brush.cursor.y);
						SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
					}

					if (leftClick && (!brush.type) && !weight) {
						brush.Pen(image.bytes, data, brush.cursor.x, brush.cursor.y, rect.x, rect.y, image.widthInPixels, image.heightInPixels);
						SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
					}

					if(leftClick && brush.type == 2){
						brush.Spray(image.bytes, data, brush.cursor.x, brush.cursor.y, rect.x, rect.y, image.widthInPixels, image.heightInPixels);
						SDL_UpdateTexture(texture, NULL, &data[0], image.widthInPixels * 2);
					}

					if(!weight){
						lastX = brush.cursor.x;
						lastY = brush.cursor.y;
					}
					else{
						lastDrawingPos.x = brush.cursor.x;
						lastDrawingPos.y = brush.cursor.y;
					}
				}
				else{
					if(!curIsDefault){
						SDL_SetCursor(SDL_GetDefaultCursor());
						curIsDefault = 1;
					}
					if(focus){
						SDL_RaiseWindow(paletteWindow);
						focus = 0;
					}
				}

				break;

			case SDL_MOUSEWHEEL:

				if(brush.cursor.x >= renderLogicalWidth / 4){
					if (event.wheel.y > 0 && renderLogicalHeight > 16 && renderLogicalWidth > 16) {

						if(rect.w > rect.h){
							renderLogicalWidth -= 8;
						}
						else{
							renderLogicalHeight -= 8;
						}
						rect.x -= 4;
						rect.y -= 4;
					}
					else if (event.wheel.y < 0) {
						if(renderLogicalHeight < 1024){
							if(rect.w > rect.h){
								renderLogicalWidth += 8;
							}
							else{
								renderLogicalHeight += 8;
							}
							rect.x += 4;
							rect.y += 4;
						}
					}

					if(rect.w > rect.h){
						renderLogicalHeight = (renderLogicalWidth / 4) * 3;
					}
					else{
						renderLogicalWidth = (renderLogicalHeight / 3) * 4;
					}
					if(rect.x + rect.w - 5 < renderLogicalWidth / 4){
						rect.x = 4 - (renderLogicalWidth / 4);
					}
					if((rect.x + 5 > renderLogicalWidth || rect.x > renderLogicalWidth) && rect.x > 0){
						rect.x = renderLogicalWidth - 4;
					}
					if((rect.y + 5 > renderLogicalHeight || rect.y > renderLogicalHeight) && rect.y > 0){
						rect.y = renderLogicalHeight - 4;
					}
					if(rect.y + rect.h - 5 < 0){
						rect.y = 4 - rect.h;
					}
					currentWidth = renderLogicalWidth;
					currentHeight = renderLogicalHeight;

					SDL_RenderSetLogicalSize(renderer, renderLogicalWidth, renderLogicalHeight);
				}

				break;
			}
		}

		if (brush.cursor.x != mouseX || brush.cursor.y != mouseY) {

			int distanceX = mouseX - brush.cursor.x;
			int distanceY = mouseY - brush.cursor.y;

			//printf("%i\n%i\n", distanceX, distanceY);



		}

		if(shouldRender){
			SDL_RenderClear(renderer);

			SDL_RenderCopy(renderer, alphaTex, NULL, &rect);
			SDL_RenderCopy(renderer, texture, NULL, &rect);
			if(brush.cursor.x + brush.cursor.w >= rect.x && brush.cursor.x < rect.x + rect.w)
				SDL_RenderCopy(renderer, cursorTex, NULL, &brush.cursor);
			if(shift && drewYet){
				SDL_SetRenderDrawColor(renderer, ((brush.colorValue & 0b0111110000000000) >> 10) * 8, ((brush.colorValue & 0b0000001111100000) >> 5) * 8, (brush.colorValue & 0b00011111) * 8, 255);
				SDL_RenderDrawLine(renderer, lastClicks.back().x, lastClicks.back().y, brush.cursor.x, brush.cursor.y);
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			}
			SDL_RenderSetLogicalSize(renderer, 640, 480);
			SDL_RenderCopy(renderer, sidebarTex, NULL, &sidebarRect);
			SDL_RenderSetLogicalSize(renderer, renderLogicalWidth, renderLogicalHeight);

			SDL_RenderPresent(renderer);
		}
		SDL_RenderClear(paletteRenderer);
		SDL_RenderCopy(paletteRenderer, paletteTex, NULL, &paletteRect);

		SDL_RenderSetLogicalSize(paletteRenderer, 256, 256);
		SDL_RenderCopy(paletteRenderer, pEditTex, NULL, &pEditRect);

		SDL_RenderPresent(paletteRenderer);
		SDL_RenderSetLogicalSize(paletteRenderer, 16, 16);

	}

	SDL_DestroyWindow(window);
	SDL_DestroyWindow(paletteWindow);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyRenderer(paletteRenderer);
	SDL_DestroyTexture(texture);
	SDL_DestroyTexture(alphaTex);
	SDL_DestroyTexture(paletteTex);
	SDL_DestroyTexture(pEditTex);
	SDL_FreeCursor(penCur);
	SDL_FreeCursor(fillCur);
	SDL_FreeCursor(eraseCur);
	SDL_FreeCursor(sprayCur);
	SDL_FreeCursor(curCur);
	SDL_Quit();

	return 0;
}

nl::Palette	EditPalette(nl::Palette palette) {

	bool isNew = (palette.size == 0);

	nl::Palette tmpPlt;

	if( ! palette.size ){
		palette.color.reserve(16);
		for(short i = 0; i < 16; ++i)
			palette.color.push_back(0);
		palette.size = 1;
	}

	nl::Palette nullplt;

	uint16_t color = 0;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("PaletteMage", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 480, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawColor(renderer, 0x88, 0x88, 0x88, 255);

	uint16_t textPalette[2] = { 0b0000000000000000, 0b0111111111111111 };

	SDL_Texture* colors[16] = {

		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1),
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1)

	};

	for(short i = 0; i < palette.size - 1; ++i){
		SDL_UpdateTexture(colors[i+1], NULL, &palette.color[i+1],2);
	}

	SDL_Rect slots[15];

	for (int i = 0; i < 15; ++i) {

		slots[i].y = 420;
		slots[i].x = 40 + 32 * i;
		slots[i].w = 32;
		slots[i].h = 32;

	}

	SDL_Texture* colorTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 1, 1);
	SDL_UpdateTexture(colorTex, NULL, &color, 2);

	constexpr uint8_t letterR[64] =
	{
		0,1,1,1,1,1,0,0,
		0,1,0,0,0,0,1,0,
		0,1,0,0,0,0,1,0,
		0,1,1,1,1,1,0,0,
		0,1,0,0,1,0,0,0,
		0,1,0,0,0,1,0,0,
		0,1,0,0,0,0,1,0,
		0,0,0,0,0,0,0,0
	};

	constexpr uint8_t letterG[64] =
	{
		0,0,1,1,1,1,1,0,
		0,1,0,0,0,0,0,0,
		0,1,0,0,0,0,0,0,
		0,1,0,0,0,0,0,0,
		0,1,0,0,1,1,1,0,
		0,1,0,0,0,0,1,0,
		0,0,1,1,1,1,0,0,
		0,0,0,0,0,0,0,0
	};

	constexpr uint8_t letterB[64] =
	{
		0,1,1,1,1,1,0,0,
		0,1,0,0,0,0,1,0,
		0,1,0,0,0,0,1,0,
		0,1,1,1,1,1,0,0,
		0,1,0,0,0,0,1,0,
		0,1,0,0,0,0,1,0,
		0,1,1,1,1,1,0,0,
		0,0,0,0,0,0,0,0
	};

	uint16_t imageR[64], imageG[64], imageB[64];

	for (int i = 0; i < 64; ++i) {
		imageR[i] = textPalette[letterR[i]];
		imageG[i] = textPalette[letterG[i]];
		imageB[i] = textPalette[letterB[i]];
	}

	SDL_Texture* rTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 8, 8);
	SDL_UpdateTexture(rTex, NULL, &imageR[0], 16);

	SDL_Texture* gTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 8, 8);
	SDL_UpdateTexture(gTex, NULL, &imageG[0], 16);

	SDL_Texture* bTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB1555, SDL_TEXTUREACCESS_STREAMING, 8, 8);
	SDL_UpdateTexture(bTex, NULL, &imageB[0], 16);

	SDL_Rect colorRect, rectR, rectG, rectB;
	colorRect.x = 40;
	colorRect.y = 80;
	colorRect.w = 320;
	colorRect.h = 320;

	rectR.x = 468;
	rectR.y = 208;
	rectR.w = 32;
	rectR.h = 32;

	rectG.x = 532;
	rectG.y = 208;
	rectG.w = 32;
	rectG.h = 32;

	rectB.x = 596;
	rectB.y = 208;
	rectB.w = 32;
	rectB.h = 32;

	uint8_t rgb[3] = {0, 0, 0};

	printf("Init letters\n");

	bool running = 1;
	SDL_Event event;

	uint8_t selectedLetter = 1;

	for (int i = 0; i < 64; ++i) {
		if (imageR[i] == 0b0111111111111111) imageR[i] = 0b0111110000000000;
	}
	SDL_UpdateTexture(rTex, NULL, &imageR[0], 16);



	uint16_t paletteIndex = 0;
	slots[0].y -= 20;

	bool ctrl = 0, shift = 0;

	char * filename;

	const char* paletteFilterPatterns[1] = { "*.plt" };
	const char* hexFilterPatterns[1] = { "*.hex" };

	const char* helpMessage = "Right/Left = switch selected R/G/B channel\n\nUp/Down = change R/G/B value\n\nCtrl+Right/Ctrl+Left = switch selected color index\n\nSpace = write color to palette index\n\nTab = copy selected color to editor\n\nBackspace/Del = delete color\n\nEnter = Finish and save palette\n\nCtrl + O = open NiraiSprite palette file (will discard current palette)\n\nCtrl + Shift + H = Open .hex palette\n\nCtrl + R = Randomize colors\n\nNiraiSprite WILL ONLY USE THE FIRST 15 COLORS OF HEX PALETTE.\nColor depth may suffer from conversion from 24- to 16-bit.";

	while (running) {

		while (SDL_PollEvent(&event)) {

			switch (event.type) {

			case SDL_WINDOWEVENT:
				if(event.window.event == SDL_WINDOWEVENT_CLOSE)
					running = 0;
				break;
			case SDL_KEYDOWN:

				switch (event.key.keysym.sym) {

					case SDLK_h:
						if(!ctrl)SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,"Keybinds", helpMessage, window);
						else if(ctrl && shift){
							filename = tinyfd_openFileDialog("Open Hex Palette File", "", 1, hexFilterPatterns, ".hex palette files", 0);
							if(filename){
								palette = nl::LoadHexPalette(filename);
								if(palette.size){
									for(short i = 0; i < 16; ++i)
										SDL_UpdateTexture(colors[i], NULL, &palette.color[i], 2);
									isNew = 0;
								}
							}
						}
					break;
					case SDLK_r:
						if(ctrl){
							palette = RandomPalette(palette.size);
							for(short i = 0; i < palette.size; ++i)
								SDL_UpdateTexture(colors[i], NULL, &palette.color[i], 2);
						}
					break;
					case SDLK_SLASH:
					SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,"Keybinds", helpMessage, window);
					break;
					case LMOD:
						ctrl = 1;
					break;
					case RMOD:
						ctrl = 1;
					break;
					case SDLK_LSHIFT:
						shift = 1;
					break;
					case SDLK_RSHIFT:
						shift = 1;
					break;
				case SDLK_RIGHT:
					if(ctrl){
						if(paletteIndex < 14 && paletteIndex < palette.size - 1){
							slots[paletteIndex].y += 20;
							++paletteIndex;
							slots[paletteIndex].y -= 20;
						}
					}else{
						if (selectedLetter == 1) {
							selectedLetter = 2;
							for (int i = 0; i < 64; ++i) {
								if (imageG[i] == 0b0111111111111111) imageG[i] = 0b0000001111100000;
							}
							SDL_UpdateTexture(gTex, NULL, &imageG[0], 16);
							for (int i = 0; i < 64; ++i) {
								if (imageR[i] == 0b0111110000000000) imageR[i] = 0b0111111111111111;
							}
							SDL_UpdateTexture(rTex, NULL, &imageR[0], 16);
						}
						else if (selectedLetter == 2) {
							selectedLetter = 3;
							for (int i = 0; i < 64; ++i) {
								if (imageB[i] == 0b0111111111111111) imageB[i] = 0b0000000000011111;
							}
							SDL_UpdateTexture(bTex, NULL, &imageB[0], 16);
							for (int i = 0; i < 64; ++i) {
								if (imageG[i] == 0b0000001111100000) imageG[i] = 0b0111111111111111;
							}
							SDL_UpdateTexture(gTex, NULL, &imageG[0], 16);
						}
					}
					break;
					case SDLK_PERIOD:
						if(ctrl){
							if(paletteIndex < 14 && paletteIndex < palette.size - 1){
								slots[paletteIndex].y += 20;
								++paletteIndex;
								slots[paletteIndex].y -= 20;
							}
						}else{
							if (selectedLetter == 1) {
								selectedLetter = 2;
								for (int i = 0; i < 64; ++i) {
									if (imageG[i] == 0b0111111111111111) imageG[i] = 0b0000001111100000;
								}
								SDL_UpdateTexture(gTex, NULL, &imageG[0], 16);
								for (int i = 0; i < 64; ++i) {
									if (imageR[i] == 0b0111110000000000) imageR[i] = 0b0111111111111111;
								}
								SDL_UpdateTexture(rTex, NULL, &imageR[0], 16);
							}
							else if (selectedLetter == 2) {
								selectedLetter = 3;
								for (int i = 0; i < 64; ++i) {
									if (imageB[i] == 0b0111111111111111) imageB[i] = 0b0000000000011111;
								}
								SDL_UpdateTexture(bTex, NULL, &imageB[0], 16);
								for (int i = 0; i < 64; ++i) {
									if (imageG[i] == 0b0000001111100000) imageG[i] = 0b0111111111111111;
								}
								SDL_UpdateTexture(gTex, NULL, &imageG[0], 16);
							}
						}
						break;
				case SDLK_LEFT:
					if(ctrl){
						if(paletteIndex){
							slots[paletteIndex].y += 20;
							--paletteIndex;
							slots[paletteIndex].y -= 20;
						}
					}else{
						if (selectedLetter == 3) {
							selectedLetter = 2;
							for (int i = 0; i < 64; ++i) {
								if (imageB[i] == 0b0000000000011111) imageB[i] = 0b0111111111111111;
							}
							SDL_UpdateTexture(bTex, NULL, &imageB[0], 16);
							for (int i = 0; i < 64; ++i) {
								if (imageG[i] == 0b0111111111111111) imageG[i] = 0b0000001111100000;
							}
							SDL_UpdateTexture(gTex, NULL, &imageG[0], 16);
						}
						else if (selectedLetter == 2) {
							selectedLetter = 1;
							for (int i = 0; i < 64; ++i) {
								if (imageG[i] == 0b0000001111100000) imageG[i] = 0b0111111111111111;
							}
							SDL_UpdateTexture(gTex, NULL, &imageG[0], 16);
							for (int i = 0; i < 64; ++i) {
								if (imageR[i] == 0b0111111111111111) imageR[i] = 0b0111110000000000;
							}
							SDL_UpdateTexture(rTex, NULL, &imageR[0], 16);
						}
					}
					break;
					case SDLK_COMMA:
						if(ctrl){
							if(paletteIndex){
								slots[paletteIndex].y += 20;
								--paletteIndex;
								slots[paletteIndex].y -= 20;
							}
						}else{
							if (selectedLetter == 3) {
								selectedLetter = 2;
								for (int i = 0; i < 64; ++i) {
									if (imageB[i] == 0b0000000000011111) imageB[i] = 0b0111111111111111;
								}
								SDL_UpdateTexture(bTex, NULL, &imageB[0], 16);
								for (int i = 0; i < 64; ++i) {
									if (imageG[i] == 0b0111111111111111) imageG[i] = 0b0000001111100000;
								}
								SDL_UpdateTexture(gTex, NULL, &imageG[0], 16);
							}
							else if (selectedLetter == 2) {
								selectedLetter = 1;
								for (int i = 0; i < 64; ++i) {
									if (imageG[i] == 0b0000001111100000) imageG[i] = 0b0111111111111111;
								}
								SDL_UpdateTexture(gTex, NULL, &imageG[0], 16);
								for (int i = 0; i < 64; ++i) {
									if (imageR[i] == 0b0111111111111111) imageR[i] = 0b0111110000000000;
								}
								SDL_UpdateTexture(rTex, NULL, &imageR[0], 16);
							}
						}
						break;
				case SDLK_UP:
					if (rgb[selectedLetter - 1] < 31) {
						++rgb[selectedLetter - 1];
						//printf("%i\n", rgb[selectedLetter - 1]);
						color = rgb[0] << 10;
						color |= rgb[1] << 5;
						color |= rgb[2];
						SDL_UpdateTexture(colorTex, NULL, &color, 2);
					}
					printf("\rR: %d    G: %d    B: %d    ", rgb[0], rgb[1], rgb[2]);
					break;
				case SDLK_DOWN:
					if (rgb[selectedLetter - 1] > 0) {
						--rgb[selectedLetter - 1];
						//printf("%i\n", rgb[selectedLetter - 1]);
						color = rgb[0] << 10;
						color |= rgb[1] << 5;
						color |= rgb[2];
						SDL_UpdateTexture(colorTex, NULL, &color, 2);
					}
					printf("\rR: %d    G: %d    B: %d    ", rgb[0], rgb[1], rgb[2]);
					break;
				case SDLK_SPACE:
					if (paletteIndex < 14) {
						palette.color[paletteIndex+1] = (color | 0b1000000000000000);
						palette.size += (paletteIndex == (palette.size - 1));
						slots[paletteIndex].y += 20;
						++paletteIndex;
						slots[paletteIndex].y -= 20  * (paletteIndex <= 14);
						SDL_UpdateTexture(colors[paletteIndex], NULL, &palette.color[paletteIndex], 2);
					}
					else{
						palette.color[15] = (color | 0b1000000000000000);
						palette.size = 16;
						paletteIndex = 14;
						SDL_UpdateTexture(colors[15], NULL, &palette.color[15], 2);
					}
					break;
				case SDLK_RETURN:

					if (palette.size > 1) running = 0;

					else SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "no colors", "You have not entered any colors.\nTo add a color to the palette, adjust the RGB\nvalues with the arrow keys and press space.", window);
					break;
				case SDLK_BACKSPACE:
					if (paletteIndex > 0) {
						slots[paletteIndex].y += 20;
						palette.color.erase(palette.color.begin() + paletteIndex + 1);
						palette.color.push_back(0);
						--paletteIndex;
						slots[paletteIndex].y -= 20;
						--palette.size;
						SDL_UpdateTexture(colors[palette.size], NULL, &palette.color[0], 2);
					}
				break;
				case SDLK_TAB:
					color = palette.color[paletteIndex+1];
					rgb[0] = (color & 0b111110000000000) >> 10;
					rgb[1] = (color & 0b1111100000) >> 5;
					rgb[2] = color & 0x1f;
					SDL_UpdateTexture(colorTex, NULL, &color, 2);
				break;
				case SDLK_o:
					if(ctrl){
						filename = tinyfd_openFileDialog("Open Palette File", "", 1, paletteFilterPatterns, "NiraiSprite palette files", 0);
						if(filename){
							palette = nl::LoadPalette(filename);
							if(palette.size){
								for(short i = 0; i < 16; ++i)
									SDL_UpdateTexture(colors[i], NULL, &palette.color[i], 2);
								isNew = 0;
							}
						}
					}
				break;
				}

				break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym) {
						case LMOD:
						 ctrl = 0;
						break;
						case RMOD:
						 ctrl = 0;
						break;
						case SDLK_LSHIFT:
							shift = 0;
						break;
						case SDLK_RSHIFT:
							shift = 0;
						break;
					}
				break;


			}
		}

		SDL_RenderClear(renderer);

		SDL_RenderCopy(renderer, colorTex, NULL, &colorRect);

		SDL_RenderCopy(renderer, rTex, NULL, &rectR);
		SDL_RenderCopy(renderer, gTex, NULL, &rectG);
		SDL_RenderCopy(renderer, bTex, NULL, &rectB);

		for (int i = 0; i < 15; ++i) {

			SDL_RenderCopy(renderer, colors[i+1], NULL, &slots[i]);

		}

		SDL_RenderPresent(renderer);

	}

	SDL_DestroyWindow(window);
	if(isNew){
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
	}

	printf("Quit PaletteMage\n isNew = %i\n",isNew);

	if(!isNew)
		SavePalette(palette, tinyfd_saveFileDialog("Save Palette", NULL, 1, paletteFilterPatterns, "NiraiSprite Palette Files"));

	nl::Palette foo;

	foo.color.push_back(0);
	foo.color.push_back(0b1111111111111111);
	foo.size = 2;

	if (palette.size < 2) return foo;
	else return palette;
}

nl::Palette RandomPalette(unsigned short size)
{
	nl::Palette palette;
	palette.size = size;

	printf("Creating random palette.\n");
/*
_getsize:
	printf("How many colors in the new palette?\n");
	std::cin >> palette.size;

	if (palette.size > 16) {
		printf("Size must be less than or equal to 16.\n");
		goto _getsize;
	}
	else if (palette.size == 0) {
		printf("size must be greater than 0.\n");
		goto _getsize;
	}
*/
	uint16_t rgb[3];
	uint16_t color = 0;

		palette.color.push_back(0b0000000000000000);

	for (int i = 1; i < palette.size; ++i) {

		for (int j = 0; j < 3; ++j) {
			rgb[j] = rand() % 31;
		}

		color = rgb[0] << 10;
		color |= rgb[1] << 5;
		color |= rgb[2];

		color |= 0b1000000000000000;

		palette.color.push_back(color);

	}

	return palette;
}

void	SavePalette(nl::Palette palette, const char* filename) {

	std::ofstream file;

	file.open(filename, std::ios::out | std::ios::binary);

	file << palette.size;

	uint8_t bytes[2];

	for (int i = 0; i < palette.size; ++i) {

		bytes[0] = palette.color[i] >> 8;
		file << bytes[0];
		bytes[1] = palette.color[i] & 0xff;
		file << bytes[1];

	}

	file.close();

}

nl::Image	NewImage() {

	printf("creating image structure\n");
	nl::Image image;

	printf("creating width and height variables\n");

	uint16_t width = 0, height = 0;

	char* sizeholder = "00";

_getw:
	sizeholder = (tinyfd_inputBox("Set image width","Please enter the width for the image in 8x8-pixel tiles (1-128).","4, for example"));
	if(sizeholder == NULL){
		return image;
	}
	width = std::stoi(sizeholder);
	if (width <= 0) {
		int messagebox = tinyfd_messageBox("Invalid size","Image width cannot be 0.", "ok", "error", 0);
		goto _getw;
	}
	else if (width > 128) {
		{
		int messagebox = tinyfd_notifyPopup("Invalid size","Image width must not be greater than 128 tiles.","error");
		goto _getw;
		}
	}
	image.widthInPixels = width * 8;
	image.w = width;
_geth:
	sizeholder = tinyfd_inputBox("Set image height","Please enter the width height for the image in 8x8-pixel tiles (1-128).","4, for example");
	if(sizeholder == NULL){
		return image;
	}
	height = std::stoi(sizeholder);
	if (height <= 0) {
		{
		int messagebox = tinyfd_messageBox("Invalid size","Image height cannot be 0.", "ok", "error", 0);
		goto _geth;
		}
	}
	if (height > 128) {
		{
		int messagebox = tinyfd_notifyPopup("Invalid size","Image height must not be greater than 128 tiles.","error");
		goto _geth;
		}
	}
	image.heightInPixels = height * 8;
	image.h = height;
	for (int i = 0; i < image.widthInPixels * image.heightInPixels; ++i) {
		image.bytes.push_back(0);
	}

	return image;
}

void	SaveImage(nl::Image image, char* filename) {

	std::ofstream file;

	//printf("Name for image file to save?\n");
	//std::cin >> filename;

	file.open(filename, std::ios::out | std::ios::binary);

	int compress = 0;

_getcomp:
	//printf("Compress image? 1/0\n");
	//std::cin >> compress;
	compress = tinyfd_messageBox("Compression", "Compress Image?", "yesno", "question", 1);
	if (compress != 0 && compress != 1) {
		printf("Non-boolean value was entered; please enter 1 for true or 0 for false.\n");
		goto _getcomp;
	}
	image.compress = compress;

	-- image.h;
	-- image.w;

	if (image.compress) image.h ^= 0b10000000;

	file << image.w;
	file << image.h;

	if (image.compress) {
		printf("Compressing image.\n");
		image.bytes = CompressRLE(image.bytes);
		printf("Image compressed.\n");
	}

	printf("Saving image.\n");

	uint8_t byte = 0;

	for (int i = 0; i < image.bytes.size(); ++i) {

		if(!image.compress){
			byte = image.bytes[i] << 4;

			if (i != image.bytes.size() - 1) {
				++i;
				byte |= image.bytes[i];
			}

			file << std::noskipws << byte;
	  }
		else file << std::noskipws << image.bytes[i];

	}

	file.close();
	printf("Image saved.\n");
}

std::vector<uint8_t> CompressRLE(std::vector<uint8_t> bytes) {

  uint8_t newByte = 0;
	std::vector<uint8_t> newBytes;

	uint8_t consecutiveBytes = 0;

	uint8_t currentByte = 0;

	for (int i = 0; i < bytes.size(); i += consecutiveBytes) {

		consecutiveBytes = 0;
		currentByte = bytes[i];

		while (bytes[i + consecutiveBytes] == currentByte) {
			if (consecutiveBytes == 15) break;
			++consecutiveBytes;
			if (i + consecutiveBytes == bytes.size()) break;
		}

    newByte = consecutiveBytes << 4;
		newByte |= currentByte;
		newBytes.push_back(newByte);

		if (i == bytes.size()) break;
	}

	return newBytes;
}

void SaveImageToBMP(std::vector<uint16_t> data, uint16_t width, uint16_t height) {

	char* bmpfilter[1] = {"*.bmp"};

	char* filename = tinyfd_saveFileDialog("Save Image", NULL, 1, bmpfilter	, "Windows Bitmap files");

	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom( & data[0], width, height, 15, width * 2, SDL_PIXELFORMAT_ARGB1555);

	SDL_SaveBMP(surface, filename);

	printf("BMP saved.\n");

	SDL_FreeSurface(surface);

}
