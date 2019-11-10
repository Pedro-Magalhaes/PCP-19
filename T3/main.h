typedef struct Tour_Struct Tour;
typedef Tour* tour_t;
typedef struct DEQueue_Struct DEQueue;
typedef struct Stack_Struct Stack;
typedef Stack* my_stack_t;
typedef int bool;

typedef struct barrier_struct Barrier;
typedef Barrier* my_barrier_t;
typedef DEQueue* my_queue_t;

// Funcoes ---------------------------------------------------------------------
void read_matrix_file(char* file_path);
void print_matrix(int* digraph);
int compute_cost(int c1, int c2);
int get_last_city(tour_t t);
void Print_winner(tour_t tour);

bool Feasible(tour_t tour, int city);
void Add_city(tour_t tour, int city);
void Remove_last_city(tour_t tour);
void Push_copy(); //TODO
void Print_tour(tour_t tour);

bool Terminated(my_stack_t stack, long my_rank);
tour_t WorkSteal(my_stack_t stack, long my_rank);
tour_t PopBottom(my_stack_t stack, long my_rank);


void set_tasks(my_stack_t stack);
void* Thread_tree_search(void* rank);
int Tour_city(tour_t tour, int i);
tour_t Pop(my_stack_t stack);
void Push(my_stack_t stack, tour_t tour);
void Push_copy(my_stack_t stack, tour_t tour);
tour_t Alloc_tour();
int Tour_cost(tour_t tour);
int Tour_count(tour_t t);

bool Empty_stack(my_stack_t stack);
my_stack_t Init_stack(void);
void Free_stack(my_stack_t stack);

void Free_tour(tour_t tour);
bool Best_tour(tour_t tour);
void Copy_tour(tour_t tour1, tour_t tour2);
void Update_best_tour(tour_t tour);
void Broeadcast_best_cost_to_all(int tour_cost);

void Print_stack(my_stack_t stack, long pthrdID, char* title);
void Print_queue(my_queue_t queue, long id, char* title);
// -----------------------------------------------------------------------------


void Set_init_tours(long my_rank, int* my_first_tour_p, int* my_last_tour_p);
void My_barrier_destroy(my_barrier_t bar);
my_barrier_t My_barrier_init(int thr_count);
void Build_initial_queue(void);
int Get_upper_bd_queue_sz(void);
tour_t getDEQsTour(my_queue_t q, int i);
void Partition_tree(long my_rank, my_stack_t stack);

void Init_tour(tour_t tour, int cost);
my_queue_t Init_queue(int size);
void Push_Bottom_DEQ(my_queue_t queue, tour_t tour);
tour_t Pop_Top_DEQ(my_queue_t queue);
bool wasVisited(tour_t tour, int city);
bool isQueueEmpty(my_queue_t queue);
