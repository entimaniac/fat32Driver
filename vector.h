/*NOTE:
 * Credit for the structure of vector.h and
 * vector.c is given to Happy Bear Software
 * Article via this link:
 * https://www.happybearsoftware.com/implementing-a-dynamic-array
*/
#ifndef VECTOR_H
#define VECTOR_H

#define VECTOR_INITIAL_CAPACITY 100

// Define a vector type
typedef struct {
  int size;      // slots used so far
  int capacity;  // total available slots
  char** data;     // char array we're storing
  char** data_mode; //mode relating to data in char** data
} Vector;

void vector_init(Vector *vector);
void vector_append(Vector *vector, char* value, char* mode);
char* vector_get(Vector *vector, int index);
char* vector_get_mode(Vector *vector, int index);
void vector_set(Vector *vector, int index, char* value);
void vector_double_capacity_if_full(Vector *vector);
int vector_get_size(Vector *vector);
void vector_remove(Vector *vector, char* value);
void vector_free(Vector *vector);

#endif
