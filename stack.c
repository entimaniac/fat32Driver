#include "stack.h"
#include "util.h"

void stack_init(stack *stack) {
  // initialize size and capacity
  stack->size = 0;
  stack->capacity = stack_INITIAL_CAPACITY;

  // allocate memory for stack->data
  stack->data = (int*)malloc(sizeof(int) * stack->capacity);
}

void stack_append(stack *stack, int value) {
  // make sure there's room to expand into
  stack_double_capacity_if_full(stack);

  // append the value and increment stack->size
//  stack->data[stack->size++] = value;
  stack->data[stack->size] = value;
  stack->size++;
}

int stack_get(stack *stack, int index) {
  if (index >= stack->size || index < 0) {
    printf("Index %d out of bounds for stack of size %d\n", index, stack->size);
    exit(1);
  }
  return stack->data[index];
}


void stack_set(stack *stack, int index, int value) {
  // zero fill the stack up to the desired index
  while (index >= stack->size) {
    stack_append(stack,0);
  }

  // set the value at the desired index
//  stack->data[index] = value;
  stack->data[stack->size++] = value;
}

void stack_double_capacity_if_full(stack *stack) {
  if (stack->size >= stack->capacity) {
    // double stack->capacity and resize the allocated memory accordingly
    stack->capacity *= 2;
    stack->data = (int*)realloc(stack->data, sizeof(int) * stack->capacity);
  }
}

int stack_get_size(stack *stack)
{
	return stack->size;
}

void stack_remove(stack *stack, int value)
{
	int i, j;
	int flag = 0;
	for(i = 0; i < stack->size; i++){
		if(stack->data[i],value == 0){
			flag = 1;
			break;
		}
	}
	if(flag == 1){
		for(j = i; i < stack->size-1; i++){
			stack->data[i] = stack->data[i+1];
		}
		stack->size--;
		stack->capacity = stack->size;
		stack->data = (int*)realloc(stack->data, sizeof(int) * stack->capacity);
	}
}
void stack_free(stack *stack) {
  free(stack->data);
}
