#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define TOL 0.0001
#define NUM_THREADS 3


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
int fifo_push(tarefa data);
tarefa fifo_pull(void);




double fx3(double x);
double fx2(double x);

double calculaArea( double a, double b, double (*f)(double));
int InsereTarefa(tarefa t);
tarefa RetiraTarefa();


int main(int argc, char *argv[]) {

    int threads_finalizadas = 0;
    int a = 1;
    int b = 2;
    double vetor_area[NUM_THREADS];
    double incremento = fabs(b-a)/(double)NUM_THREADS;
    double inicio = a;
    double fim = 0;
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
        fifo_push(t);
    }



    #pragma omp parallel
    while (threads_finalizadas < NUM_THREADS) {
        tarefa task;
        #pragma omp critical
        {
            task = fifo_pull();
        }
        if(task.area > -1){
            double areaCalc;
            #pragma omp critical
            {
                areaCalc = calculaArea(task.a,task.b,fx2);
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
    
    // #pragma omp parallel for num_threads(NUM_THREADS) reduction(+:area)
    // for (i=0; i<NUM_THREADS; i++) {
        
    //     double ini = a + (i*incremento);
    //     double fim = a+((i+1)*incremento);
    //     if(i == NUM_THREADS - 1) { // tratando erro numerico de divisão do intervalo
    //         fim = b;
    //     }
    //     area += calculaArea(ini,fim, fx2);
    // }
    printf("area somada = %lf \n",area);

    return 0;
}


double calculaArea(double a, double b, double (*f)(double)) {
    double h = fabs(b-a);
    double meio = (a+b)/2;
    double hmeio = h/2;
    double area_trapezio_maior =  ((f(a) + f(b)) * h) / 2;
    double area_trapezios =  (((f(a) + f(meio)) * hmeio) / 2 )+ (((f(b) + f(meio)) * hmeio) / 2);
    // printf("area+t_m = %lf , area_ts = %lf f(a) = %lf  f(b) = %lf h = %lf abs=%lf\n", area_trapezio_maior,area_trapezios, f(a),f(b), h, fabs(b-a));
    if(fabs(area_trapezio_maior - area_trapezios) > (double)TOL) {
        
        tarefa t1,t2;
        t1.a = a;
        t1.b = meio;
        t2.a = meio;
        t2.b = b;
        // printf("inserindo tarefas\n t1.a=%lf t1.b=%lf\nt2.a=%lf t2.b=%lf\n",t1.a,t1.b,t2.a,t2.b);
        fifo_push(t1);
        fifo_push(t2);

        // area_trapezios = calculaArea(a,meio,f) + calculaArea(meio,b,f); // Add tarefa para lista

        // printf("area+t_m = %lf , area_ts = %lf \n", area_trapezio_maior,area_trapezios);
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
  if (fifo_n_data > 0)
  {
    return 1;
  }
  else
  {
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

int fifo_push(tarefa data)
{
  if (!fifo_data_isfull())
  {
    fifo[fifo_head] = data;
    if (fifo_head < 255)
    {
      fifo_head ++;
    }
    else
    {
      fifo_head = 0;
    }

    fifo_n_data ++;
    return 1;
  }
  else
  {
    return 0;
  }

}

tarefa fifo_pull(void) {
    tarefa data;
    if(fifo_data_isavailable()) {
        data = fifo[fifo_tail];
        if (fifo_tail < 255)  {
            fifo_tail ++;
        }
        else {
            fifo_tail = 0;
        }
        fifo_n_data --;
        return data;
    }
    data.area = -1;
    return data;
}
