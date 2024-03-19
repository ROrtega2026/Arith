#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include <math.h>
#include <mem.h>
#include <stdlib.h>
#include <string.h>

void map_error(int col, int row, A2Methods_UArray2 arr, void *elem, void *cl);

typedef struct Closure {
        double *error;
        A2Methods_UArray2 otherPixels;
        A2Methods_T methods;
        unsigned int denominator1;
        unsigned int denominator2;
} *Closure;

int main(int argc, char *argv[]) {
        if (argc != 3) {
                printf("Not correct arguments\n");;
                return EXIT_FAILURE;
        }

        FILE *f1;
        FILE *f2;
        if (strcmp(argv[1], "-") != 0 && strcmp(argv[2], "-") != 0) {
                f1 = fopen(argv[1], "r");
                f2 = fopen(argv[2], "r");
        }

        else if (strcmp(argv[1], "-") == 0 && strcmp(argv[2], "-") != 0) {
                f1 = stdin;
                f2 = fopen(argv[2], "r");
        }

        else {
                f1 = fopen(argv[1], "r");
                f2 = stdin;
        }
        
        A2Methods_T methods = uarray2_methods_plain; 
        Pnm_ppm pic1 = Pnm_ppmread(f1, methods);
        Pnm_ppm pic2 = Pnm_ppmread(f2, methods);

        if (abs((int)pic1->width - (int)pic2->width) > 1 || abs((int)pic1->height - (int)pic2->height) > 1) {
                fprintf(stderr, "Difference in dimensions to big\n");
                printf("1.0");
                return EXIT_FAILURE;
        }

        Closure cl;
        NEW(cl);

        double error = 0;
        cl->error = &error;
        cl->otherPixels = pic2->pixels;
        cl->methods = methods;
        cl->denominator1 = pic1->denominator;
        cl->denominator2 = pic2->denominator;

        methods->map_default(pic1->pixels, map_error, cl);

        error /= (3.0 * fmin(pic1->width, pic2->width) * fmin(pic1->height, pic2->height));
        error = sqrt(error);

        printf("Error: %.4f\n", error);
        FREE(cl);
        Pnm_ppmfree(&pic1);
        Pnm_ppmfree(&pic2);
        if (f1 != stdin) {
                fclose(f1);
        }
        if (f1 != stdin) {
                fclose(f2);
        }

        return EXIT_SUCCESS;
}

void map_error(int col, int row, A2Methods_UArray2 arr, void *elem, void *cl)
{
        (void)arr;
        Closure closure = cl;
        double *error = closure->error;
        A2Methods_UArray2 secondArr = closure->otherPixels;
        A2Methods_T methods = closure->methods;
        double denominator1 = closure->denominator1;
        double denominator2 = closure->denominator2;

        if (col >= methods->width(secondArr) || row >= methods->height(secondArr)) {
                return;
        }

        Pnm_rgb pixel = (Pnm_rgb)elem;

        Pnm_rgb otherPixel = (Pnm_rgb)methods->at(secondArr, col, row);

        *error += ((pow((double)(pixel->red / denominator1) - (double)(otherPixel->red / denominator2), 2) + 
                pow((double)(pixel->blue / denominator1) - (double)(otherPixel->blue / denominator2), 2) + 
                pow((double)(pixel->green / denominator1) - (double)(otherPixel->green / denominator2), 2)));
}
