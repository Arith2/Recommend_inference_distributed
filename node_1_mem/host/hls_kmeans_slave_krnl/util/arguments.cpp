

#include "arguments.h"


void parsing_args(int argc, char* argv[], size_t *n, size_t *k, size_t *d, int *threads, char *filename, bool isBinary, uint32_t*iter, uint32_t *repetition, int* batch_size, float* threshold){
  char c;
  FILE *file;

  if ( argc < 4 ){
    fprintf (stderr, "The parameters are obligatory.\n");
    fprintf (stderr, "Usage: ./MKM ");

    fprintf(stderr, "Obligatory parameters: \n");
    fprintf(stderr, "n (number of objects in millions)\nk (number of clusters)\nd (dimensionality)\n");
    fprintf(stderr, "Optional parameters: \n t number of threads\n\n");
    fprintf(stderr, "f (filename) if there is no filename we use random generated data [0.0, 1.0)\n");
    fprintf(stderr, "b use the -b argument without options to specify that it is a binary file.\n");
    fprintf(stderr, "Example (with default values): ./MKM -n 64 -k 40 -d 20 -t 4\n");
    exit(1);
  }

  while ((c = getopt(argc, argv, "bn:i:r:k:d:t:f:s:e:")) != -1) {
	if ( optarg ){
        switch(c){
            case 'n':
    		        *n = atol(optarg);                
    		    break;
    		    case 't':
    		        *threads = atoi(optarg);
    			  break;
            case 'd':
                *d = atoi(optarg);
            break;
            case 'k':
                *k = atoi(optarg);
            break;
            case 'f':
                strcpy(filename, optarg);
            break;
            case 'b':
                isBinary = true;
            break;
      	    case 'i':
            		*iter = atoi(optarg);
        		break;
            case 'r':
            		*repetition = atoi(optarg);
        		break;
            case 's':
                *batch_size = atoi(optarg);
            break;
            case 'e':
                *threshold = atoi(optarg);
            break;
            case '?':
              if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
              else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
              else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
              exit(1);
    		default:
    			break;
        }
	}

  }


}
