#include "stdafx.h"
#include "Placa.h"

Placa::Placa(){ 
}  //constructor de la clase Placa

Placa::Placa(Mat img, Rect pos){
	placaImg = img;
	posicion = pos;
} //Creamos un objeto de tipo Placa con los paremetros (img,pos)

string Placa::numPlaca(){
	string resultado = "";
	//Ordenar Numeros
	vector<int> indiceOrd; // indice de cada posicion(rectangulo) de letra
	vector<int> xposicion; // posicion x de ese rectangulo
	for (int i = 0; i< posLetras.size(); i++){ // como maximo debe haber 6 posiciones de letras
		indiceOrd.push_back(i);
		xposicion.push_back(posLetras[i].x);
	}
	float min = xposicion[0];
	int minIdx = 0;
	for (int i = 0; i< xposicion.size(); i++){
		min = xposicion[i]; 
		minIdx = i;
		for (int j = i; j<xposicion.size(); j++){
			if (xposicion[j]<min){
				min = xposicion[j];
				minIdx = j;
			}
		}
		//Se actualiza el array poniendo el menor al principio , tanto los indices de orden como las posiciones x de los rectangulos
		int aux_i = indiceOrd[i];
		int aux_min = indiceOrd[minIdx];
		indiceOrd[i] = aux_min;
		indiceOrd[minIdx] = aux_i;    

		float aux_xi = xposicion[i];
		float aux_xmin = xposicion[minIdx];
		xposicion[i] = aux_xmin;
		xposicion[minIdx] = aux_xi;
	}
	for (int i = 0; i<indiceOrd.size(); i++){
		resultado = resultado + letras[indiceOrd[i]];
	}
	return resultado;
}
