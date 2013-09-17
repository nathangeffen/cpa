/**
  (C) Nathan Geffen 2013 under GPL version 3.0. This is free software.
  See the file called COPYING for license.

  # Cumulative Probability Array Processing library.

  This code provides functions to generate and search a cumulative
  probability array. The binary search function handles selection without 
  replacement. It can be called iteratively and will return a different index 
  each time until there are no more indices to return. This code conforms to c89 and
  is also valid C++ .
 */

#ifndef CPA_H
#define CPA_H

/* Error codes */
static const int OUT_OF_MEMORY = 1;
static const int ZERO_ARRAY_SIZE = 2;
static const int NOT_FOUND = 3;

/* Entry in cumulative probability array */

struct cpa_entry_s {
  void *data;
  double weight;
  double cumulative_weight;
  double adder;
  double left_subtractor;
  double right_subtractor;
  double linear_subtractor;
  int found;  
};

typedef struct cpa_entry_s Cpa_entry;

/* Structure containing cumulative probability array and other 
   housekeeping information.
*/

struct cpa_s {
  Cpa_entry *entries;
  size_t capacity;
  size_t size;
  size_t num_found;
  double cumulative_weight;
  int error;
};

typedef struct cpa_s Cpa;

struct cpa_iterator_s {
  int started;
  size_t stack[64*3];
  size_t q[64];
  size_t stack_size;
  size_t q_size;
};

typedef struct cpa_iterator_s Cpa_iterator;

/**
   Generates a random integer in the semi-open range specified 
   by its two parameters. 

   E.g. This will generate either 2, 3, 4 or 5 but not 6.
   cpa_rand_range_int(2, 6); 
   
   Input parameters:

   from_closed: Beginning of closed range.   
 */

int cpa_rand_range_int(const int from_closed, const int to_open);

/**     
    Generates a weight for a CPA. Useful for test purposes. 
    
    Input parameters:
    
    data: This function does nothing with its input, but more 
    sophisticated weight generators might need to use data.
*/
double cpa_generate_weight(void* data);

/**
  Generates a new cumulative probability array with *size* entries
  and the array's data set to *data*. 

  Input parameters:

  size: number of entries in the array
  
  data: array of user data used to initialize the array
  
  generator: pointer to function that generates a probability for a given
  index. If set to NULL, the **for test purposes only** function, 
  cpa_generate_weight is called.
  
  Return value: cumulative probability array.
  
*/
Cpa  *cpa_new(const size_t size, void* data[], 
              double (* generator) (void *data));

/**
   Appends a data entry to a cumulative probability array.
   
   Input/Output parameters:
   
   cpa: cumulative probability array
   
   Input parameters:

   data: void pointer to the data to be added.

   weight: weight of entry in cumulative probability array
   
*/

void cpa_append(Cpa *cpa, void *data, double weight);

/**
   Returns 1 if all entries in the cumulative probability arra have been found
   or traversed.

   Input parameters:
   
   cpa: cumulative probability array
*/

int cpa_all_found(const Cpa* cpa);

/**
  Inefficiently searches a cumulative probability array for the given key 
  and returns a pointer to the data stored in the found entry, or NULL if not found.
  The time complexity of this algorithm is O(n), where n is the size of array.
  
  Input/output parameters:

  cpa: cumulative probability array
  
  Input parameter:

  key: random number key to search for.

  Return value: pointer to data stored in the found entry in the array.
*/
void *cpa_linear_search(Cpa *cpa, const double key);

/**
  Efficiently searches a cumulative probability array for the given key 
  and returns a pointer to the data stored in the found entry, or NULL if not found.
  The time complexity of this algorithm is O(log n), where n is the size of array.

  Input/output parameters:

  cpa: cumulative probability array

  Input parameter:

  key: random number key to search for.

  Returns value: pointer to data stored in the found entry in the array.
*/
void *cpa_binary_search(Cpa *cpa, const double key);

/**
  Does a binary traversal of a cumulative probability array. On each iteration it 
  calls a function parameter with the address of the data in the next entry of the array.
  Not clear that this function is useful, but it is available nevertheless.

  Input/output parameters:

  cpa: cumulative probability array

  Input parameters:

  func: function that is called with pointer to user data on each iteration
*/
void cpa_traverse(Cpa *cpa, void (func)(void*));

/**
   Resets all the entries in the cumulative probability array to
   found, so that a fresh iteration of it can be done.

  Input/output parameters:

  cpa: cumulative probability array
 */

void cpa_reset(Cpa * cpa);

/**
  On each successive call with the same variables, this function does a binary 
  iteration through a cumulative probability array, each time returning a pointer 
  to the data of the next entry in the array. It eventually returns NULL when there 
  are  no more entries to iterate.

  Input/output parameters:

  cpa: cumulative probability array

  cpa_iterator: keeps track of iteration information through cpa
*/
void *cpa_iterate(Cpa *cpa, Cpa_iterator *cpa_iterator);

/**
  Frees all memory used by a cumulative probability array and returns NULL. 
  This should be called when all processing with a cpa is complete.

  Input/output parameters:

  cpa: cumulative probability array to free
*/
Cpa  *cpa_free(Cpa *cpa);

#endif /* CPA_H */
