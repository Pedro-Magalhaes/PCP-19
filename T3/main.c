#include <stdio.h>

#include <stdlib.h>

#include <pthread.h>

#include <sys/sysinfo.h>

#include <math.h>

#define FIFO_MAX 100000
#define INF 2147483647

typedef struct Tarefa{
    double a;
    double b;
    double area;
} tarefa;

typedef struct Fila{
    int tail;
    int head;
    int n_data;
    pthread_mutex_t mutex;
    tarefa data[FIFO_MAX]
} fila;

tarefa fifo[FIFO_MAX];

fila **filas;

int fifo_data_isavailable(fila f);
int fifo_data_isfull(fila f);
int InsereTarefa(tarefa data, fila *f);
tarefa RetiraTarefa(fila f);

double f(double x);
void work(void* i);
double calculaArea(double a, double b, double (*func)(double), fila f);
double TOL,delta,sum;
int a,b,num_cpu;
pthread_mutex_t sum_mutex;

void exibe_filas(fila *tmp_flista, int i);
void exibe_filas_all(fila *tmp_flista);
void exibe_fila(fila tmp_flista);

int main(int argc, char * argv[]) {
    long i;
    pthread_t * worker_id;

    sum = 0.0;
    pthread_attr_t attr; /* descriptors */
    pthread_attr_init( & attr);
    pthread_attr_setscope( & attr, PTHREAD_SCOPE_SYSTEM);

    // num_cpu = get_nprocs(); TODO
    // num_cpu = 1;

    printf("n_cpu = %d\n",num_cpu);
    if (argc < 3 + 1) {
        printf("Erro, os parametros necessários não foram informados.\
								\nO Programa recebe os parametros nesta ordem:\
								\n\t<inicio do intervalo>\n\t<final do intervalo \
                \n\t<num_threads OU num_cpu>\n");
        printf(" Numero de parametros recebidos: %d\n", argc);
        exit(-1);
    }
    a  = atoi(argv[1]);
    b  = atoi(argv[2]);
    num_cpu = atoi(argv[3]); //TODO
    delta = (a-b)/num_cpu;
    filas = malloc(sizeof(fila) * num_cpu);
    worker_id = malloc(sizeof(pthread_t) * num_cpu);

    pthread_mutex_init(&sum_mutex, NULL);
    // inicializa filas
    for (int i = 0; i < num_cpu; i++) {
        tarefa t;
        t.a = a + i*delta;
        t.b = i == num_cpu - 1 ? b : a + delta; // o ultimo tem que ir até b
        t.area = 0;
        fila f;
        pthread_mutex_t mutex;
        f.tail = 0;
        f.head = 0;
        f.n_data = 0;
        f.mutex = mutex;

        // filas[i].n_data = 0;
        filas[i] = f;

        pthread_mutex_init(&filas[i].mutex, NULL);
        pthread_mutex_lock(&filas[i].mutex);
        InsereTarefa(t, &filas[i]); // inserindo primeira tarefa
        pthread_mutex_unlock(&filas[i].mutex);
    }
    // cria os workers
    for (int i = 0; i < num_cpu; i++) {
        pthread_create( & worker_id[i], & attr, work, (void * ) i);
    }

    for (int i = 0; i < num_cpu; i++) {
        pthread_join(worker_id[i], NULL);
    }

    printf("Sum = %lf\n",sum);

    // free(filas);
    return 0;
}

void work(void* i) {
    int index = (int) i;
    printf("\t\tCalled 'work(%d)' exibindo FILAS...\n",index);
    // exibe_filas(filas, index);
    exibe_filas_all(filas);
    printf("n_data = %d\n",filas[index].n_data); // WHYYYYY?
    pthread_mutex_lock(&filas[index].mutex);
    if( fifo_data_isavailable(filas[index]) == 1) {
        tarefa my_task = RetiraTarefa(filas[index]);
        pthread_mutex_unlock(&filas[index].mutex);

        my_task.area = calculaArea(my_task.a,my_task.b,f,filas[index]);
        if(my_task.area != -1) {
            pthread_mutex_lock(&sum_mutex);
            sum += my_task.area;
            pthread_mutex_unlock(&sum_mutex);
        }
        work(i);
    } else {
        pthread_mutex_unlock(&filas[index].mutex);
    }

    return;
}

double f(double x) {
    // return sin(x);
    return x*x;
}


int fifo_data_isavailable(fila f) {
  if (f.n_data > 0) {
    return 1;
  }
  else {
    return 0;
  }
}

int fifo_data_isfull(fila f) {
  if (f.n_data < FIFO_MAX)
    return 0;
  else
    return 1;
}

int InsereTarefa(tarefa data, fila *f) {
  printf("  INSERIU tarefa => t.a=%lf t.b=%lf\n",data.a,data.b);
  if (!fifo_data_isfull(*f)) {
    // fifo[f.head] = data; //original

    f->data[f->head] = data;
    // f->data[0] = data;

    if (f->head < FIFO_MAX-1)
    {
      f->head ++;
    }
    else {
      f->head = 0;
    }
    f->n_data ++;
    exibe_fila(*f);
    return 1;
  } else {
    exibe_fila(*f);
    return 0;
  }
}

tarefa RetiraTarefa(fila f) {
    tarefa data;
    if(fifo_data_isavailable(f)) {
        data = f.data[f.tail];
        printf("  RETIROU - available!\tData => t.a=%lf t.b=%lf\n",data.a,data.b);
        if (f.tail < FIFO_MAX-1)  {
            f.tail ++;
        } else {
            f.tail = 0;
        }
        f.n_data --;
        return data;
    }
    printf("  RETIROU - not available\tData => t.a=%lf t.b=%lf |   Area -1 FINALIZANDO\n",data.a,data.b);
    data.area = -1;
    return data;
}


double calculaArea(double a, double b, double (*func)(double), fila f) {
    double h = fabs(b-a);
    double meio = (a+b)/2;
    double hmeio = h/2;
    double fa,fb,fm;
    fa = fabs(func(a));
    fb = fabs(func(b));
    fm = fabs(func(meio));
    double area_trapezio_maior =  ((fa + fb) * h) / 2;
    double area_trapezios =  (((fa + fm) * hmeio) / 2 )+ (((fb + fm) * hmeio) / 2);
    if(fabs(area_trapezio_maior - area_trapezios) > (double)TOL) {
        tarefa t1,t2;
        t1.a = a;
        t1.b = meio;
        t2.a = meio;
        t2.b = b;
        // printf("inserindo tarefas\n  t1.a=%lf t1.b=%lf\n  t2.a=%lf t2.b=%lf\n",t1.a,t1.b,t2.a,t2.b);
        pthread_mutex_lock(&f.mutex);
        InsereTarefa(t1,&f);
        InsereTarefa(t2,&f);
        pthread_mutex_unlock(&f.mutex);
        return -1;
    }
    return area_trapezios;
}


void exibe_filas(fila *tmp_flista, int i) {
  printf("\t\tExibindo filAS[%d]\n",i);
  printf("\t\t\t tail: %d \n",tmp_flista[i].tail);
  printf("\t\t\t head: %d \n",tmp_flista[i].head);
  printf("\t\t\t n_data: %d \n",tmp_flista[i].n_data);
  printf("\t\t\t tarefa.a: %lf \n",tmp_flista[i].data[0].a);
  printf("\t\t\t tarefa.b: %lf \n",tmp_flista[i].data[0].b);
  printf("\t\t\t tarefa.area: %lf \n",tmp_flista[i].data[0].area);
  printf("\t\tEND\n");
}

void exibe_filas_all(fila *tmp_flista) {
  printf("\t\tExibindo filas ALL\n");
  for(int i=0; i< num_cpu; i++) {
    printf("\t\tExibindo fila[%d]\n",i);
    printf("\t\t\t tail: %d \n",tmp_flista[i].tail);
    printf("\t\t\t head: %d \n",tmp_flista[i].head);
    printf("\t\t\t n_data: %d \n",tmp_flista[i].n_data);
    printf("\t\t\t tarefa.a: %lf \n",tmp_flista[i].data[0].a);
    printf("\t\t\t tarefa.b: %lf \n",tmp_flista[i].data[0].b);
    printf("\t\t\t tarefa.area: %lf \n",tmp_flista[i].data[0].area);
    printf("\t\tEND\n");
  }
}

void exibe_fila(fila tmp_flista) {
  printf("\t\tExibindo filA\n");
  printf("\t\t\t tail: %d \n",tmp_flista.tail);
  printf("\t\t\t head: %d \n",tmp_flista.head);
  printf("\t\t\t n_data: %d \n",tmp_flista.n_data);
  printf("\t\t\t tarefa.a: %lf \n",tmp_flista.data[0].a);
  printf("\t\t\t tarefa.b: %lf \n",tmp_flista.data[0].b);
  printf("\t\t\t tarefa.area: %lf \n",tmp_flista.data[0].area);
  printf("\t\tEND\n");
}
