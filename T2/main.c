#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define FIFO_MAX 100000
#define INF 2147483647

typedef struct Tarefa{
    double a;
    double b;
    double area;
} tarefa;

tarefa fifo[FIFO_MAX];
int fifo_tail;
int fifo_head;
int fifo_n_data = 0;

int fifo_data_isavailable();
int fifo_data_isfull();
int InsereTarefa(tarefa data);
tarefa RetiraTarefa(void);

double TOL;

double fx1(double x);
double fx2(double x);
double fx3(double x);

double calculaArea1( double a, double b, double (*f)(double));
double calculaArea2( double a, double b, double (*f)(double));

int main(int argc, char *argv[]) {
    int NUM_THREADS;
    int a;
    int b;
    int version;
    int function = 1;
    double (*fx)(double);

    if (argc < 6) {
        printf("Erro, os parametros necessários não foram informados.\
                \nO Programa recebe os parametros nesta ordem:\
                \n\t<numero de threads>\n\t<valor de 'a'>\n\t<valor de 'b'>\
                \n\t<tolerancia>\n\t<versao da implementacao> (valor inteiro 1 ou 2)\
                \n\t[Opcional] <funcao de teste> (valor inteiro 1,2 ou 3)");
        printf("\nNumero de parametros recebidos: %d\n\n", argc);
        exit(-1);
    }

    NUM_THREADS = atoi(argv[1]);
    a = atoi(argv[2]);
    b = atoi(argv[3]);
    TOL = atof(argv[4]);
    version = atoi(argv[5]);

    if (version != 1 && version != 2) {
        printf("Erro, o parametro 5, <versao da implementacao>, deve ser um numero inteiro: 1 ou 2");
        exit(-1);
    }
    if (argc == 7) {
        function = atoi(argv[6]);
    }
    printf("Nthreads=%d, a=%d, b=%d, Tol=%lf, Implementacao=%d, Funcao=fx%d\n",\
            NUM_THREADS,a,b,TOL, version, function);

    double incremento = fabs(b-a)/(double)NUM_THREADS;
    double area = 0;
    int i=0;
    clock_t start;
    clock_t end;
    double time_spent;

    if(function == 1) {
        fx = fx1;
    } else if(function == 2) {
        fx = fx2;
    }  else if(function == 3) {
        fx = fx3;
    }

    /*
    *       Implementacao 1
    */
    if (version == 1) {
      start = clock();

      omp_set_num_threads(NUM_THREADS);
      // #pragma omp parallel for num_threads(NUM_THREADS) reduction(+:area)
      #pragma omp parallel for reduction(+:area)
      for (i=0; i<NUM_THREADS; i++) {
          double ini = a + (i*incremento);
          double fim = a+((i+1)*incremento);
          if(i == NUM_THREADS - 1) { // tratando erro numerico de divisão do intervalo
              fim = b;
          }
          area += calculaArea1(ini,fim, fx);
      }
      end = clock();
      time_spent = (double)(end - start) / (CLOCKS_PER_SEC*NUM_THREADS);
      printf("Implementacao 1: Area somada = %lf | tempo = %lf\n",area, time_spent);
    }

    /*
    *       Implementacao 2
    */
    else if (version == 2) {
      int threads_finalizadas = 0;

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
          InsereTarefa(t);
      }

      omp_set_num_threads(NUM_THREADS);
      start = clock();

      #pragma omp parallel
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
                  areaCalc = calculaArea2(task.a,task.b, fx);
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
      time_spent = (double)(end - start) / (CLOCKS_PER_SEC*NUM_THREADS);
      if(fifo_data_isavailable()) {
          printf("erro! ainda tinha tarefa****************\n");
      }
      printf("Implementacao 2: Area somada = %lf | tempo = %lf\n",area, time_spent);
    }
    return 0;
}


double calculaArea1(double a, double b, double (*f)(double)) {
    double h = fabs(b-a);
    double meio = (a+b)/2;
    double hmeio = h/2;
    double fa,fb,fm;
    fa = fabs(f(a));
    fb = fabs(f(b));
    fm = fabs(f(meio));
    double area_trapezio_maior =  ((fa + fb) * h) / 2;
    double area_trapezios =  (((fa + fm) * hmeio) / 2 )+ (((fb + fm) * hmeio) / 2);
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
    double fa,fb,fm;
    fa = fabs(f(a));
    fb = fabs(f(b));
    fm = fabs(f(meio));
    double area_trapezio_maior =  ((fa + fb) * h) / 2;
    double area_trapezios =  (((fa + fm) * hmeio) / 2 )+ (((fb + fm) * hmeio) / 2);
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

// Funcoes com carga de processamento relevante
double fx1(double x) {
    double res = 0;
    for (int i = 0; i < 10000; i++) {
        res += (exp(3*x)*sin(2*x));
    }
    return res;
}

double fx2(double x) {
    double res = 0;
    for (int i = 0; i < 100000; i++) {
        res += exp(x);
    }
    // res = exp(x);
    return res;
}

// Funcao leve
double fx3(double x) {
    double res = 0;
    for (int i = 0; i < 100; i++) {
        res += x*x;
    }
    return res;
}


// Funcoes descartadas
// double fx7(double x) {
//     double pt1 = sqrt(x);
//     double pt2 = x*x;
//     double pt3 = 1 - (x*x);
//     if (pt3 == 0.0) {
//       pt3 = 0.00001;
//     }
//     double pt4 = sqrt(pt3);
//     if (x == 0.0) {
//       pt1 = pt4;
//     }
//     double res = (pt1/pt4);
//     return res;
// }
//
// double fx2(double x) {
//     return x * x;
// }
// double fx3(double x) {
//     return x * x * x;
// }

int fifo_data_isavailable() {
  if (fifo_n_data > 0) {
    return 1;
  }
  else {
    return 0;
  }
}

int fifo_data_isfull() {
  if (fifo_n_data < FIFO_MAX)
    return 0;
  else
    return 1;
}

int InsereTarefa(tarefa data) {
  // printf("  INSERIU tarefa => t.a=%lf t.b=%lf\n",data.a,data.b);
  if (!fifo_data_isfull()) {
    fifo[fifo_head] = data;
    if (fifo_head < FIFO_MAX-1)
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
        if (fifo_tail < FIFO_MAX-1)  {
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
