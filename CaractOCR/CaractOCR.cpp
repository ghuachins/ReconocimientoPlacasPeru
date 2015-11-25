// CaractOCR.cpp: archivo de proyecto principal.

#include "stdafx.h"

#include "opencv2/opencv.hpp"

#include "OCR.h"

#include <iostream>
#include <vector>

using namespace std;
using namespace cv;
using namespace System;
                                //0  1  2  3  4  5  6  7  8  9   A  B  C  D  E  F  H  I  K  L  N  O  P  Q  U  V  
const int numArchivoCaracts[] = { 17, 7, 7, 8, 7, 4, 1, 6, 7, 8, 11, 6, 3, 6, 2, 4, 3, 1, 1, 1, 3, 1, 2, 4, 1, 2 };

int main(int argc, char** argv)
{
	cout << "Extracion de caracteristicas y elaborar el archivo OCR.xml \n";
	cout << "\n";

	char* patron;

	//Confirmamos si se ingresan los argumentos respectivos
	if (argc >= 1)
	{
		patron = argv[1]; // tmp\

	}
	else{
		cout << "Falta argumento:\n" << argv[0] << " <Patron del nombre de la carpeta que contiene los caracteres> \n";
		system("pause");
		return 0;
	}






	Mat clases;
	Mat trainingDataf5;
	Mat trainingDataf10;
	Mat trainingDataf15;
	Mat trainingDataf20;

	vector<int> etiquetasEntrenamiento;
	OCR ocr; // instanciamos un objeto de la clase OCR

	for (int i = 0; i< 26; i++) // el numCharacters = 26 => Este este for es por cada caracter a entrenar
	{
		int numFiles = numArchivoCaracts[i];  // Se asigna el numero de elementos de prueba para el caracter [i] Ejm. 17 elementos hay de CEROS
		for (int j = 0; j< numFiles; j++){ // este for es para cada elemento que haya de un caracter tipo
			cout << "Caracter " << OCR::Caracteres[i] << " Archivo: " << j << "\n";
			stringstream ss(stringstream::in | stringstream::out); //se crea un flujo de string
			ss << patron << OCR::Caracteres[i] << "/" << j << ".jpg"; // en el flujo 'ss' se añade 'path'+...
			cout << ss.str() << endl;
			Mat img = imread(ss.str(), 0); // flag '0' return a grayscale image 

			Mat f5 = ocr.caracteristicas(img, 5);     // array con 65 elementos
			Mat f10 = ocr.caracteristicas(img, 10);   // array con 140 elementos
			Mat f15 = ocr.caracteristicas(img, 15);   // array con 265 elementos
			Mat f20 = ocr.caracteristicas(img, 20);   // array con 440 elementos

			trainingDataf5.push_back(f5);
			trainingDataf10.push_back(f10);
			trainingDataf15.push_back(f15);
			trainingDataf20.push_back(f20);

			etiquetasEntrenamiento.push_back(i);
		}
	}


	trainingDataf5.convertTo(trainingDataf5, CV_32FC1);
	trainingDataf10.convertTo(trainingDataf10, CV_32FC1);
	trainingDataf15.convertTo(trainingDataf15, CV_32FC1);
	trainingDataf20.convertTo(trainingDataf20, CV_32FC1);
	Mat(etiquetasEntrenamiento).copyTo(clases);

	FileStorage fs("OCR.xml", FileStorage::WRITE);
	fs << "TrainingDataF5" << trainingDataf5;
	fs << "TrainingDataF10" << trainingDataf10;
	fs << "TrainingDataF15" << trainingDataf15;
	fs << "TrainingDataF20" << trainingDataf20;
	fs << "classes" << clases;
	fs.release();

	system("pause");
	return 0;
}