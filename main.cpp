/*
  (C) Nathan Geffen 2013 under GPL version 3.0.

  See COPYING for license.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cpa.h"
#include "match_pair.h"

/* Size of array */

using namespace std;
using namespace mp;

static const size_t CPA_SIZE = 10;

static const size_t NUM_INDIV = 20;

void match(void* data)
{
  Indiv* i = (Indiv *) data;
  printf("Entries index, entries[index] %p %d\n", i, i->age);
}

void cpa_test()
{
  size_t i;
  unsigned rand_array[CPA_SIZE];
  Cpa *cpa;
  Indiv population[NUM_INDIV], *indiv_addresses[NUM_INDIV], *indiv;
  time_t seconds;
  
  for (i = 0; i < CPA_SIZE; ++i) {
    population[i].sex = (unsigned) i % 2;
    population[i].age = (unsigned) i;
    population[i].risk_group = (unsigned) i % 5;
    population[i].partner = NULL;
    indiv_addresses[i] = &population[i];
  }

  cpa = cpa_new(CPA_SIZE, (void**) indiv_addresses, NULL);
  for (i = 0; i < cpa->size; ++i)
    printf("CPA: %zu %p %.2f %.2f\n", i, cpa->entries[i].data, cpa->entries[i].weight,
           cpa->entries[i].cumulative_weight);

  /* Binary searches through cpa */
  seconds = time(NULL);
  printf("BINARY SEARCHES\n");
  for (i = 0; i < cpa->size; ++i) {
    rand_array[i] = rand() % ((unsigned) cpa->cumulative_weight);
    if ((indiv = (Indiv *) cpa_binary_search(cpa, rand_array[i]))) {
      printf("Found: %zu %d %p %d\n", i, rand_array[i], indiv, indiv->age);
    } else {
      printf("Not found - this shouldn't happen %zu %d\n", i, rand_array[i]);
    }
  }
  seconds = time(NULL) - seconds;
  printf("Binary took: %ld seconds\n", seconds);
  
  /* Reset parts of cpa so that linear searches can be done */
  for (i = 0; i < cpa->size; ++i) cpa->entries[i].found = 0;
  cpa->cumulative_weight = cpa->entries[cpa->size - 1].cumulative_weight; 

  /* Linear searches through cpa */
  seconds = time(NULL);
  printf("LINEAR SEARCHES\n");
  for (i = 0; i < cpa->size; ++i) {
    if ((indiv = (Indiv *) cpa_linear_search(cpa, rand_array[i]))) {
      printf("Found: %zu %d %p %d\n", i, rand_array[i], indiv, indiv->age);
    } else {
      printf("Not found - this shouldn't happen %zu %d\n", i, rand_array[i]);
    }
  }
  seconds = time(NULL) - seconds;
  printf("Linear took %ld seconds\n", seconds);  

  cpa_traverse(cpa, match);

  Cpa_iterator iterator = {};
  cpa_reset(cpa);
  while( (indiv = (Indiv*) cpa_iterate(cpa, &iterator)) ) {
    printf("Iterating: %p %d\n", indiv, indiv->age);    
  }
  
  cpa = cpa_free(cpa);
}


int main(int argc, char *argv[])
{

  cpa_test();

  vector<Indiv> population;

  size_t num_indiv = argc > 1 ? atoi(argv[1]) : NUM_INDIV;
  unsigned num_executions = argc > 2 ? atoi(argv[2]) : 1;

  for (size_t i = 0; i < num_indiv; ++i) {
    Indiv ind;
    ind.sex = (unsigned) i % 2;
    ind.age = (unsigned) rand_int_range(17, 65);
    ind.age_group = ind.age / 5;
    ind.risk_group = (unsigned) rand_int_range(0, 1);
    ind.partner = NULL;
    population.push_back(ind);
  }

  // printf("BEFORE MATCH_PAIR\n");
  // print_partners(population);

  for(unsigned i = 0; i < num_executions; ++i) {
    match_pair(population);
    printf("MATCHES %d\n", i);
    print_partners(population);
  }
  return 0;
}
