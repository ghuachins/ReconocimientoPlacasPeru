// EntrenamientoOCR.cpp: archivo de proyecto principal.

#include "stdafx.h"


#include "opencv2/opencv.hpp"
#include "OCR.h"
#include <time.h>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

OCR ocr;
                 //  0      0        rows          vector de ejemplos '0'
void generarRandom(int n, int min, int max, vector<int> *samples){
	int range = max - min;
	int r = rand() % range + min;

	if (samples->at(r) == 0){
		samples->at(r) = 1;
		n++;
		cout << "n=" << n << endl;
	}
	if (n<122){
		generarRandom(n, min, max, samples);
	}
}

float tasaError(Mat samples, Mat classes){
	float errors = 0;
	for (int i = 0; i<samples.rows; i++){

		int result = ocr.clasificacion(samples.row(i));

		if (result != classes.at<int>(i))
			errors++;
	}
	return errors / samples.rows;
}

int main(int argc, char** argv)
{
	int nh;
	char* data;
	//Check params
	if (argc >= 2)
	{
		nh = atoi(argv[1]);
		data = argv[2];
		cout << "Nh:" << nh << "Data = " << data << endl;
	}
	else{
		cout << "Usage:\n" << argv[0] << " <num hidden nodes> <data to load from xml>\n";
		return 0;
	}

	Mat classes;
	Mat trainingData;

	//Read file storage.
	FileStorage fs;
	fs.open("OCR.xml", FileStorage::READ);

	fs[data] >> trainingData;
	fs["classes"] >> classes;

	//cout << "trainingData = " << endl << " " << trainingData << endl << endl;

	float result;

	//init random generator
	srand(time(NULL));
	//Create 10 random pos for samples
	std::vector<int> isSample(trainingData.rows, 0); // crea un array de '0' del tamaño de 'trainingData.rows'

	cout << "\ntrainingDataRows = " << trainingData.rows << endl;

	generarRandom(0, 0, trainingData.rows - 1, &isSample);

	/*cout << "isSample = [";
	for (int i = 0; i < isSample.size(); i++){
	cout <<i<<"-"<< isSample.at(i)<<"**";
	}
	cout << "] " << endl;*/



	//Create sample data
	Mat train, trainClasses;
	Mat samples, samplesClasses;
	for (int i = 0; i<trainingData.rows; i++){
		if (isSample[i] == 0){
			samples.push_back(trainingData.row(i));
			samplesClasses.push_back(classes.row(i));
		}
		else{
			train.push_back(trainingData.row(i)); // va almacenar 11 filas de datos de 65 columna (11x65)
			trainClasses.push_back(classes.row(i));//  11 filas 

		}
	}
	//cout << "trainingData = " << endl << " " << train << endl << endl;

	result = 0;

	ocr.entrenamiento(train, trainClasses, nh);

	result = tasaError(samples, samplesClasses);

	cout << nh << "\t" << data << "\t" << result << "\n";
	cout << result;

	system("pause");

	return 0;
}