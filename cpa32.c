#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "helpers.h"


#define SAMPLES 200
#define WAVELENGTH 20000
#define KEYBYTES 16
#define KEYS 256

float corelation[KEYS][KEYBYTES];

float maximum(float *array,int size){
	float max=array[0];
	int j;
	for (j=1;j<size;j++){
		if (array[j]>max){
			max=array[j];
		}  
	 }
	 return max;
}

float hamming(unsigned int M, unsigned int R){
	unsigned int H=M^R;
	// Count the number of set bits
	int dist=0;
	while(H){
		dist++; 
		H &= H - 1;
	}
	return (float)dist;
}



float maxCorelation(float **wavedata, unsigned int **cipher, int keyguess, int keybyte){
	  float hammingArray[SAMPLES];
	  int i;
	  for(i=0;i<SAMPLES;i++){
		  hammingArray[i]=hamming(cipher[i][keybyte],keyguess);
		  //printf("haming[%d]=%f\n",i,hammingArray[i]);
	  }
	  
	  float sigmaWH=0,sigmaW=0,sigmaH=0,sigmaW2=0,sigmaH2=0;
	  for(i=0;i<SAMPLES;i++){
		  sigmaH+=hammingArray[i];
		  sigmaH2+=hammingArray[i]*hammingArray[i];
	  }
	  //printf("sgmaH : %f , sigmaH2 : %f\n",sigmaH, sigmaH2);
	  
	  float corelations[WAVELENGTH];
	  int j;
	  for(j=0;j<WAVELENGTH;j++){
		sigmaW=0;sigmaW2=0;sigmaWH=0;	
		for(i=0;i<SAMPLES;i++){
			sigmaW+=wavedata[i][j];
			sigmaW2+=wavedata[i][j]*wavedata[i][j];
			sigmaWH+=wavedata[i][j]*hammingArray[i];
		}
		//printf("sgmaW : %f , sigmaW2 : %f, sigmaWH : %f\n",sigmaW, sigmaW2,sigmaWH);
		float numerator=SAMPLES*sigmaWH - sigmaW*sigmaH;
		float denominator=sqrt(SAMPLES*sigmaW2 - sigmaW*sigmaW)*sqrt(SAMPLES*sigmaH2 - sigmaH*sigmaH);
		corelations[j]=numerator/denominator;
		//printf("numerator : %f  den : %f, corelation[%d] :  %f\n",numerator,denominator,j,corelations[j]);
	  }
	  
	  float max=maximum(corelations,WAVELENGTH);
	  return max;
}

int main(int argc, char *argv[]){
	
	int i,j;
		
	//check args
	if(argc!=3){
		fprintf(stderr,"%s\n", "Not enough args. eg ./cpa wavedata.txt cipher.txt");
		exit(EXIT_FAILURE);
	}
	
	//get wave data
	float **wavedata=malloc(sizeof(float*) * SAMPLES);
	checkMalloc(wavedata);

	for (i=0; i<SAMPLES; i++){
		wavedata[i]=malloc(sizeof(float) * WAVELENGTH);
		checkMalloc(wavedata[i]);
	}
		
	FILE *file=openFile(argv[1],"r");
	for(i=0; i<SAMPLES ;i++){
		for(j=0; j<WAVELENGTH; j++){
			float dat;
			//fscanf(file,"%d",&dat);
			fread((void*)(&dat),sizeof(dat),1,file);
			wavedata[i][j]=dat/(float)256;
		}
	}
	
	//get cipher texts
	unsigned int **cipher=malloc(sizeof(unsigned int*)*SAMPLES);
	checkMalloc(cipher);

	for (i=0; i<SAMPLES; i++){
		cipher[i]=malloc(sizeof(unsigned int)*KEYBYTES);
		checkMalloc(cipher[i]);
	}
		
	file=openFile(argv[2],"r");
	for(i=0; i<SAMPLES ;i++){
		for(j=0; j<KEYBYTES; j++){
			fscanf(file,"%x",&cipher[i][j]);
		}
	}
	
	//printf("Corelation for 0,0 : %f\n",maxCorelation(wavedata, cipher, 0, 0));
	
	for (i=0;i<KEYS;i++){
		for(j=0;j<KEYBYTES;j++){
			corelation[i][j]=maxCorelation(wavedata, cipher, i, j);
			//printf("%f\t",corelation[i][j]);
		}
		//printf("\n");
		//fprintf(stderr,"Done for key %d\n",i);
	}
	
	int resultkey[KEYBYTES];
	float resultco[KEYBYTES];
	
	for (j=0;j<KEYBYTES;j++){
		int key=0;
		float value=corelation[0][j];
		for (i=1;i<KEYS;i++){
			if (corelation[i][j]>value){
				value=corelation[i][j];
				key=i;
			}  
		 }
		 resultkey[j]=key;
		 resultco[j]=value;
	}
	 
	 for (j=0;j<KEYBYTES;j++){
		 printf("%X\t",resultkey[j]);
	 }
	 printf("\n");
	 for (j=0;j<KEYBYTES;j++){
		 printf("%f\t",resultco[j]);
	 } 
	 
	
	return 0;
}

