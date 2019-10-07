#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

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
#define INF 2147483647;

int fifo_data_isavailable();
int fifo_data_isfull();
int InsereTarefa(tarefa data);
tarefa RetiraTarefa(void);

double TOL;

// TODO ajustar funcoes
double fx3(double x);
double fx2(double x);
double fx7(double x);
double fx13(double x);
double fx14(double x);

double calculaArea1( double a, double b, double (*f)(double));
double calculaArea2( double a, double b, double (*f)(double));

int main(int argc, char *argv[]) {

    int NUM_THREADS;
    int a;
    int b;
    int version;

    if (argc < 6) {
        printf("Erro, os parametros necessários não foram informados.\
                \nO Programa recebe os parametros nesta ordem:\
                \n\t<numero de threads>\n\t<valor de 'a'>\n\t<valor de 'b'>\
                \n\t<tolerancia>\n\t<versao da implementacao> (valor inteiro 1 ou 2)");
        printf("\nNumero de parametros recebidos: %d\n", argc);
        exit(-1);
    }

    NUM_THREADS = atoi(argv[1]);
    a = atoi(argv[2]);
    b = atoi(argv[3]);
    TOL = atof(argv[4]);
    version = atof(argv[5]);
    printf("Nthreads=%d, a=%d, b=%d, Tol=%lf, Implementacao=%d\n",NUM_THREADS,a,b,TOL, version);

    double incremento = fabs(b-a)/(double)NUM_THREADS;
    double area = 0;
    int i=0;
    clock_t start;
    clock_t end;
    double time_spent;

    if (version == 1) {
      double vetor_area[NUM_THREADS];
      start = clock();

      #pragma omp parallel for num_threads(NUM_THREADS) reduction(+:area)
      for (i=0; i<NUM_THREADS; i++) {
          double ini = a + (i*incremento);
          double fim = a+((i+1)*incremento);
          if(i == NUM_THREADS - 1) { // tratando erro numerico de divisão do intervalo
              fim = b;
          }
          area += calculaArea1(ini,fim, fx14);
      }
      end = clock();
      time_spent = (double)(end - start) / CLOCKS_PER_SEC;
      printf("Implementacao 1: area somada = %lf | tempo = %f\n",area, time_spent);
    }

    else if (version == 2) {
      int threads_finalizadas = 0;
      start = clock();

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
          // printf("inserindo a=%lf b=%lf \n", t.a,t.b);
          InsereTarefa(t);
      }

      omp_set_num_threads(NUM_THREADS);

      #pragma omp parallel
      // printf("\tNumber of Threads: omp_get_num_threads() returned %d\n", omp_get_num_threads());
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
                  areaCalc = calculaArea2(task.a,task.b, fx14);
              }
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
      end = clock();
      time_spent = (double)(end - start) / CLOCKS_PER_SEC;
      printf("Implementacao 2: area somada = %lf | tempo = %f\n",area, time_spent);
    }
    return 0;
}


double calculaArea1(double a, double b, double (*f)(double)) {
    double h = fabs(b-a);
    double meio = (a+b)/2;
    double hmeio = h/2;
    double area_trapezio_maior =  ((f(a) + f(b)) * h) / 2;
    double area_trapezios =  (((f(a) + f(meio)) * hmeio) / 2 )+ (((f(b) + f(meio)) * hmeio) / 2);
    // printf("area+t_m = %lf , area_ts = %lf f(a) = %lf  f(b) = %lf h = %lf abs=%lf\n", area_trapezio_maior,area_trapezios, f(a),f(b), h, fabs(b-a));
    if(fabs(area_trapezio_maior - area_trapezios) > (double)TOL) {
        area_trapezios = calculaArea1(a,meio,f) + calculaArea1(meio,b,f);
        // printf("area+t_m = %lf , area_ts = %lf \n", area_trapezio_maior,area_trapezios);
    }

    return area_trapezios;
}


double calculaArea2(double a, double b, double (*f)(double)) {
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

double fx7(double x) {
    long double pt1 = sqrt(x);
    long double pt2 = x*x;
    long double pt3 = 1 - (x*x);
    if (pt3 == 0.0) {
      pt3 = 0.00001;
    }
    long double pt4 = sqrt(pt3);
    if (x == 0.0) {
      pt1 = pt4;
    }
    long double res = (pt1/pt4);
    // printf(" x*x = %Lf\n", pt2);
    // printf(" 1 - x*x = %Lf\n", pt3);
    // printf(" %lf / %lf = %Lf\n\n",pt1, pt4, res);
    return res;
}

double fx13(double x) {
    double res = exp(-(x*x)/2);
    // printf("result = %lf", res);
    return res;
}

double fx14(double x) {
    double res = exp(x);
    // double pt2 = cos(x);
    // double res = pt1 * pt2;
    // printf("result = %lf\n", res);
    return res;
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

int InsereTarefa(tarefa data) {
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
