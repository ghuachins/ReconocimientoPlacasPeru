
#ifndef RegionPlaca_h
#define RegionPlaca_h

#include <string.h>
#include <vector>

#include "stdafx.h"
#include "Placa.h"

#include "opencv2/opencv.hpp"
#include <time.h> 

using namespace std;
using namespace cv;

class RegionPlaca{
public:
	RegionPlaca();
	string nombreArchivo;
	void obtNombreArchivo(string f);
	bool saveRegions;
	bool showSteps;
	vector<Placa> run(Mat input);
private:
	vector<Placa> segmento(Mat input);
	bool verificarTamaño(RotatedRect angRect, bool option);
	Mat c1Bgr(Mat in);
};

#endif