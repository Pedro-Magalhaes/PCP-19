typedef struct Tour_Struct Tour;
typedef Tour* tour_t;
typedef struct DEQueue_Struct DEQueue;
typedef struct Stack_Struct Stack;
typedef Stack* my_stack_t;
typedef int bool;

// Funcoes ---------------------------------------------------------------------
void read_matrix_file(char* file_path);
void print_matrix(int* digraph);
int compute_cost(int c1, int c2);
int get_last_city(tour_t t);

bool Feasible(tour_t tour, int city);
void Add_city(tour_t tour, int city);
void Remove_last_city(tour_t tour);
void Push_copy(); //TODO

void set_tasks(my_stack_t stack);
void Thread_tree_search(void* num);
int Tour_city(tour_t tour, int i);
tour_t Pop(my_stack_t stack);
void Push(my_stack_t stack, int city);
void Push_copy(my_stack_t stack, tour_t tour);
tour_t Alloc_tour();

bool Empty_stack(my_stack_t stack);
my_stack_t Init_stack(void);
void Free_stack(my_stack_t stack);

void Free_tour(tour_t tour);
bool Best_tour(tour_t tour);
void Copy_tour(tour_t tour1, tour_t tour2);
void Update_best_tour(tour_t tour);
void Broeadcast_best_cost_to_all(int tour_cost);

// -----------------------------------------------------------------------------
