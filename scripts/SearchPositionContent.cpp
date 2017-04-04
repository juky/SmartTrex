// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <windows.h>
#include <conio.h>

#include <fstream>
#include <vector>
#include <ctime>

using namespace std;

const char *_dinoPixelFile = "DinoPixels.txt";
#define DINO_COUNT_PIXELS 6732
#define DINO_TO_WINDOW_HEIGHT 107
const int floorPixelsTowardstDino = 45; //example dino is at y10 floor will be at dinoY+this variable (kijkt nu naar bovenste van de 2pixel line)


static bool IsPixelWhite(RGBQUAD pixel)
{
	const int pixelWhiteColorMargin = 200;//every value under this one is seen as black

	if ((int)pixel.rgbBlue>pixelWhiteColorMargin && (int)pixel.rgbGreen>pixelWhiteColorMargin && (int)pixel.rgbRed>pixelWhiteColorMargin)
	{
		return true;
	}

	return false;
}


void DrawRectAt(RECT r) {

	HDC screen = GetDC(NULL);

	HBRUSH br = CreateSolidBrush(RGB(0, 255, 0));

	FillRect(screen, &r, br);

	_getch();
	InvalidateRect(NULL, &r, true);
}



vector<RGBQUAD> GetScreenPixels(int nScreenWidth,int nScreenHeight,int startXPos,int startYPos)
{
	HWND hDesktopWnd = GetDesktopWindow();//to let other functions now what window they are working with
	HDC hDesktopDC = GetDC(hDesktopWnd);//NULL = get dc that covers entire screen
	HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);// creates a memory DC compatible with the application's current screen
	HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC, nScreenWidth, nScreenHeight);
	SelectObject(hCaptureDC, hCaptureBitmap);//replace object

	BitBlt(hCaptureDC, startXPos, startYPos, nScreenWidth, nScreenHeight, hDesktopDC, 0, 0, SRCCOPY | CAPTUREBLT);//performs a bit-block transfer of the color data corresponding to a rectangle of pixels from the specified source device context into a destination device context
																								  //desitination, x-cor dest rect ,y-cor  dest rect ,width, height, a handle to the source device, x-cor src rect ,y-cor src rect ,define how color data for src rect is combined with color data dest rect (to achieve final color) 

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = nScreenWidth;
	bmi.bmiHeader.biHeight = nScreenHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	vector<RGBQUAD> pPixels(nScreenWidth * nScreenHeight);

	GetDIBits(
		hCaptureDC,
		hCaptureBitmap,
		0,
		nScreenHeight,
		&pPixels[0],
		&bmi,
		DIB_RGB_COLORS
	);

	ReleaseDC(hDesktopWnd, hDesktopDC);
	DeleteDC(hCaptureDC);
	DeleteObject(hCaptureBitmap);

	return pPixels;
}




void WriteDinoToFileAt(int minX, int maxX, int minY, int maxY)//the "min && max" x y pixels are all read and written to the file (minX=first pixel to be read maxX=last pixel to be read) 
{
	cout << "Press enter to continue" << endl;
	cin.get();

	SetProcessDPIAware();

	int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	HWND hDesktopWnd = GetDesktopWindow();//to let other functions now what window they are working with
	HDC hDesktopDC = GetDC(hDesktopWnd);//NULL = get dc that covers entire screen
	HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);// creates a memory DC compatible with the application's current screen
	HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC, nScreenWidth, nScreenHeight);
	SelectObject(hCaptureDC, hCaptureBitmap);//replace object

	BitBlt(hCaptureDC, 0, 0, nScreenWidth, nScreenHeight, hDesktopDC, 0, 0, SRCCOPY | CAPTUREBLT);//performs a bit-block transfer of the color data corresponding to a rectangle of pixels from the specified source device context into a destination device context
																								  //desitination, x-cor dest rect ,y-cor  dest rect ,width, height, a handle to the source device, x-cor src rect ,y-cor src rect ,define how color data for src rect is combined with color data dest rect (to achieve final color) 

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = nScreenWidth;
	bmi.bmiHeader.biHeight = nScreenHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	RGBQUAD *pPixels = new RGBQUAD[nScreenWidth * nScreenHeight];

	GetDIBits(
		hCaptureDC,
		hCaptureBitmap,
		0,
		nScreenHeight,
		pPixels,
		&bmi,
		DIB_RGB_COLORS
	);


	FILE *pFile;
	errno_t err = fopen_s(&pFile, _dinoPixelFile, "w");//w= create new file if exist remake, b= binary
	if (err != 0)
	{
		printf("Error opening dinoFile");
		exit(1);
	}
	if(maxX>nScreenWidth||maxY>nScreenHeight||minX<0||minY<0)
	{
		printf("Error Dino out of bounds");
		exit(1);
	}

	int widthDino = maxX-minX+1;
	int heightDino = (maxY - minY + 1);
	fwrite(&widthDino, sizeof(int), 1, pFile);

	std::cout << "Dino width=" << widthDino << " height=" << heightDino << endl;

	int p;
	int x, y;
	for (y = minY; y <= maxY; y++) {
		for (x = minX; x <= maxX; x++) {
			p = (nScreenHeight - y - 1)*nScreenWidth + x; // upside down
			unsigned char r = pPixels[p].rgbRed;
			unsigned char g = pPixels[p].rgbGreen;
			unsigned char b = pPixels[p].rgbBlue;

			fwrite(&r, sizeof(char), 1, pFile);
			fwrite(&g, sizeof(char), 1, pFile);
			fwrite(&b, sizeof(char), 1, pFile);

			if(x == minX&&y == minY ||x==maxX&&y==maxY)
			{
				std::cout << "P "<<p << " at:" << x << "," << y <<endl ;
				std::cout << (int)r;
				std::cout << ", " << (int)g;
				std::cout << ", " << (int)b << std::endl;
			}
		}
	}
	fclose(pFile);

	delete[] pPixels;

	ReleaseDC(hDesktopWnd, hDesktopDC);
	DeleteDC(hCaptureDC);
	DeleteObject(hCaptureBitmap);
	cout << "Successfully wrote all pixels to file: "<< _dinoPixelFile << endl;
	if(DINO_COUNT_PIXELS!=(widthDino*heightDino*3))
		cout << "Please, Change 'DINO_COUNT_PIXELS' to " << (widthDino*heightDino * 3) << endl;
}


void GetScreenSize(int& nScreenWidth, int& nScreenHeight)
{
	SetProcessDPIAware();

	nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
}



vector<RGBQUAD> ReadDinoPixels(int& arrayWidth)
{
	FILE *pFile;
	errno_t err = fopen_s(&pFile, _dinoPixelFile, "r");//w= create new file if exist remake, b= binary
	if (err != 0||pFile==NULL)
	{
		printf("Error opening dinoFile");
		exit(1);
	}

	int dinoWidth[1];
	unsigned char dinoRGBPixels[DINO_COUNT_PIXELS];
	size_t result = fread(dinoWidth,sizeof(int), 1, pFile);
	result = fread(dinoRGBPixels,sizeof(char), DINO_COUNT_PIXELS, pFile);
	
	//std::cout << dinoWidth[0] << " ";
	//std::cout << (int)dinoRGBPixels[0] << " ";
	//std::cout << dinoWidth[0] << " ";
	//std::cout << (int)dinoRGBPixels[DINO_COUNT_PIXELS-3] << " ";
	//std::cout << (int)dinoRGBPixels[DINO_COUNT_PIXELS-2] << " ";
	//std::cout << (int)dinoRGBPixels[DINO_COUNT_PIXELS-1] << " ";

	vector<RGBQUAD> dinoPixelsInRGB(DINO_COUNT_PIXELS / 3);

	int j = 0;
	for (int i=0; i<sizeof(dinoRGBPixels);i+=3)
	{
		j = i / 3;
		dinoPixelsInRGB[j].rgbRed = dinoRGBPixels[i];
		dinoPixelsInRGB[j].rgbGreen = dinoRGBPixels[i+1];
		dinoPixelsInRGB[j].rgbBlue = dinoRGBPixels[i+2];
	}

	/*std::cout << dinoWidth[0] << " nu ";
	std::cout << (int)dinoPixelsInRGB[DINO_COUNT_PIXELS/3-1].rgbRed << " ";
	std::cout << (int)dinoPixelsInRGB[DINO_COUNT_PIXELS/3-1].rgbGreen << " ";
	std::cout << (int)dinoPixelsInRGB[DINO_COUNT_PIXELS/3-1].rgbBlue << " ";*/

	arrayWidth = dinoWidth[0];
	fclose(pFile);

	return dinoPixelsInRGB;
}




struct Vector2
{
public:
	double x;
	double y;

	Vector2(double x, double y);
};

Vector2::Vector2(double startX, double startY)
{
	x = startX;
	y = startY;
}



void FindWindowGameSize(int& startXPos, int& endXPos,vector<RGBQUAD> PixelsWholeScreen, Vector2 dinoPos,int nscreenWidth,int nScreenHeight)//returns -1 if nothing was found
{
	startXPos = -1;
	endXPos = -1;
	const int maxWhiteSpace = 5;
	int firstWhitePixel = -1;
	int p;

	for (int x = dinoPos.x; x >= 0; x--)
	{
		p = (nScreenHeight - (dinoPos.y+ floorPixelsTowardstDino) - 1)*nscreenWidth + x; // upside down

		if (firstWhitePixel == -1)
		{
			if(IsPixelWhite(PixelsWholeScreen[p]))
			{
				cout << "found white space at " << x << endl;
				firstWhitePixel = x;
			}
			else
			{
				cout << "No white space at " << x << endl;
			}
		}
		else if (!IsPixelWhite(PixelsWholeScreen[p]))
		{
			cout << "found back black pixel at " << x << endl;

			if (firstWhitePixel - x>maxWhiteSpace)//check if the white gap is caused by a cactus
			{
				cout << "abort " << firstWhitePixel - x << endl;

				int prevPixelOneUp = (nScreenHeight - (dinoPos.y + floorPixelsTowardstDino-1) - 1)*nscreenWidth + x+1;
				int prevPixelOneDown = (nScreenHeight - (dinoPos.y + floorPixelsTowardstDino + 1) - 1)*nscreenWidth + x + 1;

				if (!IsPixelWhite(PixelsWholeScreen[prevPixelOneUp]) || !IsPixelWhite(PixelsWholeScreen[prevPixelOneDown]))//check if the white gap is caused by a hill
				{
					firstWhitePixel = -1;
				}
				else
				{
					cout << "Found border of chrome at " << x << "," << (dinoPos.y + floorPixelsTowardstDino) << endl;
					startXPos = firstWhitePixel;
					break;//Found border of chrome
				}
			}
			else
			{
				firstWhitePixel = -1;//abort
			}
		}
	}
	if (firstWhitePixel == -1)
		return;

	firstWhitePixel = -1;

	for (int x = dinoPos.x+50; x <nscreenWidth; x++)// 50 pixels for the dino width
	{
		p = (nScreenHeight - (dinoPos.y + floorPixelsTowardstDino) - 1)*nscreenWidth + x; // upside down

		if (firstWhitePixel == -1)
		{
			if (IsPixelWhite(PixelsWholeScreen[p]))
			{
				cout << "found white space at " << x << endl;
				firstWhitePixel = x;
			}
			else
			{
				cout << "No white space at " << x << endl;
			}
		}
		else if (!IsPixelWhite(PixelsWholeScreen[p]))
		{
			cout << "found back black pixel at " << x << endl;

			if (x-firstWhitePixel>maxWhiteSpace)//check if the white gap is caused by a cactus
			{
				cout << "abort " << x-firstWhitePixel << endl;

				int prevPixelOneUp = (nScreenHeight - (dinoPos.y + floorPixelsTowardstDino - 1) - 1)*nscreenWidth + x - 1;
				int prevPixelOneDown = (nScreenHeight - (dinoPos.y + floorPixelsTowardstDino + 1) - 1)*nscreenWidth + x - 1;

				if (!IsPixelWhite(PixelsWholeScreen[prevPixelOneUp]) || !IsPixelWhite(PixelsWholeScreen[prevPixelOneDown]))//check if the white gap is caused by a hill
				{
					firstWhitePixel = -1;
				}
				else
				{
					cout << "Found border of chrome at " << x << "," << (dinoPos.y + floorPixelsTowardstDino) << endl;
					break;//Found border of chrome
				}
			}
			else
			{
				firstWhitePixel = -1;//abort
			}
		}
	}

	if(firstWhitePixel==-1)
		return;

	startXPos = startXPos;
	endXPos = firstWhitePixel;
}



void SearchXSizeWindowInWholeScreen(int& startXPos,int& endXPos,Vector2 dinoPos)
{
	//cout << "Press enter to continue" << endl;
	//cin.get();

	int dinoImageWidth = 0;
	vector<RGBQUAD> dinoImage = ReadDinoPixels(dinoImageWidth);

	int screenWidth = 0;
	int screenHeight = 0;
	GetScreenSize(screenWidth, screenHeight);
	vector<RGBQUAD> pPixels = GetScreenPixels(screenWidth, screenHeight, 0, 0);


	FindWindowGameSize(startXPos, endXPos,pPixels, dinoPos, screenWidth, screenHeight);

}


Vector2 TryToFindDino(vector<RGBQUAD> fullImage, int fullImageWidth, int fullImageHeight, vector<RGBQUAD> dinoImage, int dinoImageWidth, int dinoImageHeight) {

	for (int y = 0; y < fullImageHeight - dinoImageHeight+1; y++)           // for every Y values of the screen, minus dinoSize because out of bounds
	{
		for (int x = 0; x < (fullImageWidth- dinoImageWidth+1); x++)      // continue to iterate over all X values of the screen, minus dinoSize because out of bounds
		{
			int p = (fullImageHeight - y - 1)*fullImageWidth + x; // upside down

			bool matchesValueX = true;
			for (int dy = 0; dy < dinoImageHeight; dy++)                // iterate over all Y values of the dino image (relevant)? possibly missing something here
			{
				for (int dx = 0; dx < dinoImageWidth; dx++)         // continue to iterate over X values of the dino image
				{
					int dp = dy * dinoImageWidth + dx;
					int evenX = x + dx;
					int evenY = y + dy;
					int even = (fullImageHeight - evenY - 1)*fullImageWidth +evenX;

					if(!IsPixelWhite(dinoImage[dp]))
					{
						if (fullImage[even].rgbBlue != dinoImage[dp].rgbBlue || fullImage[even].rgbGreen != dinoImage[dp].rgbGreen || fullImage[even].rgbRed != dinoImage[dp].rgbRed) //if value of fullImage is not equal to dinoImage, break; (
						{
							/*if (x == 980 && y == 220)
							{
							cout << "Fault at " <<dx<<","<<dy<<" "<<dp<<"  "<< (int)fullImage[even].rgbBlue <<" vs "<<(int)dinoImage[dp].rgbBlue << endl;
							continue;
							}*/

							matchesValueX = false;
							break;
						}
					}

					if (dx == dinoImageWidth - 1 && dy == dinoImageHeight - 1) {
						return Vector2(x, y);
					}
				}

				if (matchesValueX == false)
				{
					break;
				}
			}
		}
	}
	return Vector2(-1, -1);
}


/* OLD
Vector2 SearchDinoOnTheWholeScreen()
{
	cout << "Press enter to continue" << endl;
	cin.get();
	SetProcessDPIAware();

	int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	HWND hDesktopWnd = GetDesktopWindow();//to let other functions now what window they are working with
	HDC hDesktopDC = GetDC(hDesktopWnd);//NULL = get dc that covers entire screen
	HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);// creates a memory DC compatible with the application's current screen
	HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC, nScreenWidth, nScreenHeight);
	SelectObject(hCaptureDC, hCaptureBitmap);//replace object

	BitBlt(hCaptureDC, 0, 0, nScreenWidth, nScreenHeight, hDesktopDC, 0, 0, SRCCOPY | CAPTUREBLT);//performs a bit-block transfer of the color data corresponding to a rectangle of pixels from the specified source device context into a destination device context
																								  //desitination, x-cor dest rect ,y-cor  dest rect ,width, height, a handle to the source device, x-cor src rect ,y-cor src rect ,define how color data for src rect is combined with color data dest rect (to achieve final color) 

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = nScreenWidth;
	bmi.bmiHeader.biHeight = nScreenHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	RGBQUAD *pPixels = new RGBQUAD[nScreenWidth * nScreenHeight];

	GetDIBits(
		hCaptureDC,
		hCaptureBitmap,
		0,
		nScreenHeight,
		pPixels,
		&bmi,
		DIB_RGB_COLORS
	);


	int dinoImageWidth = 0;
	vector<RGBQUAD> dinoImage = ReadDinoPixels( dinoImageWidth);
	int dinoImageHeight = dinoImage.size() / dinoImageWidth;

	Vector2 antwoord = TryToFindDino(pPixels, nScreenWidth, nScreenHeight, dinoImage, dinoImageWidth, dinoImageHeight);
	cout << "Found at " << antwoord.x << "  " << antwoord.y << endl;


	delete[] pPixels;

	ReleaseDC(hDesktopWnd, hDesktopDC);
	DeleteDC(hCaptureDC);
	DeleteObject(hCaptureBitmap);
}*/



Vector2 SearchDinoOnTheWholeScreen()
{
	//cout << "Press enter to continue" << endl;
	//cin.get();

	int dinoImageWidth = 0;
	vector<RGBQUAD> dinoImage = ReadDinoPixels(dinoImageWidth);
	int dinoImageHeight = dinoImage.size() / dinoImageWidth;

	int screenWidth = 0;
	int screenHeight = 0;
	GetScreenSize(screenWidth, screenHeight);
	vector<RGBQUAD> pPixels =GetScreenPixels(screenWidth,screenHeight,0,0);

	Vector2 antwoord = TryToFindDino(pPixels, screenWidth, screenHeight, dinoImage, dinoImageWidth, dinoImageHeight);
	cout << antwoord.x <<"," <<antwoord.y << endl;
	return antwoord;
}


void StartFindStuff()
{

	Vector2 dinoPos = SearchDinoOnTheWholeScreen();

	int startPos = 0;
	int endPos = 0;
	SearchXSizeWindowInWholeScreen(startPos, endPos, dinoPos);
	cout << startPos << "  " << endPos << endl;
	cout << "rect " << startPos << " " << endPos - startPos << "  " << DINO_TO_WINDOW_HEIGHT << "  " << (int)(dinoPos.y + floorPixelsTowardstDino - DINO_TO_WINDOW_HEIGHT) << endl;

	DrawRectAt({ startPos,(int)(dinoPos.y + floorPixelsTowardstDino - DINO_TO_WINDOW_HEIGHT),endPos,(int)(dinoPos.y + floorPixelsTowardstDino) });
}


int main()
{
	cout << "Press enter to continue" << endl;
	cin.get();
	

	//DrawRectAt({0, 0, 500, 500 });
	//GetScreenPixels();
	//EvenVoorPixelsDoorTegeven();
	WriteDinoToFileAt(980, 1034,220, 263);// int minX, int maxX, int minY, int maxY

	StartFindStuff();

	std::cout << "End";
	std::cin.get();
	return 0;
}






