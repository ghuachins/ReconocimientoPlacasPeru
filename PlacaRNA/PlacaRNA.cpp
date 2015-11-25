// PlacaRNA.cpp: archivo de proyecto principal.

#include "stdafx.h"
#include <iostream>
#include <vector>
#include "opencv2/opencv.hpp"
#include "RegionPlaca.h"
#include "OCR.h"

using namespace System;

string getFilename(string s) {

	char sep = '/';
	char sepExt = '.';

#ifdef _WIN32
	sep = '\\';
#endif

	size_t i = s.rfind(sep, s.length());
	if (i != string::npos) {
		string fn = (s.substr(i + 1, s.length() - i));
		size_t j = fn.rfind(sepExt, fn.length());
		if (i != string::npos) {
			return fn.substr(0, j);
		}
		else{
			return fn;
		}
	}
	else{
		return "";
	}
}


int main(int argc, char** argv)
{
	//Mat input = imread("tmp/4.jpg", CV_LOAD_IMAGE_UNCHANGED); //read the image data in the file "MyPic.JPG" and store it in 'img'

	cout << "PROYECTO DE REDES NEURONALES";
	string filename = "tmp/3.jpg";
	Mat input_image;

	//Check if user specify image to process
	if (argc >= 2)
	{
		//filename = argv[1];
		//load image  in gray level
		input_image = imread(filename, 1);
		cout << "Trabajando con el archivo: " << filename << "\n";


	}
	else{
		printf("Argumento:\n\t%s imagen\n", argv[0]);
		system("pause");
		return 0;
	}

	string filename_whithoutExt = getFilename(filename);
	cout << "Trabajando con el archivo: " << filename_whithoutExt << "\n";

	//Detectar las posibles regiones de placa
	RegionPlaca regionPlacas;
	regionPlacas.obtNombreArchivo(filename_whithoutExt);
	regionPlacas.saveRegions = false;
	regionPlacas.showSteps = false;
	vector<Placa> posible_regions = regionPlacas.run(input_image);   // la rectangulos son cortados en un tamaño 144x33 pixeles

	//SVM for each plate region to get valid car plates
	//Read file storage.
	FileStorage fs;
	fs.open("SVM.xml", FileStorage::READ);
	Mat SVM_TrainingData;
	Mat SVM_Classes;
	fs["TrainingData"] >> SVM_TrainingData;
	fs["classes"] >> SVM_Classes;
	//Set SVM params
	CvSVMParams SVM_params;
	SVM_params.svm_type = CvSVM::C_SVC;
	SVM_params.kernel_type = CvSVM::LINEAR; //CvSVM::LINEAR;
	SVM_params.degree = 0;
	SVM_params.gamma = 1;
	SVM_params.coef0 = 0;
	SVM_params.C = 1;
	SVM_params.nu = 0;
	SVM_params.p = 0;
	SVM_params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 1000, 0.01);
	//Train SVM
	CvSVM svmClassifier(SVM_TrainingData, SVM_Classes, Mat(), Mat(), SVM_params); //constructor de svm

	//For each possible plate, classify with svm if it's a plate or no
	vector<Placa> plates;
	std::string cadena = "";
	for (int i = 0; i< posible_regions.size(); i++)
	{


		cadena = static_cast<std::ostringstream*>(&(std::ostringstream() << i))->str();

		Mat img = posible_regions[i].placaImg;
		Mat p = img.reshape(1, 1); //  transforma la imagen a una imagen con 1 canal y 1 fila
		p.convertTo(p, CV_32FC1);

		int response = (int)svmClassifier.predict(p);
		if (response == 1)
			plates.push_back(posible_regions[i]);
		imshow("PLATES" + cadena, posible_regions[i].placaImg);
	}

	cout << "Numero de placas detectadas: " << plates.size() << "\n";
	//For each plate detected, recognize it with OCR
	OCR ocr("OCR.xml");
	ocr.saveSegments = true;
	ocr.DEBUG = false;
	ocr.nombreArchivo = filename_whithoutExt;
	for (int i = 0; i< plates.size(); i++){
		Placa plate = plates[i];

		string plateNumber = ocr.run(&plate);

		string licensePlate = plate.numPlaca();
		cout << "================================================\n";
		cout << "Numero de Placa: " << licensePlate << "\n";
		cout << "================================================\n";
		rectangle(input_image, plate.posicion, Scalar(0, 0, 200));
		putText(input_image, licensePlate, Point(plate.posicion.x, plate.posicion.y), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(200, 0, 0), 2);
		if (false){
			imshow("Placa de segmento Detectado", plate.placaImg);
			cvWaitKey(0);
		}

	}
	
	
	imshow("Placa detectada", input_image);


	for (;;)
	{
		int c;
		c = cvWaitKey(10);
		if ((char)c == 27)
			break;
	}

	return 0;
}
