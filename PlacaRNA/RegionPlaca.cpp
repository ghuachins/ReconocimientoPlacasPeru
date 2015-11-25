
#include "stdafx.h"
#include "RegionPlaca.h"

void RegionPlaca::obtNombreArchivo(string s) {
	nombreArchivo = s;
}

RegionPlaca::RegionPlaca(){
	showSteps = false;
	saveRegions = false;
}

bool RegionPlaca::verificarTamaño(RotatedRect angRect, bool option){

	float error = 0.9;
	// Tamaño de placa Peruana : 300x150cm  , entonces tiene una razon de 2/1
	float razon = 2;
	// Definir un min y un max de area
	int min = 50 * razon * 50; // minimum area   = 5000
	int max = 110 * razon * 110; // maximum area  = 24,200
	//Definir los minimos y maximos que puede tomar la razon.
	float rmin = razon - razon*error; // res = 1.2
	float rmax = razon + razon*error;  // res = 2.8

	int area = angRect.size.height * angRect.size.width;

	if (option)
		cout << area << " ";
	//calculamos la razon (ancho/altura)
	float r = (float)angRect.size.width / (float)angRect.size.height;
	
	if (r<1)
		r = (float)angRect.size.height / (float)angRect.size.width;

	if (option)
		cout << (float)angRect.size.width << " - " << (float)angRect.size.height << " r= " << r << endl;

	//Si el area del rectangulo del segmento detectado se encuentra entre los limites.
	if ((area < min || area > max) || (r < rmin || r > rmax)){
		return false;
	}
	else{
		return true;
	}

}

Mat RegionPlaca::c1Bgr(Mat in) // Esta funcion convierte una imagen de 3 canales a 1 canal
{ 
	Mat out(in.size(), in.type()); //Se crea una imagen del mismo tamaño y mismo tipo de la imagen de entrada
	if (in.channels() == 3){ 
		Mat hsv;
		vector<Mat> hsvSplit;
		cvtColor(in, hsv, CV_BGR2HSV);
		split(hsv, hsvSplit);
		equalizeHist(hsvSplit[2], hsvSplit[2]);
		merge(hsvSplit, hsv);
		cvtColor(hsv, out, CV_HSV2BGR);
	}
	else if (in.channels() == 1){
		equalizeHist(in, out);
	}
	return out;

}

vector<Placa> RegionPlaca::segmento(Mat input){
	vector<Placa> output;// creamos un array de clases de tipo Placa

	//Transformamos la imagen a escala de grises
	Mat img_gray;
	cvtColor(input, img_gray, CV_BGR2GRAY);
	blur(img_gray, img_gray, Size(5, 5));

	//Para encontrar las lineas verticales de la placa, se debe resaltar las lineas
	Mat img_sobel;
	Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	if (showSteps)
		imshow("Sobel", img_sobel);

	//Se binariza la imagen
	Mat img_threshold;
	threshold(img_sobel, img_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
	if (showSteps)
		imshow("Threshold", img_threshold);

	//Se hace un barrido de las lineas verticales y horizontales
	Mat element = getStructuringElement(MORPH_RECT, Size(20, 5));
	morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element);
	if (showSteps)
		imshow("Close", img_threshold);

	//Encontramos todas las posibles regiones de placas de autos
	vector< vector< Point> > regiones;
	findContours(img_threshold,
		regiones, // Array de regiones
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_NONE); // all pixels of each contours

	//Empezamos a analizar las regiones una por una
	vector<vector<Point> >::iterator itc = regiones.begin();
	vector<RotatedRect> rects; // Rectangulos dentro de los limites indicados


	//Eliminamos las regiones que no se encuentran dentro de los limites permitidos.    
	while (itc != regiones.end()) {
		//Create bounding rect of object
		RotatedRect angRect = minAreaRect(Mat(*itc));
		if (!verificarTamaño(angRect, false)){
			itc = regiones.erase(itc);
		}
		else{
			++itc;
			rects.push_back(angRect);
		}
	}

	// Pintamos de color azul las regiones que se encuentran dentro de los limites
	cv::Mat resultado;
	input.copyTo(resultado); // copiamos la imagen de entrada en la variable 'resultado'
	cv::drawContours(resultado, regiones,
		-1, // pintamos todos los contornos
		cv::Scalar(255, 0, 0), // asignamos que color
		1, // Pintamos los contornos que encierran otros contornos
		16);// Grosor de las lineas

	if (showSteps)
		imshow("ContornosAzul", resultado);

	
	for (int i = 0; i< rects.size(); i++){
		std::string cadena = "";

		cadena = static_cast<std::ostringstream*>(&(std::ostringstream() << i))->str();
		//For better rect cropping for each posible box
		//Make floodfill algorithm because the plate has white background
		//And then we can retrieve more clearly the contour box
		circle(resultado, rects[i].center, 3, Scalar(0, 255, 0), -1);
		//get the min size between width and height
		float minSize = (rects[i].size.width < rects[i].size.height) ? rects[i].size.width : rects[i].size.height;
		minSize = minSize - minSize*0.5;
		//initialize rand and get 5 points around center for floodfill algorithm
		srand(time(NULL));
		//Initialize floodfill parameters and variables
		Mat mask;
		mask.create(input.rows + 2, input.cols + 2, CV_8UC1);
		mask = Scalar::all(0);
		int loDiff = 30;
		int upDiff = 30;
		int connectivity = 4;
		int newMaskVal = 255;
		int NumSeeds = 10;
		Rect ccomp;
		int flags = connectivity + (newMaskVal << 8) + CV_FLOODFILL_FIXED_RANGE + CV_FLOODFILL_MASK_ONLY;
		for (int j = 0; j<NumSeeds; j++){
			Point seed;
			seed.x = rects[i].center.x + rand() % (int)minSize - (minSize / 2);
			seed.y = rects[i].center.y + rand() % (int)minSize - (minSize / 2);
			circle(resultado, seed, 1, Scalar(0, 255, 255), -1);
			int area = floodFill(input, mask, seed, Scalar(255, 0, 0), &ccomp, Scalar(loDiff, loDiff, loDiff), Scalar(upDiff, upDiff, upDiff), flags);
		}
		if (showSteps)
			imshow("MASK" + cadena, mask);
		//cvWaitKey(0);

		//Check new floodfill mask match for a correct patch.
		//Get all points detected for get Minimal rotated Rect
		vector<Point> pointsInterest;
		Mat_<uchar>::iterator itMask = mask.begin<uchar>();
		Mat_<uchar>::iterator end = mask.end<uchar>();
		for (; itMask != end; ++itMask)
			if (*itMask == 255)
				pointsInterest.push_back(itMask.pos());

		RotatedRect minRect = minAreaRect(pointsInterest);

		//imshow("Rotated minRECT" + cadena, result);

		if (verificarTamaño (minRect, true)){
			// rotated rectangle drawing 
			Point2f rect_points[4]; minRect.points(rect_points);
			for (int j = 0; j < 4; j++)
				line(resultado, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 0, 255), 1, 8);

			//imshow("Rotated mminRECT SEGUNDO" + cadena, result);
			//Get rotation matrix
			float r = (float)minRect.size.width / (float)minRect.size.height;
			float angle = minRect.angle;
			if (r<1)
				angle = 90 + angle;
			Mat rotmat = getRotationMatrix2D(minRect.center, angle, 1);

			//Create and rotate image
			Mat img_rotated;
			warpAffine(input, img_rotated, rotmat, input.size(), CV_INTER_CUBIC);

			//Se corta la imagen de la placa identificada 
			Size rect_size = minRect.size;
			if (r < 1)
				swap(rect_size.width, rect_size.height);
			Mat img_crop;
			getRectSubPix(img_rotated, rect_size, minRect.center, img_crop);
			if (showSteps)
				imshow("imgCrop" + cadena, img_crop);

			Mat resultResized;
			resultResized.create(33, 144, CV_8UC3);
			resize(img_crop, resultResized, resultResized.size(), 0, 0, INTER_CUBIC);
			//Se convierte a escala de grises la imagen cortada
			Mat grayResult;
			cvtColor(resultResized, grayResult, CV_BGR2GRAY);
			blur(grayResult, grayResult, Size(3, 3));
			grayResult = c1Bgr(grayResult);
			if (showSteps){
				stringstream ss(stringstream::in | stringstream::out);
				ss << "tmp/" << nombreArchivo << "_" << i << ".jpg";

				vector<int> compression_params;
				compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
				compression_params.push_back(9);

				bool success = imwrite("C:/Users/gian/Documents/Visual Studio 2013/Projects/PlacaRNA/CaractSVM/" + ss.str(), grayResult, compression_params);
				if (success)
					cout << ss.str() << endl;
			}
			output.push_back(Placa(grayResult, minRect.boundingRect()));
		}
	}
	if (showSteps)
		imshow("Contours", resultado);

	return output;
}

vector<Placa> RegionPlaca::run(Mat input){

	//Segment image by white 
	vector<Placa> tmp = segmento(input);

	//return detected and posibles regions
	return tmp;
}
