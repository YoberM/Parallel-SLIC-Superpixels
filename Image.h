
#ifndef IMAGE_H
#define IMAGE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

struct Coordinate{
    int x;
    int y;

    Coordinate(int x, int y){
        this->x = x;
        this->y = y;
    }
};

struct Color{
    int r;
    int g;
    int b;

    Color(){}

    Color(int r,int g,int b){
        this->r = r;
        this->g = g;
        this->b = b;
    }
};

class Image{
public:
    Image(){
        channels = 0;
        width = 0;
        height = 0;
        data = nullptr;
    }
    Image(char* file_path){
        load(file_path);
    }

    void load(char* file_path){
        data = stbi_load(file_path, &width, &height, &channels, 0);

        if(data == nullptr) {
            printf("Error in loading the image\n");
            exit(1);
        }
    }

    int size(){
        return channels*width*height;
    }

    void write(char* filename){
        stbi_write_png(filename,width,height,channels,data,width*channels);
    }

    void IMG_RBG_TO_LAB(){
        for (int i = 0 ; i < width ; i++ ){
            for (int j = 0 ; j < height ; j++ ){
                RGB2LAB( data + channels * (j * width + i));
            }   
        }
    }
    
    unsigned char* data;
    int channels;
    int width;
    int height;

private: 

    void RGB2LAB ( unsigned char * pixel)
    {
        double RGB[3];
        double XYZ[3];
        double Lab[3];
        double RGB2[3];
        double XYZ2[3];
        double Lab2[3];
        double adapt[3];

        double value;

        adapt[0] = 0.950467;
        adapt[1] = 1.000000;
        adapt[2] = 1.088969;

        RGB[0] = (int)(*(pixel + 0)) * 0.003922;
        RGB[1] = (int)(*(pixel + 1)) * 0.003922;
        RGB[2] = (int)(*(pixel + 2)) * 0.003922;

        XYZ[0] = 0.412424 * RGB[0] + 0.357579 * RGB[1] + 0.180464 * RGB[2];
        XYZ[1] = 0.212656 * RGB[0] + 0.715158 * RGB[1] + 0.0721856 * RGB[2];
        XYZ[2] = 0.0193324 * RGB[0] + 0.119193 * RGB[1] + 0.950444 * RGB[2];

        Lab[0] = 116 * H( XYZ[1] / adapt[1] ) - 16;
        Lab[1] = 500 * ( H( XYZ[0] / adapt[0] ) - H ( XYZ[1] / adapt[1] ) );
        Lab[2] = 200 * ( H( XYZ[1] / adapt[1] ) - H ( XYZ[2] / adapt[2] ) );

        *(pixel + 0) =  Lab[0];
        *(pixel + 1) =  Lab[1];
        *(pixel + 2) =  Lab[2];
    }

    double H(double q)
    {
        double value;
        if ( q > 0.008856 ) {
            value = pow ( q, 0.333333 );
            return value;
        }
        else {
            value = 7.787*q + 0.137931;
            return value;
        }
    }
};

Color get2D(Image* img,int row,int col){
    Color temp;
    temp.r = (int)(*((img->data) + img->channels * (row * img->width + col) + 0));
    temp.g = (int)(*((img->data) + img->channels * (row * img->width + col) + 1)) ;
    temp.b = (int)(*((img->data) + img->channels * (row * img->width + col) + 2)) ;

    return temp;

}

void set2D(Image* img,int row,int col, Color color){
    *((img->data) + img->channels * (row * img->width + col) + 0) = (char)color.r;
    *((img->data) + img->channels * (row * img->width + col) + 1) = (char)color.g;
    *((img->data) + img->channels * (row * img->width + col) + 2)  = (char)color.b;
}


#endif