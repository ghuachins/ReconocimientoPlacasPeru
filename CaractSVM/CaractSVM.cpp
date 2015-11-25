// CaractSVM.cpp: archivo de proyecto principal.

#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include <time.h>
#include <iostream>
#include <vector>


using namespace System;
using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
	cout << "Obtencion de caracteristicas XML para el SVM \n";
	cout << "\n";

	char* nombre_Placas;   //  Placa
	char* nombre_NoPlacas;  // No placas
	int numPlacas;  // 3
	int numNoPlacas; // 2 
	int imagenAncho = 144; // pixeles
	int imagenAlto = 33;   // pixeles

	//Confirmamos si se ingresan los argumentos respectivos
	if (argc >= 5)
	{
		numPlacas = atoi(argv[1]);
		numNoPlacas = atoi(argv[2]);
		nombre_Placas = argv[3];
		nombre_NoPlacas = argv[4];

	}
	else{
		cout << "Falta datos:\n" << argv[0] << " <numero de archivos de placas> <numero de archivos de NO placas> <Nombre del archivo Placa> <Nombre del archivo NO Placas> \n";
		system("pause");
		return 0;
	}

	Mat clases;//(numPlates+numNoPlates, 1, CV_32FC1);
	Mat dataEntrenamiento;//(numPlates+numNoPlates, imageWidth*imageHeight, CV_32FC1 );

	Mat imagenesEntrenamiento;
	vector<int> etiquetasEntrenamiento; // '0' Imagenes no placas , '1' Imagenes Placas 

	for (int i = 0; i< numPlacas; i++)
	{
		std::string cadena = "";

		cadena = static_cast<std::ostringstream*>(&(std::ostringstream() << i))->str();

		stringstream ss(stringstream::in | stringstream::out);
		ss << nombre_Placas << i << ".jpg";
		//cout << ss.str() << endl;
		Mat img = imread(ss.str(), 0); // en escala de grises
		img = img.reshape(1, 1); //  transforma la imagen a una imagen con 1 canal y 1 fila
		//imshow("Reshape" + cadena, img);
		//cvWaitKey(0);

		imagenesEntrenamiento.push_back(img);
		etiquetasEntrenamiento.push_back(1);

	}


	for (int i = 0; i< numNoPlacas; i++)
	{
		stringstream ss(stringstream::in | stringstream::out);
		ss << nombre_NoPlacas << i << ".jpg";
		Mat img = imread(ss.str(), 0);
		img = img.reshape(1, 1);
		imagenesEntrenamiento.push_back(img);
		etiquetasEntrenamiento.push_back(0);
	}


	Mat(imagenesEntrenamiento).copyTo(dataEntrenamiento); //copia todas las imagenes en 'dataEntrenamiento'
	//trainingData = trainingData.reshape(1,trainingData.rows);
	dataEntrenamiento.convertTo(dataEntrenamiento, CV_32FC1); // convierte cada matriz de imagen a una matriz de 32 bits de floats y de 1 canal
	Mat(etiquetasEntrenamiento).copyTo(clases); // '0' Imagenes no placas , '1' Imagenes Placas

	FileStorage fs("SVM.xml", FileStorage::WRITE); //Creamos y escribimos un archivo XML con las caracteristicas calculadas
	fs << "TrainingData" << dataEntrenamiento;
	fs << "classes" << clases;
	fs.release();

	return 0;
}