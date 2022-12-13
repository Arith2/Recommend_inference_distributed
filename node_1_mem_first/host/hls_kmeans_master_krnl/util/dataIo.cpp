
#include "dataIo.h"

void random_init(float *array, const size_t N, const size_t D){
  /*  short unsigned seed[3];
    int i,j;
    int max = 10;

    // seed[0]=1; seed[1]=1; seed[2]=2;

    //#pragma omp parallel for firstprivate(seed)
    for ( i=0 ; i < N  ; i++ ){
	for(j=0; j<D; j++){
            array[i*D+j] = (double)(rand()%max);
	    // printf("%f ", array[i*D+j]);
	}
	// printf("\n");
    }*/
   short unsigned seed[3];
    int i;

    seed[0]=1; seed[1]=1; seed[2]=2;

    #pragma omp parallel for firstprivate(seed)
    for ( i=0 ; i < N * D ; i++ ){
        array[i] = erand48(seed);
    }

}

void read_file(float *array, const size_t N, const size_t D, const char *filename, bool isBinary){
    FILE *fp;
    size_t counts = 0;
    size_t i=0,j=0;
    char line[MAX_LINE_LENGTH];
    char *token=NULL;
    const char space[2] = " ";

    fp = fopen(filename,"r");

    if ( fp == NULL ){
        fprintf(stderr, "File '%s' does not exists!", filename);
        exit(1);
    }

    if ( isBinary ){
        // read binary file, everything at once
        counts = fread(array, sizeof(float) * N * D, 1, fp);

        if ( counts == 0 ) {
            fprintf(stderr, "Binary file '%s' could not be read. Wrong format.", filename);
            exit(1);
        }
    }else{
        // processing a text file
        // format: there are D float values each line. Each value is separated by a space character.
        // notice MAX_LINE_LENGTH = 2049
        i = 0;
        while ( fgets ( line, MAX_LINE_LENGTH, fp ) != NULL &&
                i < N ) {


            if ( line[0] != '%'){ // ignore '%' comment char
                token = strtok(line, space);
                j=0;


                while ( token != NULL &&
                        j < D ){
                            
                    array[i*D + j] = atof(token); // 0.0 if no valid conversion
                    token = strtok(NULL, space);
                    j++;
                }
                i++;
            }
        }
    }

    fclose(fp);
}

void save_binary_file(float *array, const size_t N, const size_t D, char filename[]){
    FILE *fp=NULL;
    size_t counts = 0;

    fp = fopen(filename, "w");

    if ( fp == NULL ){
        fprintf(stderr, "Could not open file '%s'!", filename);
        exit(1);
    }

    counts = fwrite(array,sizeof(float) * N * D, 1, fp);

    if ( counts == 0 ){
        fprintf(stderr, "Error in writing file '%s'. Abort.", filename);
        exit(1);
    }

    fclose(fp);
}

void save_text_file(float *array, const size_t N, const size_t D, char filename[]){
    FILE *fp=NULL;
    size_t counts = 0;
    size_t i=0, j=0;
    char line[MAX_LINE_LENGTH];
    char strfloat[50];

    fp = fopen(filename, "w");

    if ( fp == NULL ){
        fprintf(stderr, "Could not open file '%s'!", filename);
        exit(1);
    }

    for ( i=0 ; i < N ; i++ ){
        strcpy(line, "");
        for ( j=0 ; j < D ; j++ ){
            strcpy(strfloat, "");
            sprintf(strfloat, "%f ", array[i*D + j]);
            strcat(line, strfloat);

        }
        fprintf(fp, "%s\n", line);

    }

    fclose(fp);
}
