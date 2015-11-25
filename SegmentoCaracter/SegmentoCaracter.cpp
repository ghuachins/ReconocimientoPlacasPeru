// SegmentoCaracter.cpp: archivo de proyecto principal.

#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <time.h> 

using namespace cv;
using namespace std;

using namespace System;

Mat preprocessChar(Mat in);
bool verifySizes(Mat r);

int main(int argc, const char** argv)
{
	Mat input = imread("tmp/plate21.jpg", CV_LOAD_IMAGE_UNCHANGED); //Leemos la imagen en el archivo "tmp/plate21.jpg" y guardamos en 'input'

	if (input.empty()) //check whether the image is loaded or not
	{
		cout << "Error : Imagen no puede ser cargada..!!" << endl;
		system("pause"); //wait for a key press
		return -1;
	}

	//Para diferenciar los píxeles que nos interesa del resto (que con el tiempo se rechazó), realizamos una comparación de cada valor de intensidad de los píxeles con respecto a un umbral (determinado de acuerdo con el problema a resolver).
	//podemos establecer con un valor determinado para identificarlos(es decir, podemos asignarles un valor de 0 (negro), 255 (blanco)o cualquier valor que se adapte a sus necesidades).

	//Threshold imagen
	Mat img_threshold;
	threshold(input, img_threshold, 60, 255, CV_THRESH_BINARY_INV);
	if (true)
		imshow("Threshold placa", img_threshold);
	Mat img_contours;
	img_threshold.copyTo(img_contours);
	//Find contours of possibles characters
	vector< vector< Point> > contours;
	findContours(img_contours,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_NONE); // all pixels of each contours

	// Draw blue contours on a white image
	cv::Mat result;
	img_threshold.copyTo(result);
	cvtColor(result, result, CV_GRAY2RGB);
	cv::drawContours(result, contours,
		-1, // draw all contours
		cv::Scalar(255, 0, 0), // in blue
		1); // with a thickness of 1

	if (true)
		imshow("ContornosAzules", result);


	//Start to iterate to each contour founded
	vector<vector<Point> >::iterator itc = contours.begin();
	int i = 0;
	Mat Charaux;
	//Remove patch that are no inside limits of aspect ratio and area.    
	while (itc != contours.end()) {

		//Create bounding rect of object
		Rect mr = boundingRect(Mat(*itc));
		rectangle(result, mr, Scalar(0, 255, 0));
		//Crop image
		Mat auxRoi(img_threshold, mr);


		if (verifySizes(auxRoi)){
			auxRoi = preprocessChar(auxRoi);
			cout << "Corte" << endl;

			cout << "columnas = " << auxRoi.cols << "Filas = " << auxRoi.rows << endl;
			stringstream ss(stringstream::in | stringstream::out);
			ss << "char/" << "_" << i << ".jpg";

			vector<int> compression_params;
			compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
			compression_params.push_back(9);

			bool success = imwrite("C:/Users/gian/Documents/Visual Studio 2013/Projects/PlacaRNA/SegmentoCaracter/" + ss.str(), auxRoi, compression_params);
			if (success)
				cout << ss.str() << endl;

			rectangle(result, mr, Scalar(0, 125, 255));
			cout << "*************************" << endl;

			Charaux = auxRoi;
		}

		++itc;
		++i;
	}

	if (true)
		imshow("Character", Charaux);
	if (0)
		imshow("COLUMNA 0", Charaux.col(0));

	if (true)
		imshow("SEgmented Chars", result);

	waitKey(0); //wait infinite time for a keypress

	destroyWindow("SEgmented Chars"); //destroy the window with the name, "MyWindow"


}



Mat preprocessChar(Mat in){
	//Remap image
	int h = in.rows;
	int w = in.cols;
	Mat transformMat = Mat::eye(2, 3, CV_32F);
	int m = max(w, h);
	transformMat.at<float>(0, 2) = m / 2 - w / 2;
	transformMat.at<float>(1, 2) = m / 2 - h / 2;

	Mat warpImage(m, m, in.type());
	warpAffine(in, warpImage, transformMat, warpImage.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(0));

	Mat out;
	resize(warpImage, out, Size(20, 20));

	return out;
}

bool verifySizes(Mat r){
	//Char sizes 45x77
	float aspect = 40.0f / 80.0f;
	float charAspect = (float)r.cols / (float)r.rows;
	float error = 0.35;
	float minHeight = 10;
	float maxHeight = 80;
	//We have a different aspect ratio for number 1, and it can be ~0.2
	float minAspect = 0.2;
	float maxAspect = 1.1;
	//area of pixels
	float area = countNonZero(r);
	//bb area
	float bbArea = r.cols*r.rows;
	//% of pixel in area
	float percPixels = area / bbArea;

	if (false)
		cout << "Aspect: " << aspect << " [" << minAspect << "," << maxAspect << "] " << "Area " << percPixels << " Char aspect " << charAspect << " Height char " << r.rows << "\n";
	if (percPixels < 1.5 && charAspect > minAspect && charAspect < maxAspect && r.rows >= minHeight && r.rows < maxHeight)
		return true;
	else
		return false;

}
