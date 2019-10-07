#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>

typedef struct Tarefa{
    double a;
    double b;
    double fa;
    double fb;
    double area;
} tarefa;

tarefa fifo[256];
int fifo_tail;
int fifo_head;
int fifo_n_data;

#define FIFO_MAX 256;

int fifo_data_isavailable();
int fifo_data_isfull();
int InsereTarefa(tarefa data);
tarefa RetiraTarefa(void);

double TOL;

// TODO add funcoes complexas
double fx3(double x);
double fx2(double x);

// double calculaArea( double a, double b, double TOL, double (*f)(double));
double calculaArea( double a, double b, double (*f)(double));

// TODO use argc and argv
int main(int argc, char *argv[]) {
    int NUM_THREADS = atoi(argv[1]);
    int a = atoi(argv[2]);
    int b = atoi(argv[3]);
    TOL = atof(argv[4]);

    printf("Nthreads=%d, a=%d, b=%d, Tol=%lf\n",NUM_THREADS,a,b,TOL);

    int threads_finalizadas = 0;
    double incremento = fabs(b-a)/(double)NUM_THREADS;
    double area = 0;
    int i=0;

    // INICIALIZA
    // Adiciona todas as tarefas iniciais (num_threads tarefas)
    // Lista vai conter tarefas
    // threads_finalizadas = 0

    for (i=0; i<NUM_THREADS; i++) {
        tarefa t;
        t.a = incremento*i + a;
        t.b = t.a + incremento;
        if(i == NUM_THREADS-1){
            t.b = b;
        }
        printf("inserindo a=%lf b=%lf \n", t.a,t.b);
        InsereTarefa(t);
    }

    omp_set_num_threads(NUM_THREADS);

    #pragma omp parallel
    printf("\tNumber of Threads: omp_get_num_threads() returned %d\n", omp_get_num_threads());
    while (threads_finalizadas < NUM_THREADS) {
        tarefa task;
        #pragma omp critical
        {
            task = RetiraTarefa();
        }
        if(task.area > -1){
            double areaCalc;
            #pragma omp critical
            {
                areaCalc = calculaArea(task.a,task.b, fx2);
            }
            printf("tarefa t.a=%lf t.b=%lf  areaCalc=%lf\n",task.a,task.b, areaCalc);

            if(areaCalc >= 0) {
                #pragma omp critical
                {
                    area += areaCalc;
                }
            }
        } else {
            #pragma omp critical
            {
                threads_finalizadas++;
            }
        }
    }
    printf("area somada = %lf \n",area);
    return 0;
}


double calculaArea(double a, double b, double (*f)(double)) {
// double calculaArea(double a, double b, double TOL, double (*f)(double)) {
    double h = fabs(b-a);
    double meio = (a+b)/2;
    double hmeio = h/2;
    double area_trapezio_maior =  ((f(a) + f(b)) * h) / 2;
    double area_trapezios =  (((f(a) + f(meio)) * hmeio) / 2 )+ (((f(b) + f(meio)) * hmeio) / 2);
    if(fabs(area_trapezio_maior - area_trapezios) > (double)TOL) {

        tarefa t1,t2;
        t1.a = a;
        t1.b = meio;
        t2.a = meio;
        t2.b = b;
        // printf("inserindo tarefas\n  t1.a=%lf t1.b=%lf\n  t2.a=%lf t2.b=%lf\n",t1.a,t1.b,t2.a,t2.b);
        InsereTarefa(t1);
        InsereTarefa(t2);
        return -1;
    }
    return area_trapezios;
}

double fx2(double x) {
    return x * x;
}
double fx3(double x) {
    return x * x * x;
}

int fifo_data_isavailable()
{
  if (fifo_n_data > 0) {
    return 1;
  }
  else {
    return 0;
  }
}

int fifo_data_isfull()
{
  if (fifo_n_data < 256)
    return 0;
  else
    return 1;
}

int InsereTarefa(tarefa data)
{
  // printf("  INSERIU tarefa => t.a=%lf t.b=%lf\n",data.a,data.b);
  if (!fifo_data_isfull()) {
    fifo[fifo_head] = data;
    if (fifo_head < 255)
    {
      fifo_head ++;
    }
    else {
      fifo_head = 0;
    }
    fifo_n_data ++;
    return 1;
  } else {
    return 0;
  }
}

tarefa RetiraTarefa(void) {
    tarefa data;
    if(fifo_data_isavailable()) {
        data = fifo[fifo_tail];
        // printf("  RETIROU - available!\tData => t.a=%lf t.b=%lf\n",data.a,data.b);
        if (fifo_tail < 255)  {
            fifo_tail ++;
        } else {
            fifo_tail = 0;
        }
        fifo_n_data --;
        return data;
    }
    // printf("  RETIROU - not available\tData => t.a=%lf t.b=%lf |   Area -1 FINALIZANDO\n",data.a,data.b);
    data.area = -1;
    return data;
}
