/*
  (C) Nathan Geffen 2013 under GPL version 3.0. This is free software.
  See the file called COPYING for the license.

  # Definitions of functions for processing cumulative probability arrays.

  See cpa.h for documentation of extern functions. Only functions not declared in 
  cpa.h are documented here. These should not be called by programs using this library.
*/

#include <assert.h>  
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cpa.h"

/*
  This function uses a very poor random number generating technique.
  Use just for test purposes.
*/
double cpa_generate_probability(void* data)
{
  return (double) (rand() % 10) + 1;
}

Cpa  *cpa_new(const size_t size, void* data[], 
              double (* generator) (void *data))
{
  size_t i;
  Cpa *cpa;
  cpa = (Cpa *) malloc(sizeof(Cpa));
  if (!cpa) return NULL;
  cpa->entries = (Cpa_entry *) malloc(sizeof(Cpa_entry) * size);
  if (!cpa->entries || size == 0) {
    cpa->error = size ? OUT_OF_MEMORY : ZERO_ARRAY_SIZE;
    cpa->size = 0;
    cpa->capacity = 0;
    cpa->num_found = 0;
    return cpa;
  }
  cpa->error = 0;
  cpa->size = 0;
  cpa->capacity = size;
  cpa->num_found = 0;
  if (!generator) generator = cpa_generate_probability;
  if (data) 
    for (i = 0; i < size; ++i) 
      cpa_append(cpa, data[i], generator(data[i]));
  return cpa;
}

void cpa_append(Cpa *cpa, void *data, double weight) 
{
  assert(weight != 0.0);
  cpa->entries[cpa->size].data = data;
  cpa->entries[cpa->size].adder = 0.0;
  cpa->entries[cpa->size].left_subtractor = 0.0;
  cpa->entries[cpa->size].right_subtractor = 0.0;
  cpa->entries[cpa->size].linear_subtractor = 0.0;
  cpa->entries[cpa->size].found = 0;
  cpa->entries[cpa->size].weight = weight;
  if (cpa->size == 0) 
    cpa->entries[cpa->size].cumulative_weight = 
      cpa->entries[cpa->size].weight;
  else 
    cpa->entries[cpa->size].cumulative_weight = 
      cpa->entries[cpa->size - 1].cumulative_weight +
      cpa->entries[cpa->size].weight;          
  cpa->cumulative_weight = cpa->entries[cpa->size].cumulative_weight;
  ++cpa->size;
}

int cpa_all_found(const Cpa* cpa) 
{
  return cpa->num_found == cpa->size;
}

void *cpa_linear_search(Cpa *cpa, const double key) 
{
  size_t i;
  double subtractor = 0.0, comparator;

  for (i = 0; i < cpa->size; ++i) {
    subtractor += cpa->entries[i].linear_subtractor;
    if (!cpa->entries[i].found) {
      comparator = cpa->entries[i].cumulative_weight + subtractor;
      if (key < comparator &&
          key >= comparator - cpa->entries[i].weight) {
        cpa->entries[i].found = 1;
        ++cpa->num_found;
        cpa->entries[i].linear_subtractor -= cpa->entries[i].weight;
        cpa->cumulative_weight -= cpa->entries[i].weight;
        return cpa->entries[i].data;
      }
    }
  }
  return NULL;
}

/*
  This function is used by the binary search and binary traversal 
  functions to ensure these algorithms find the correct 
  cumulative probability entry on successive calls. 
*/

void cpa_set_subtractors(Cpa *cpa, const size_t q[], const size_t q_size, 
                         const size_t found_index)
{
  size_t j;
  int set = 0;
  ++cpa->num_found;
  cpa->entries[found_index].found = 1;
  for (j = 0; j < q_size; ++j) {
    if (!set && q[j] > found_index) {
      set = 1;
      cpa->entries[q[j]].left_subtractor -= cpa->entries[found_index].weight;
      cpa->entries[q[j]].right_subtractor -= cpa->entries[found_index].weight;
    } else if (set && q[j] < found_index) {
      set = 0;
      cpa->entries[q[j]].left_subtractor += cpa->entries[found_index].weight;  
      cpa->entries[q[j]].right_subtractor += cpa->entries[found_index].weight;
    } else if (!set && q[j] == found_index) {
      cpa->entries[q[j]].right_subtractor -= cpa->entries[found_index].weight;
    } else if (set && q[j] == found_index) {
      cpa->entries[q[j]].left_subtractor += cpa->entries[found_index].weight;
    }
  }
}

void *cpa_binary_search(Cpa *cpa, const double key)
{
  size_t lower = 0, higher = cpa->size - 1, q_size = 0, i;
  size_t q[64];
  double subtractor = 0.0;
  
  while(1) {
    if ( (signed) higher < (signed) lower) return NULL;  /* Not found */
    i = (lower + higher + 1) / 2;
    q[q_size] = i;
    ++q_size;
    subtractor += cpa->entries[i].right_subtractor;
    if (cpa->entries[i].cumulative_weight + subtractor <= key) {
      lower = i + 1;
      continue;
    } 
    subtractor -= cpa->entries[i].right_subtractor;  
    subtractor += cpa->entries[i].left_subtractor;
    if (cpa->entries[i].found || 
        cpa->entries[i].cumulative_weight + subtractor - 
        cpa->entries[i].weight  > key) {
      higher = i - 1;
      continue;
    }
    cpa->cumulative_weight -= cpa->entries[i].weight;
    cpa_set_subtractors(cpa, q, q_size, i);
    return cpa->entries[i].data;
  }
}

void cpa_traverse(Cpa *cpa, void (func)(void*))
{
  size_t stack[64*3];
  size_t counter = 0;
  size_t q[64], q_size = 0, high, low, index;

  stack[0] = 0;              /* initial low */
  stack[1] = cpa->size - 1;  /* initial high */
  stack[2] = 1;              /* initial q size */
  ++counter;  

  while(counter) {
    --counter;
    low = stack[counter*3];
    high = stack[counter*3+1];
    q_size = stack[counter*3+2];
    index = (low + high + 1) / 2;
    q[q_size-1] = index;
    if (index > low) {
      stack[counter*3] = low;
      stack[counter*3+1] = index - 1;
      stack[counter*3+2] = q_size + 1;
      ++counter;
    }
    if (index < high) {
      stack[counter*3] = index + 1;
      stack[counter*3+1] = high;
      stack[counter*3+2] = q_size + 1;
      ++counter;
    }
    if (func) {
      func(cpa->entries[index].data);
    }
    cpa->cumulative_weight -= cpa->entries[index].weight;
    cpa_set_subtractors(cpa, q, q_size, index);
  }
}

/**
   
*/

void cpa_reset(Cpa * cpa) 
{
  size_t i;
  for (i = 0; i < cpa->size; ++i) cpa->entries[i].found = 0;
}

/*
  Used by cpa_iterate to initialize the iterator.
*/

void cpa_iterate_init(const Cpa *cpa, Cpa_iterator *cpa_iterator)
{
  cpa_iterator->started = 1;
  cpa_iterator->stack[0] = 0;              /* initial low */
  cpa_iterator->stack[1] = cpa->size - 1;  /* initial high */
  cpa_iterator->stack[2] = 1;              /* initial q size */ 
  cpa_iterator->q_size = 0;
  cpa_iterator->stack_size = 1;  
} 

void* cpa_iterate(Cpa *cpa, Cpa_iterator *cpa_iterator)
{
  size_t low, high, index;

  if(!cpa_iterator->stack_size) {
    if (cpa_iterator->started) return NULL;
    cpa_iterate_init(cpa, cpa_iterator);
  }

  do {
    --cpa_iterator->stack_size;
    low = cpa_iterator->stack[cpa_iterator->stack_size*3];
    high = cpa_iterator->stack[cpa_iterator->stack_size*3+1];
    cpa_iterator->q_size = cpa_iterator->stack[cpa_iterator->stack_size*3+2];
    index = (low + high + 1) / 2;
    cpa_iterator->q[cpa_iterator->q_size-1] = index;
    if (index > low) {
      cpa_iterator->stack[cpa_iterator->stack_size*3] = low;
      cpa_iterator->stack[cpa_iterator->stack_size*3+1] = index - 1;
      cpa_iterator->stack[cpa_iterator->stack_size*3+2] = 
        cpa_iterator->q_size + 1;
      ++cpa_iterator->stack_size;
    }
    if (index < high) {
      cpa_iterator->stack[cpa_iterator->stack_size*3] = index + 1;
      cpa_iterator->stack[cpa_iterator->stack_size*3+1] = high;
      cpa_iterator->stack[cpa_iterator->stack_size*3+2] = 
        cpa_iterator->q_size + 1;
      ++cpa_iterator->stack_size;
    }
  } while (cpa->entries[index].found);
  cpa->cumulative_weight -= cpa->entries[index].weight;
  cpa_set_subtractors(cpa, cpa_iterator->q, cpa_iterator->q_size, index);
  return cpa->entries[index].data;
}

Cpa *cpa_free(Cpa *cpa)
{
  free(cpa->entries);
  free(cpa);
  return NULL;
}
