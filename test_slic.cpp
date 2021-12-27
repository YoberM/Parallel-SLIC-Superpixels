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
    /* Load the image and convert to Lab colour space. */
    char* file_path = "dog.png";
    Image img(file_path);
    
    //cvCvtColor(image, img, CV_BGR2Lab);
    
    /* Yield the number of superpixels and weight-factors from the user. */
    int nr_superpixels = atoi(argv[1]);
    int nc = atoi(argv[2]);
    
    double step = sqrt((img.width * img.height) / (double) nr_superpixels);
    
    // cout << nr_superpixels << " " << nc << endl;
    // cout << img.channels << " " << img.width << " " << img.height << endl;

    /* Perform the SLIC superpixel algorithm. */
    Slic slic;

    slic.initialize(&img, step, nc);

    double *global_centers= new double[slic.getCenters() *5]{-1};
    for (int i = 0; i < slic.getCenters() *5; i++)
    {
        global_centers[i]=-1;
    }
    
    //slic.generate_superpixels(&img, step, nc); // por verse xd
    // inicializa todo ----
  	int my_rank;  
	int p;	
	int i;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int *local_clusters= new int[img.width*img.height]{-1};
    for (int i = 0; i < img.width*img.height; i++)
    {
        local_clusters[i] = -1;
    }
    //Leer imagen
    int dim[3];
    //Iteraciones
    for (int it = 0; it < 10; it++)
    {
        float *local_distances=new float[img.width*img.height]{FLT_MAX};
        
        for (int i = 0; i < img.width*img.height; i++)
        {
            local_distances[i]=FLT_MAX;
        }
        


        // Enviar centros
        int div = ceil(double(slic.getCenters())/double(p)) * 5 ;
        int img_tam = img.width * img.height; //RELLENAR POGDO PLS :v 
        
        double local_centers[div] = {-1};
        for (int i = 0; i < div; i++)
        {
            local_centers[i]=-1;
        }

        slic.centersToArray(global_centers);

        MPI_Scatter(global_centers,div,MPI_DOUBLE,local_centers,div, MPI_DOUBLE,0,MPI_COMM_WORLD);
        
        // A trabajal
        // el modo de hacer q xd
        

        int number_centers = (my_rank + 1 != p ) ? div/5 : (div/5 - slic.getCenters()%p);
        number_centers = div/5;
        number_centers = slic.getCenters();
        // cout << number_centers << endl;
        // cout << "div : "<<div<< "   div*p: " <<div*p << endl;
        // total centros = 408
        // 408 / 4  = 102  * 5 = 510 
        // 0 - >        
        // 1
        // 2 
        // 102 - 408%4
        //Mal p, number of centers
        calculate_superpixel(&img, step, nc , 
                                local_centers,div, 
                               local_distances, local_clusters);
        

        // a juntal 
        int i = 0;
        if(my_rank==0){
            cout <<"prooteadas\n";
        }
        while (true){
            if ( int(my_rank/(pow(2,i))) % 2 != 0 ){ // Envia
                // cout << "SOY: "<<my_rank<< " Y le envio a: " << my_rank-pow(2,i) << endl;
                MPI_Send(local_distances,img_tam,MPI_FLOAT,my_rank-pow(2,i),my_rank,MPI_COMM_WORLD); // mandar al rank - 2^i               n
                MPI_Send(local_clusters,img_tam,MPI_INT,my_rank-pow(2,i),my_rank,MPI_COMM_WORLD); // mandar al rank - 2^i               n
                
                break;// >:v
            }
            else{
                if (my_rank+pow(2,i) < p){
                    float *external_distances= new float[img.width*img.height];
                    int   *external_clusters= new int[img.width*img.height];
                    MPI_Recv(external_distances,img_tam,MPI_FLOAT,my_rank+pow(2,i),my_rank+pow(2,i),MPI_COMM_WORLD, MPI_STATUS_IGNORE); // RECIBIR DEL rank + 2^i      n -> 2n
                    MPI_Recv(external_clusters,img_tam,MPI_INT,my_rank+pow(2,i),my_rank+pow(2,i),MPI_COMM_WORLD, MPI_STATUS_IGNORE); // RECIBIR DEL rank + 2^i      n -> 2n
                    for (int i = 0; i < img_tam; i++){
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

        //Actualizar los centros
        if ( my_rank == 0){
            slic.updateClusters(local_clusters,img.width,img.height);
            slic.recalculate_centers(&img);
        }
        
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(local_clusters, img.width*img.height, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        delete[] local_distances;

    }
    if(my_rank==0){

    slic.create_connectivity(&img);
    /* Display the contours and show the result. */

    slic.display_contours(&img, Color(255,0,0));
    img.write("result.png");
    }
    MPI_Finalize();
    
    //  Enviar centros a los procesos
    //  Calcular distancia y clusters
    //  Recolectar distancias y clusters modo arbol xd
    // Nodo master calcular centros
    // Reenviar 

    // slic.create_connectivity(&img);
    // /* Display the contours and show the result. */

    // slic.display_contours(&img, Color(255,0,0));
    // img.write("result.png");

}