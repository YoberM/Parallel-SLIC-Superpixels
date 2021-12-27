#include "slic.h"

/*
 * Constructor. Nothing is done here.
 */
Slic::Slic() {

}

/*
 * Destructor. Clear any present data.
 */
Slic::~Slic() {
    clear_data();
}

/*
 * Clear the data as saved by the algorithm.
 *
 * Input : -
 * Output: -
 */
void Slic::clear_data() {
    clusters.clear();
    distances.clear();
    centers.clear();
    center_counts.clear();
}

/*
 * Initialize the cluster centers and initial values of the pixel-wise cluster
 * assignment and distance values.
 *
 * Input : The image (IplImage*).
 * Output: -
 */
void Slic::init_data(Image *image) {
    /* Initialize the cluster and distance matrices. */
    for (int i = 0; i < image->width; i++) { 
        vector<int> cr;
        vector<double> dr;
        for (int j = 0; j < image->height; j++) {
            cr.push_back(-1);
            dr.push_back(FLT_MAX);
        }
        clusters.push_back(cr);
        distances.push_back(dr);
    }
    
    /* Initialize the centers and counters. */
    for (int i = step; i < image->width - step/2; i += step) {
        for (int j = step; j < image->height - step/2; j += step) {
            vector<double> center;
            /* Find the local minimum (gradient-wise). */
            Coordinate nc = find_local_minimum(image, Coordinate(i,j));
            Color colour = get2D(image, nc.y, nc.x);
            
            /* Generate the center vector. */
            center.push_back(colour.r);
            center.push_back(colour.g);
            center.push_back(colour.b);
            center.push_back(nc.x);
            center.push_back(nc.y);
            
            /* Append to vector of centers. */
            centers.push_back(center);
            center_counts.push_back(0);
        }
    }
}

/*
 * Compute the distance between a cluster center and an individual pixel.
 *
 * Input : The cluster index (int), the pixel (CvPoint), and the Lab values of
 *         the pixel (CvScalar).
 * Output: The distance (double).
 */
double Slic::compute_dist(int ci, Coordinate pixel, Color colour) {
    double dc = sqrt(pow(centers[ci][0] - colour.r, 2) + pow(centers[ci][1]
            - colour.g, 2) + pow(centers[ci][2] - colour.b, 2));
    double ds = sqrt(pow(centers[ci][3] - pixel.x, 2) + pow(centers[ci][4] - pixel.y, 2));
    return sqrt(pow(dc / nc, 2) + pow(ds / ns, 2));
    
    //double w = 1.0 / (pow(ns / nc, 2));
    //return sqrt(dc) + sqrt(ds * w);
}

double compute_dist(double *ci, Coordinate pixel, Color colour, int step, int nc) {
    int ns = step;
    double dc = sqrt(pow(*(ci+0) - colour.r, 2) + pow(*(ci+1)
            - colour.g, 2) + pow(*(ci+2) - colour.b, 2));
    double ds = sqrt(pow(*(ci+3) - pixel.x, 2) + pow(*(ci+4) - pixel.y, 2));
    return sqrt(pow(dc / nc, 2) + pow(ds / ns, 2));
    
    //double w = 1.0 / (pow(ns / nc, 2));
    //return sqrt(dc) + sqrt(ds * w);
}

/*
 * Find a local gradient minimum of a pixel in a 3x3 neighbourhood. This
 * method is called upon initialization of the cluster centers.
 *
 * Input : The image (IplImage*) and the pixel center (CvPoint).
 * Output: The local gradient minimum (CvPoint).
 */
Coordinate Slic::find_local_minimum(Image *image, Coordinate center) {
    double min_grad = FLT_MAX;
    Coordinate loc_min = Coordinate(center.x, center.y);
    
    for (int i = center.x-1; i < center.x+2; i++) {
        for (int j = center.y-1; j < center.y+2; j++) {
            Color c1 = get2D(image, j+1, i);
            Color c2 = get2D(image, j, i+1);
            Color c3 = get2D(image, j, i);
            /* Convert colour values to grayscale values. */
            double i1 = c1.r;
            double i2 = c2.r;
            double i3 = c3.r;
            /*double i1 = c1.val[0] * 0.11 + c1.val[1] * 0.59 + c1.val[2] * 0.3;
            double i2 = c2.val[0] * 0.11 + c2.val[1] * 0.59 + c2.val[2] * 0.3;
            double i3 = c3.val[0] * 0.11 + c3.val[1] * 0.59 + c3.val[2] * 0.3;*/
            
            /* Compute horizontal and vertical gradients and keep track of the
               minimum. */
            if (sqrt(pow(i1 - i3, 2)) + sqrt(pow(i2 - i3,2)) < min_grad) {
                min_grad = fabs(i1 - i3) + fabs(i2 - i3);
                loc_min.x = i;
                loc_min.y = j;
            }
        }
    }
    
    return loc_min;
}

/*
 * Compute the over-segmentation based on the step-size and relative weighting
 * of the pixel and colour values.
 *
 * Input : The Lab image (IplImage*), the stepsize (int), and the weight (int).
 * Output: -
 */
void Slic::generate_superpixels(Image *image, int step, int nc) {
    this->step = step;
    this->nc = nc;
    this->ns = step;
    
    /* Clear previous data (if any), and re-initialize it. */
    clear_data();
    init_data(image);

    /* Run EM for 10 iterations (as prescribed by the algorithm). */
    for (int i = 0; i < NR_ITERATIONS; i++) {
        /* Reset distance values. */
        for (int j = 0; j < image->width; j++) {
            for (int k = 0;k < image->height; k++) {
                distances[j][k] = FLT_MAX;
            }
        }

        for (int j = 0; j < (int) centers.size(); j++) {
            /* Only compare to pixels in a 2 x step by 2 x step region. */
            for (int k = centers[j][3] - step; k < centers[j][3] + step; k++) {
                for (int l = centers[j][4] - step; l < centers[j][4] + step; l++) {
                
                    if (k >= 0 && k < image->width && l >= 0 && l < image->height) {
                        Color colour = get2D(image, l, k);
                        double d = compute_dist(j, Coordinate(k,l), colour);
                        
                        /* Update cluster allocation if the cluster minimizes the
                           distance. */
                        if (d < distances[k][l]) {
                            distances[k][l] = d;
                            clusters[k][l] = j;
                        }
                    }
                }
            }
        }
                
        /* Clear the center values. */
        for (int j = 0; j < (int) centers.size(); j++) {
            centers[j][0] = centers[j][1] = centers[j][2] = centers[j][3] = centers[j][4] = 0;
            center_counts[j] = 0;
        }
        
        /* Compute the new cluster centers. */
        for (int j = 0; j < image->width; j++) {
            for (int k = 0; k < image->height; k++) {
                int c_id = clusters[j][k];
                
                if (c_id != -1) {
                    Color colour = get2D(image, k, j);
                    
                    centers[c_id][0] += colour.r;
                    centers[c_id][1] += colour.g;
                    centers[c_id][2] += colour.b;
                    centers[c_id][3] += j;
                    centers[c_id][4] += k;
                    
                    center_counts[c_id] += 1;
                }
            }
        }

        /* Normalize the clusters. */
        for (int j = 0; j < (int) centers.size(); j++) {
            centers[j][0] /= center_counts[j];
            centers[j][1] /= center_counts[j];
            centers[j][2] /= center_counts[j];
            centers[j][3] /= center_counts[j];
            centers[j][4] /= center_counts[j];
        }

    }
    
}

void Slic::recalculate_centers(Image *image){
    /* Clear the center values. */
    for (int j = 0; j < (int) centers.size(); j++) {
        centers[j][0] = centers[j][1] = centers[j][2] = centers[j][3] = centers[j][4] = 0;
        center_counts[j] = 0;
    }
    
    /* Compute the new cluster centers. */
    for (int j = 0; j < image->width; j++) {
        for (int k = 0; k < image->height; k++) {
            int c_id = clusters[j][k];
            
            if (c_id != -1) {
                Color colour = get2D(image, k, j);
                
                centers[c_id][0] += colour.r;
                centers[c_id][1] += colour.g;
                centers[c_id][2] += colour.b;
                centers[c_id][3] += j;
                centers[c_id][4] += k;
                
                center_counts[c_id] += 1;
            }
        }
    }

    /* Normalize the clusters. */
    for (int j = 0; j < (int) centers.size(); j++) {
        centers[j][0] /= center_counts[j];
        centers[j][1] /= center_counts[j];
        centers[j][2] /= center_counts[j];
        centers[j][3] /= center_counts[j];
        centers[j][4] /= center_counts[j];
    }
}


void calculate_superpixel(Image *image, int step, int nc, double * centers,int n_centers, 
                        float *distances, int *clusters)
{
    for (int j = 0; j < image->width * image->height; j++) {

            distances[j] = FLT_MAX;
    }
    for (int j = 0; j < n_centers; j+=5) {
        /* Only compare to pixels in a 2 x step by 2 x step region. */
        for (int k = centers[j+3] - step; k < centers[j+3] + step; k++) {
            for (int l = centers[j+4] - step; l < centers[j+4] + step; l++) {
            
                if (k >= 0 && k < image->width && l >= 0 && l < image->height) {
                    Color colour = get2D(image, l, k);
                    double d = compute_dist((centers+j), Coordinate(k,l), colour,step,nc);
                    
                //     /* Update cluster allocation if the cluster minimizes the
                //         distance. */
                    if (d < distances[l*image->width + k]) {
                            distances[l*image->width + k] = d;
                             clusters[l*image->width + k] = j/5;
                    } 
                }
            }
        }
    }
}

/*
 * Enforce connectivity of the superpixels. This part is not actively discussed
 * in the paper, but forms an active part of the implementation of the authors
 * of the paper.
 *
 * Input : The image (IplImage*).
 * Output: -
 */
void Slic::create_connectivity(Image *image) {
    int label = 0, adjlabel = 0;
    const int lims = (image->width * image->height) / ((int)centers.size());
    
    const int dx4[4] = {-1,  0,  1,  0};
	const int dy4[4] = { 0, -1,  0,  1};
    
    /* Initialize the new cluster matrix. */
    vec2di new_clusters;
    for (int i = 0; i < image->width; i++) { 
        vector<int> nc;
        for (int j = 0; j < image->height; j++) {
            nc.push_back(-1);
        }
        new_clusters.push_back(nc);
    }

    for (int i = 0; i < image->width; i++) {
        for (int j = 0; j < image->height; j++) {
            if (new_clusters[i][j] == -1) {
                vector<Coordinate> elements;
                elements.push_back(Coordinate(i, j));
            
                /* Find an adjacent label, for possible use later. */
                for (int k = 0; k < 4; k++) {
                    int x = elements[0].x + dx4[k], y = elements[0].y + dy4[k];
                    
                    if (x >= 0 && x < image->width && y >= 0 && y < image->height) {
                        if (new_clusters[x][y] >= 0) {
                            adjlabel = new_clusters[x][y];
                        }
                    }
                }
                
                int count = 1;
                for (int c = 0; c < count; c++) {
                    for (int k = 0; k < 4; k++) {
                        int x = elements[c].x + dx4[k], y = elements[c].y + dy4[k];
                        
                        if (x >= 0 && x < image->width && y >= 0 && y < image->height) {
                            if (new_clusters[x][y] == -1 && clusters[i][j] == clusters[x][y]) {
                                elements.push_back(Coordinate(x, y));
                                new_clusters[x][y] = label;
                                count += 1;
                            }
                        }
                    }
                }
                
                /* Use the earlier found adjacent label if a segment size is
                   smaller than a limit. */
                if (count <= lims >> 2) {
                    for (int c = 0; c < count; c++) {
                        new_clusters[elements[c].x][elements[c].y] = adjlabel;
                    }
                    label -= 1;
                }
                label += 1;
            }
        }
    }
}

/*
 * Display the cluster centers.
 *
 * Input : The image to display upon (IplImage*) and the colour (CvScalar).
 * Output: -
 */
/*void Slic::display_center_grid(Image *image, Color colour) {
    for (int i = 0; i < (int) centers.size(); i++) {
        cvCircle(image, Coordinate(centers[i][3], centers[i][4]), 2, colour, 2);
    }
}*/

/*
 * Display a single pixel wide contour around the clusters.
 *
 * Input : The target image (IplImage*) and contour colour (CvScalar).
 * Output: -
 */
void Slic::display_contours(Image *image, Color colour) {
    const int dx8[8] = {-1, -1,  0,  1, 1, 1, 0, -1};
	const int dy8[8] = { 0, -1, -1, -1, 0, 1, 1,  1};
	
	/* Initialize the contour vector and the matrix detailing whether a pixel
	 * is already taken to be a contour. */
	vector<Coordinate> contours;
	vec2db istaken;
	for (int i = 0; i < image->width; i++) { 
        vector<bool> nb;
        for (int j = 0; j < image->height; j++) {
            nb.push_back(false);
        }
        istaken.push_back(nb);
    }
    
    /* Go through all the pixels. */
    for (int i = 0; i < image->width; i++) {
        for (int j = 0; j < image->height; j++) {
            int nr_p = 0;
            
            /* Compare the pixel to its 8 neighbours. */
            for (int k = 0; k < 8; k++) {
                int x = i + dx8[k], y = j + dy8[k];
                
                if (x >= 0 && x < image->width && y >= 0 && y < image->height) {
                    if (istaken[x][y] == false && clusters[i][j] != clusters[x][y]) {
                        nr_p += 1;
                    }
                }
            }
            
            /* Add the pixel to the contour list if desired. */
            if (nr_p >= 2) {
                contours.push_back(Coordinate(i,j));
                istaken[i][j] = true;
            }
        }
    }
    
    /* Draw the contour pixels. */
    for (int i = 0; i < (int)contours.size(); i++) {
       set2D(image, contours[i].y, contours[i].x, colour);
    }
}

/*
 * Give the pixels of each cluster the same colour values. The specified colour
 * is the mean RGB colour per cluster.
 *
 * Input : The target image (IplImage*).
 * Output: -
 */
void Slic::colour_with_cluster_means(Image *image) {
    vector<Color> colours(centers.size());
    
    /* Gather the colour values per cluster. */
    for (int i = 0; i < image->width; i++) {
        for (int j = 0; j < image->height; j++) {
            int index = clusters[i][j];
            Color colour = get2D(image, j, i);
            
            colours[index].r += colour.r;
            colours[index].g += colour.g;
            colours[index].b += colour.b;
        }
    }
    
    /* Divide by the number of pixels per cluster to get the mean colour. */
    for (int i = 0; i < (int)colours.size(); i++) {
        colours[i].r /= center_counts[i];
        colours[i].g /= center_counts[i];
        colours[i].b /= center_counts[i];
    }
    
    /* Fill in. */
    for (int i = 0; i < image->width; i++) {
        for (int j = 0; j < image->height; j++) {
            Color ncolour = colours[clusters[i][j]];
            set2D(image, j, i, ncolour);
        }
    }
}


void Slic::centersToArray(double *buffer){
    int head = 0;
    for (int i = 0 ; i < centers.size(); i++){
        for (int j = 0 ; j < 5; j++){
            buffer[head] = centers[i][j];
            head++;
        }
    }
}

void Slic::updateClusters(int* n_clusters,int width,int height){
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                clusters[i][j]= n_clusters[j*width+i];
            }
            
        }
}

void Slic::initialize(Image* image,int step,int nc){
    this->step = step;
    this->nc = nc;
    this->ns = step;

    /* Clear previous data (if any), and re-initialize it. */
    clear_data();
    init_data(image);


}