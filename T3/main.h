
// Funcoes ---------------------------------------------------------------------
void read_matrix_file(char* file_path);
void print_matrix(int* digraph);
int compute_cost(int c1, int c2);
int get_last_city(Tour* t);

bool Feasible(Tour* tour, int city);
void Add_city(Tour* tour, int city);
void Remove_last_city(Tour* tour);
void Push_copy(); //TODO
// -----------------------------------------------------------------------------
