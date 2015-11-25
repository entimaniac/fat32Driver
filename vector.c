/*NOTE:
 * Credit for the structure of vector.h and
 * vector.c is given to Happy Bear Software
 * Article via this link:
 * https://www.happybearsoftware.com/implementing-a-dynamic-array
*/

#include "vector.h"
#include "util.h"

void vector_init(Vector *vector) {
  // initialize size and capacity
  vector->size = 0;
  vector->capacity = VECTOR_INITIAL_CAPACITY;

  // allocate memory for vector->data
  vector->data = malloc(sizeof(char**) * vector->capacity);
  vector->data_mode = malloc(sizeof(char**) * vector->capacity);
}

void vector_append(Vector *vector, char* value, char* mode) {
  // make sure there's room to expand into
  vector_double_capacity_if_full(vector);

  // append the value and increment vector->size
//  vector->data[vector->size++] = value;
  vector->data[vector->size] = malloc(sizeof(char*)*vector->capacity);
  vector->data_mode[vector->size] = malloc(sizeof(char*)*vector->capacity);
  strcpy(vector->data[vector->size],value);
  strcpy(vector->data_mode[vector->size],mode);
  vector->size++;
}

char* vector_get(Vector *vector, int index) {
  if (index >= vector->size || index < 0) {
    printf("Index %d out of bounds for vector of size %d\n", index, vector->size);
    exit(1);
  }
  return vector->data[index];
}

void vector_set(Vector *vector, int index, char* value) {
  // zero fill the vector up to the desired index
  while (index >= vector->size) {
    vector_append(vector,"","");
  }

  // set the value at the desired index
//  vector->data[index] = value;
  strcpy(vector->data[vector->size++],value);
}

void vector_double_capacity_if_full(Vector *vector) {
  if (vector->size >= vector->capacity) {
    // double vector->capacity and resize the allocated memory accordingly
    vector->capacity *= 2;
    vector->data = realloc(vector->data, sizeof(char**) * vector->capacity);
  }
}

int vector_get_size(Vector *vector)
{
	return vector->size;
}

void vector_free(Vector *vector) {
  for(int i = 0; i < vector->size; i++){
    free(vector->data[i]);
    free(vector->data_mode[i]);
  }
  free(vector->data);
}
