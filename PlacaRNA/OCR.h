
#ifndef OCR_h
#define OCR_h

#include <string.h>
#include <vector>

#include "stdafx.h"
#include "Placa.h"

#include "opencv2/opencv.hpp"
#include <time.h> 


using namespace std;
using namespace cv;


#define HORIZONTAL    1
#define VERTICAL    0

class SegmentoLetra{
public:
	SegmentoLetra();
	SegmentoLetra(Mat i, Rect p);
	Mat imagen;
	Rect posicion;
};

class OCR{
public:
	bool DEBUG;
	bool saveSegments;
	string nombreArchivo;
	static const int numCaracteres;
	static const char Caracteres[];
	OCR(string archivoEntrenamiento);
	OCR();
	string run(Placa *input);
	int tamañoCaracter;
	Mat preprocesamientoCaracter(Mat in);
	int clasificacion(Mat f);
	void entrenamiento(Mat dataEntrenamiento, Mat clasesEntrenamiento, int numCapas);
	Mat caracteristicas(Mat input, int tamaño);

private:
	bool trained;
	vector<SegmentoLetra> segmento(Placa input);
	Mat obtHistograma(Mat *hist, int type);
	void dibujarCaracteristicas(Mat character, Mat hhist, Mat vhist, Mat lowData);
	Mat normalizarImagen(Mat img, int t);
	bool verificarTamaño(Mat r);
	CvANN_MLP  redNeuronal;
	int K;
};

#endif
