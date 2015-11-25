
#ifndef Placa_h
#define Placa_h

#include <string.h>
#include <vector>
#include "stdafx.h"
#include "opencv2/opencv.hpp"


using namespace std;
using namespace cv;

class Placa{
public:
	Placa();
	Placa(Mat img, Rect pos);
	string numPlaca();
	Rect posicion;
	Mat placaImg;
	vector<char> letras;
	vector<Rect> posLetras;
};

#endif