#include "stdafx.h"
#include "OCR.h"

const char OCR::Caracteres[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'H', 'I', 'K', 'L', 'N', 'O', 'P', 'Q', 'U', 'V' };
const int OCR::numCaracteres = 26; //26*

SegmentoLetra::SegmentoLetra(){}
SegmentoLetra::SegmentoLetra(Mat i, Rect p){
	imagen = i;
	posicion = p;
}

OCR::OCR(){
	DEBUG = false;
	trained = false;
	saveSegments = false;
	tamañoCaracter = 20;
}

OCR::OCR(string trainFile){
	DEBUG = false;
	trained = false;
	saveSegments = false;
	tamañoCaracter = 20;

	//Leer el archivo almacenado OXR.xml.
	FileStorage fs;
	fs.open("OCR.xml", FileStorage::READ);
	Mat DataEntrenamiento;
	Mat Clases;
	fs["TrainingDataF15"] >> DataEntrenamiento;
	fs["classes"] >> Clases;

	entrenamiento(DataEntrenamiento, Clases, 10);
}


Mat OCR::preprocesamientoCaracter(Mat in){
	//Redimensionar la imagen
	int h = in.rows;
	int w = in.cols;
	Mat transformMat = Mat::eye(2, 3, CV_32F);
	int m = max(w, h);
	transformMat.at<float>(0, 2) = m / 2 - w / 2;
	transformMat.at<float>(1, 2) = m / 2 - h / 2;

	Mat warpImage(m, m, in.type());
	warpAffine(in, warpImage, transformMat, warpImage.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(0));

	Mat out;
	resize(warpImage, out, Size(tamañoCaracter, tamañoCaracter));

	return out;
}

bool OCR::verificarTamaño(Mat r){
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

vector<SegmentoLetra> OCR::segmento(Placa plate){
	Mat input = plate.placaImg;
	vector<SegmentoLetra> output;
	//Threshold input image
	Mat img_threshold;
	threshold(input, img_threshold, 60, 255, CV_THRESH_BINARY_INV);
	if (DEBUG)
		imshow("Threshold plate", img_threshold);
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

	//Start to iterate to each contour founded
	vector<vector<Point> >::iterator itc = contours.begin();

	//Remove patch that are no inside limits of aspect ratio and area.    
	while (itc != contours.end()) {

		//Create bounding rect of object
		Rect mr = boundingRect(Mat(*itc));
		rectangle(result, mr, Scalar(0, 255, 0));
		//Crop image
		Mat auxRoi(img_threshold, mr);
		if (verificarTamaño(auxRoi)){
			auxRoi = preprocesamientoCaracter(auxRoi);
			output.push_back(SegmentoLetra(auxRoi, mr));
			rectangle(result, mr, Scalar(0, 125, 255));
		}
		++itc;
	}
	if (DEBUG)
		cout << "Num chars: " << output.size() << "\n";



	if (DEBUG)
		imshow("SEgmented Chars", result);
	return output;
}

Mat OCR::normalizarImagen(Mat img, int t)
{

	int sz = (t) ? img.rows : img.cols;  // si T en verdadero asigna img.rows sino asigna img.cols  (Vertical= 0 || horizontal =1)

	Mat mhist = Mat::zeros(1, sz, CV_32F); //Array de ceros de 1 fila y 'sz' columnas|filas de tipo float 32 bits

	for (int j = 0; j<sz; j++){
		Mat data = (t) ? img.row(j) : img.col(j); // Agarra la columna o fila [j]
		mhist.at<float>(j) = countNonZero(data); // cuenta cuantos numeros diferentes de cero hay en la columna|fila y lo almacena en la dimension j
	}

	//Normalize histogram
	double min, max;
	minMaxLoc(mhist, &min, &max); // Halla el min de las cantidades y el max de las cantidades

	if (max>0)
		mhist.convertTo(mhist, -1, 1.0f / max, 0);// normaliza los valores dentro del rango de min - max 

	return mhist;

}

Mat OCR::obtHistograma(Mat *hist, int type)
{

	int size = 100;
	Mat imHist;


	if (type == HORIZONTAL){
		imHist.create(Size(size, hist->cols), CV_8UC3);
	}
	else{
		imHist.create(Size(hist->cols, size), CV_8UC3);
	}

	imHist = Scalar(55, 55, 55);

	for (int i = 0; i<hist->cols; i++){
		float value = hist->at<float>(i);
		int maxval = (int)(value*size);

		Point pt1;
		Point pt2, pt3, pt4;

		if (type == HORIZONTAL){
			pt1.x = pt3.x = 0;
			pt2.x = pt4.x = maxval;
			pt1.y = pt2.y = i;
			pt3.y = pt4.y = i + 1;

			line(imHist, pt1, pt2, CV_RGB(220, 220, 220), 1, 8, 0);
			line(imHist, pt3, pt4, CV_RGB(34, 34, 34), 1, 8, 0);

			pt3.y = pt4.y = i + 2;
			line(imHist, pt3, pt4, CV_RGB(44, 44, 44), 1, 8, 0);
			pt3.y = pt4.y = i + 3;
			line(imHist, pt3, pt4, CV_RGB(50, 50, 50), 1, 8, 0);
		}
		else{

			pt1.x = pt2.x = i;
			pt3.x = pt4.x = i + 1;
			pt1.y = pt3.y = 100;
			pt2.y = pt4.y = 100 - maxval;


			line(imHist, pt1, pt2, CV_RGB(220, 220, 220), 1, 8, 0);
			line(imHist, pt3, pt4, CV_RGB(34, 34, 34), 1, 8, 0);

			pt3.x = pt4.x = i + 2;
			line(imHist, pt3, pt4, CV_RGB(44, 44, 44), 1, 8, 0);
			pt3.x = pt4.x = i + 3;
			line(imHist, pt3, pt4, CV_RGB(50, 50, 50), 1, 8, 0);

		}




	}

	return imHist;
}

void OCR::dibujarCaracteristicas(Mat character, Mat hhist, Mat vhist, Mat lowData){
	Mat img(121, 121, CV_8UC3, Scalar(0, 0, 0));
	Mat ch;
	Mat ld;

	cvtColor(character, ch, CV_GRAY2RGB);

	resize(lowData, ld, Size(100, 100), 0, 0, INTER_NEAREST);
	cvtColor(ld, ld, CV_GRAY2RGB);

	Mat hh = obtHistograma(&hhist, HORIZONTAL);
	Mat hv = obtHistograma(&vhist, VERTICAL);

	Mat subImg = img(Rect(0, 101, 20, 20));
	ch.copyTo(subImg);

	subImg = img(Rect(21, 101, 100, 20));
	hh.copyTo(subImg);

	subImg = img(Rect(0, 0, 20, 100));
	hv.copyTo(subImg);

	subImg = img(Rect(21, 0, 100, 100));
	ld.copyTo(subImg);

	line(img, Point(0, 100), Point(121, 100), Scalar(0, 0, 255));
	line(img, Point(20, 0), Point(20, 121), Scalar(0, 0, 255));

	imshow("Visual Features", img);

	cvWaitKey(0);
}

Mat OCR::caracteristicas(Mat in, int sizeData){
	//Histogram features (Obtiene un array de valores normalizados de las caracteristicas(cantidad de # diferente de 0 - normalizados) de las columnas|filas)
	Mat vhist = normalizarImagen(in, VERTICAL);// vertical= 0   (ancho)
	Mat hhist = normalizarImagen(in, HORIZONTAL);// Horizontal= 1  (largo)

	//Low data feature
	Mat lowData;
	resize(in, lowData, Size(sizeData, sizeData));// Cambia el tamaño de la imagen segun el parametro SizeData

	if (false)
		dibujarCaracteristicas(in, hhist, vhist, lowData);

	//Last 10 is the number of moments components
	int numCols = vhist.cols + hhist.cols + lowData.cols*lowData.cols; // numero de columnas total

	Mat out = Mat::zeros(1, numCols, CV_32F); // crea un array de zeros con el numero de columnas calculado anteriormente
	//Asign values to feature
	int j = 0;
	for (int i = 0; i<vhist.cols; i++)
	{
		out.at<float>(j) = vhist.at<float>(i);
		j++;
	}
	for (int i = 0; i<hhist.cols; i++)
	{
		out.at<float>(j) = hhist.at<float>(i);
		j++;
	}
	for (int x = 0; x<lowData.cols; x++)
	{
		for (int y = 0; y<lowData.rows; y++){
			out.at<float>(j) = (float)lowData.at<unsigned char>(x, y);
			j++;
		}
	}
	if (DEBUG)
		cout << out << "\n===========================================\n";
	return out;  // tiene los valores verticales, horizontales del orignal y valores de la matriz de lowdata de tamaño 'sizedata'
}

void OCR::entrenamiento(Mat TrainData, Mat classes, int nlayers){

	Mat layers(1, 3, CV_32SC1);
	layers.at<int>(0) = TrainData.cols; // 65 o 140 o 265 o 440 numeros de elementos del vector de entrada
	layers.at<int>(1) = nlayers;        // numero de neuronas en la capa oculta
	layers.at<int>(2) = numCaracteres;  // Capa de salida : por mientras '1' porque se esta entrenando para que aprenda a reconocer el char '0' 

	redNeuronal.create(layers, CvANN_MLP::SIGMOID_SYM, 1, 1);
	//Prepare trainClases
	//Create a mat with n trained data by m classes
	Mat trainClasses;
	trainClasses.create(TrainData.rows, numCaracteres, CV_32FC1); // 11 x 26
	for (int i = 0; i < trainClasses.rows; i++)
	{
		for (int k = 0; k < trainClasses.cols; k++)
		{
			//If class of data i is same than a k class
			if (k == classes.at<int>(i))
				trainClasses.at<float>(i, k) = 1;
			else
				trainClasses.at<float>(i, k) = 0;
		}
	}

	Mat weights(1, TrainData.rows, CV_32FC1, Scalar::all(1));

	//Learn classifier
	redNeuronal.train(TrainData, trainClasses, weights); // algoritmo de aprendizaje RPROP
	trained = true;

}

int OCR::clasificacion(Mat f){
	int result = -1;
	Mat output(1, numCaracteres, CV_32FC1);
	redNeuronal.predict(f, output);
	Point maxLoc;
	double maxVal;
	minMaxLoc(output, 0, &maxVal, 0, &maxLoc);
	//We need know where in output is the max val, the x (cols) is the class.
	cout << maxLoc.x << endl;
	return maxLoc.x;
}


string OCR::run(Placa *input){

	//Segment chars of plate
	vector<SegmentoLetra> segmentos = segmento(*input);
	cout << "Numero de letras: "<<segmentos.size() << endl;
	for (int i = 0; i<segmentos.size(); i++){
		//Preprocess each char for all images have same sizes
		Mat ch = preprocesamientoCaracter(segmentos[i].imagen);
		if (saveSegments){
			stringstream ss(stringstream::in | stringstream::out);
			ss << "tmpChars/" << nombreArchivo << "_" << i << ".jpg";
			imwrite(ss.str(), ch);
		}
		//For each segment Extract Features
		Mat f = caracteristicas(ch, 15);
		//For each segment feature Classify
		int character = clasificacion(f);
		input->letras.push_back(Caracteres[character]);
		input->posLetras.push_back(segmentos[i].posicion);
	}
	return "-";//input->str();
}