/*NOTE:
 * Credit for the structure of stack.h and
 * stack.c is given to Happy Bear Software
 * Article via this link:
 * https://www.happybearsoftware.com/implementing-a-dynamic-array
*/
#ifndef STACK_H
#define STACK_H

#define stack_INITIAL_CAPACITY 100

// Define a stack type
typedef struct {
  int size;      // slots used so far
  int capacity;  // total available slots
  int* data;     // int array we're storing
} stack;

void stack_init(stack *stack);
void stack_append(stack *stack, int value);
int stack_get(stack *stack, int index);
void stack_set(stack *stack, int index, int value);
void stack_double_capacity_if_full(stack *stack);
int stack_get_size(stack *stack);
void stack_remove(stack *stack, int value);
void stack_free(stack *stack);

#endif
