/*
 * test_slic.cpp.
 *
 * Written by: Pascal Mettes.
 *
 * This file creates an over-segmentation of a provided image based on the SLIC
 * superpixel algorithm, as implemented in slic.h and slic.cpp.
 */
 
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <float.h>

#include "slic.h"
#include "slic.cpp"


#include <mpi.h>
using namespace std;


int main(int argc, char *argv[]) {
    
        
    int nr_superpixels = atoi(argv[1]);
    int nc = atoi(argv[2]);
    char*  file_path = (argv[3]);

    Image img(file_path);

    double step = sqrt((img.width * img.height) / (double) nr_superpixels);
    
    Slic slic;

    slic.initialize(&img, step, nc);

    

    double *global_centers= new double[slic.getCenters() *5]{-1};
    for (int i = 0; i < slic.getCenters() *5; i++)
    {
        global_centers[i]=-1;
    }

    slic.centersToArray(global_centers);
    
  	int my_rank;  
	int p;	
	int i;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


    if (my_rank == 0 ){
        cout << " centros " << slic.getCenters() <<  endl;
    }


    int *local_clusters= new int[img.width*img.height];
    for (int i = 0; i < img.width*img.height; i++)
    {
        local_clusters[i] = -1;
    }
    //Leer imagen
    int dim[3];



    //Iteraciones del algoritmo superpixel
    for (int it = 0; it < 10; it++)
    {
        float *local_distances=new float[img.width*img.height];

        // Enviar centros
        int div = ceil(double(slic.getCenters())/double(p)) * 5 ;
        int img_tam = img.width * img.height; //RELLENAR POGDO PLS :v 
        
        double local_centers[div] = {-1};
        for (int i = 0; i < div; i++)
        {
            local_centers[i]=-1;
        }

        // Se obtienen los ultimos valores de los centros 
        slic.centersToArray(global_centers);


        //Se distribuyen los centros a ser procesados mediante un sccater
        MPI_Scatter(global_centers,div,MPI_DOUBLE,local_centers,div, MPI_DOUBLE,0,MPI_COMM_WORLD);
        
        // A trabajal
        // el modo de hacer q xd
        

        int number_centers = (my_rank + 1 != p ) ? div/5 : (div/5 - slic.getCenters()%p);
        number_centers = div/5;
        number_centers = slic.getCenters();

        // cada proceso calcula sus distancias y clusters
        calculate_superpixel(&img, step, nc , 
                                local_centers,div, 
                               local_distances, local_clusters);
        

        // Se juntan los resultados mediante un all to one en estructura de arbol log(p)
        int i = 0;
        while (true){
            if ( int(my_rank/(pow(2,i))) % 2 != 0 ){ // Envia
                
                // nodo que envia sus resultados a los nodos en forma de arbol y espera la siguiente iteracion 
                MPI_Send(local_distances,img_tam,MPI_FLOAT,my_rank-pow(2,i),my_rank,MPI_COMM_WORLD); // mandar al rank - 2^i               n
                MPI_Send(local_clusters,img_tam,MPI_INT,my_rank-pow(2,i),my_rank,MPI_COMM_WORLD); // mandar al rank - 2^i               n
                
                break;
            }
            else{
                if (my_rank+pow(2,i) < p){
                    // nodo que recibe la informacion de otro nodo realiza sus calculos y realiza su siguiente iteracion en el arbol
                    float *external_distances= new float[img.width*img.height];
                    int   *external_clusters= new int[img.width*img.height];
                    MPI_Recv(external_distances,img_tam,MPI_FLOAT,my_rank+pow(2,i),my_rank+pow(2,i),MPI_COMM_WORLD, MPI_STATUS_IGNORE); // RECIBIR DEL rank + 2^i      n -> 2n
                    MPI_Recv(external_clusters,img_tam,MPI_INT,my_rank+pow(2,i),my_rank+pow(2,i),MPI_COMM_WORLD, MPI_STATUS_IGNORE); // RECIBIR DEL rank + 2^i      n -> 2n
                    for (int i = 0; i < img_tam; i++){
                        // se mantienen los minimos entre los dos nodos
                        if(external_distances[i]<local_distances[i]){
                            local_distances[i]=external_distances[i];
                            local_clusters[i]= external_clusters[i];
                        }
                    }
                    delete[] external_distances;
                    delete[] external_clusters;
                }else{
                    break;
                }
            } 
            i++;
        }

        
        MPI_Barrier(MPI_COMM_WORLD);

        // se realiza la operacion de calcular los nuevos centros con los clusters finales 
        if ( my_rank == 0 && it == 0){
            slic.updateClusters(local_clusters,img.width,img.height);
            slic.recalculate_centers(&img);
        }
        
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(local_clusters, img.width*img.height, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        delete[] local_distances;

    }
    delete [] local_clusters;
    MPI_Barrier(MPI_COMM_WORLD);

    // re conectan los centros finales y de guarda la iamgen resultante
    if(my_rank==0){

        slic.create_connectivity(&img);
        /* Display the contours and show the result. */

        slic.display_contours(&img, Color(255,0,0));
        img.write("result.png");
    }
    MPI_Finalize();

}