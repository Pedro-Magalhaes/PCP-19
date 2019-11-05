
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
void TreeSearch(void* num);
int Tour_city(tour_t tour, int i);
tour_t Pop(my_stack_t stack);
void Push(my_stack_t stack, int city);
void Push_copy(my_stack_t stack, tour_t tour);
tour_t Alloc_tour();
bool Empty_stack(my_stack_t stack);
void Free_tour(tour_t tour);
bool Best_tour(tour_t tour);

// -----------------------------------------------------------------------------
