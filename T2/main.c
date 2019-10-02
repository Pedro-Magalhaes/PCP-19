#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define TOL 0.1
#define NUM_THREADS 3

double fx3(double x);
double fx2(double x);

double calculaArea( double a, double b, double (*f)(double));

int main(int argc, char *argv[]) {
    
    int a = 1;
    int b = 2;
    double vetor_area[NUM_THREADS];
    double incremento = fabs(b-a)/(double)NUM_THREADS;
    double inicio = a;
    double fim = 0;
    double area = 0;
    int i=0;
    #pragma omp parallel for num_threads(NUM_THREADS) reduction(+:area)
    for (i=0; i<NUM_THREADS; i++) {
                
        double ini = a + (i*incremento);
        double fim = a+((i+1)*incremento);
        if(i == NUM_THREADS - 1) { // tratando erro numerico de divisão do intervalo
            fim = b;
        }
        area += calculaArea(ini,fim, fx2);
    }
    printf("area somada = %lf \n",area);

    return 0;
}


double calculaArea(double a, double b, double (*f)(double)) {
    double h = fabs(b-a);
    double meio = (a+b)/2;
    double hmeio = h/2;
    double area_trapezio_maior =  ((f(a) + f(b)) * h) / 2;
    double area_trapezios =  (((f(a) + f(meio)) * hmeio) / 2 )+ (((f(b) + f(meio)) * hmeio) / 2);
    printf("area+t_m = %lf , area_ts = %lf f(a) = %lf  f(b) = %lf h = %lf abs=%lf\n", area_trapezio_maior,area_trapezios, f(a),f(b), h, fabs(b-a));
    if(fabs(area_trapezio_maior - area_trapezios) > (double)TOL) {
        area_trapezios = calculaArea(a,meio,f) + calculaArea(meio,b,f);
        // printf("area+t_m = %lf , area_ts = %lf \n", area_trapezio_maior,area_trapezios);
    }

    return area_trapezios;
}

double fx2(double x) {
    return x * x;
}
double fx3(double x) {
    return x * x * x;
}