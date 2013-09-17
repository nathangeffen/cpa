/*
  (C) Nathan Geffen and Leigh Johnson 2013 under GPL version 3.0. 
  This is free software.
  See the file called COPYING for the license.

  # Definitions of functions for pair matching algorithm.

  See match_pair.h for documentation of extern functions. Only functions
  not declared in match_pair.h are documented here. These should not be 
  called by programs using this library.

  PROBLEMS
  --------
  1. Is the weighting a real or natural number? (Affects random num gen). 
  1a. Can a weight be zero? (Please say no)
  2. Can Indiv be changed to use pointers instead of indices?
  3. No handling of secondary partners here.
  4. Can we filter out all those not available for pairing upfront?

  WHAT LEIGH NEEDS TO CODE
  ------------------------
  1. can_pair(Indiv) - determines if an individual is available for pairing.
  2. select_age_group
  3. generate_weight

*/

#include <algorithm>
#include <vector>
#include <iterator>

#include <cstdio>

#include "match_pair.h"
#include "cpa.h"

using namespace std;

namespace mp {


  /**
     This is a function that perhaps needs to be implemented in a more
     sophisticated way.
  */

  bool can_pair_default(const Indiv *indiv)
  {
    if (indiv)
      return true;
    return false;
  }

  unsigned search_age_groups(const vector< unsigned> age_group, const unsigned key)
  {
    unsigned low = 0, high = age_group.size() - 1;

    while (true) {      
      if (high <= low) { 
        return low < age_group.size() 
          ? rand_int_range( (int) high, (int) low )
          : high;
      }
      unsigned index = (low + high + 1) / 2;
      if (age_group[index] == key) return index;
      if (age_group[index] > key)  {
        high = index - 1;
        continue;
      }
      low = index + 1;
    }
  }

  unsigned select_age_group_default(const vector< unsigned > age_group, 
                                    const Indiv *ind)
  {
    return search_age_groups(age_group, ind->age_group);
  }

  unsigned generate_weight_default(const Indiv *ind) 
  {
    if (ind->age >= 15 && ind->age < 40) return 3;
    if (ind->age >= 40 && ind->age < 50) return 2;    
    return 1;
  }

  vector< unsigned > non_empty_cpa(Cpa* cpa[], unsigned sex, unsigned risk) 
  {
    vector< unsigned > cpa_counts;
  
    for (unsigned i = 0; i < HIGHEST_AGE_GROUP; ++i) {
      unsigned j = index(sex, risk, i);
      if(cpa[ j ]->size) 
        cpa_counts.push_back(i); // Store the age group not the index
    }
    return cpa_counts;
  }


  void print_partners(const vector<Indiv> &population)
  {
    unsigned k = 0;
    for (vector<Indiv>::const_iterator 
           i = population.begin(); i != population.end(); ++i, ++k) {
      printf("%d: ", k);
      if (i->partner) {        
        printf("Person %p: sex %d age %d risk %d - ", 
               &*i, i->sex, i->age, i->risk_group);
        printf("Person %p: sex %d age %d risk %d\n", &*i->partner, i->partner->sex, 
               i->partner->age, i->partner->risk_group);
      } else {
        printf("Person %p: sex %d age %d risk %d no partner\n", 
               &*i, i->sex, i->age, i->risk_group);
      }
    }
  }

  void match_pair(vector<Indiv> &population, bool (can_pair)(const Indiv*),
                  unsigned (select_age_group) 
                  (const vector< unsigned >, const Indiv*), 
                  unsigned (generate_weight)(const Indiv*))
  {
    // Initialize cumulative probability arrays 
    // Shuffle indices into population of individuals array
    vector< size_t > indices;
    indices.reserve(population.size());
    for(size_t i = 0; i < population.size(); ++i) indices.push_back(i);
    random_shuffle(indices.begin(), indices.end(), rand_int_to_open);

    // Set the CPA sizes and initialize the CPAs
    unsigned cpa_sizes[NUM_CPA] = {0};
    for(vector<Indiv>::iterator 
          i = population.begin(); i !=population.end(); ++i) {
      if( can_pair(&*i) ) {
        ++cpa_sizes[ index(i->sex, i->risk_group, i->age_group) ];
        i->eligible = true;
      } else {
        i->eligible = false;
      }
    }
    Cpa *cpa[NUM_CPA];
    Cpa_iterator cpa_iterator[NUM_CPA];
    for(size_t j = 0; j < NUM_CPA; ++j)  {
      cpa[j] = cpa_new(cpa_sizes[j], NULL, NULL);
      cpa_iterator[j].stack_size = 0; cpa_iterator[j].started = 0;
    }

    // Assign each eligible individual to one of the CPAs. 
    for(size_t i = 0; i != indices.size(); ++i) {
      Indiv *ind = &population[indices[i]];
      if (ind->eligible) {
        cpa_append(cpa[index(ind->sex, ind->risk_group, ind->age_group)], 
                   (Indiv *) ind, generate_weight(ind));
      }
    }

    // Make vectors of 4 non empty CPAs from which potential mates can be drawn
    // Each element of these vectors is an index to a CPA age group
    // Index of MALE, LOW = 0
    //          MALE, HIGH = 1
    //          FEMALE, LOW = 2
    //          FEMALE, HIGH = 3
    vector < unsigned > age_groups[4];
    age_groups[MALE * 2 + HIGH] = non_empty_cpa(cpa, MALE, HIGH);
    age_groups[FEMALE * 2 + HIGH] = non_empty_cpa(cpa, FEMALE, HIGH);
    age_groups[MALE * 2 + LOW] = non_empty_cpa(cpa, MALE, LOW);
    age_groups[FEMALE * 2 + LOW] = non_empty_cpa(cpa, FEMALE, LOW);

    unsigned iterations = 0;
    while( age_groups[ MALE * 2 + HIGH ].size() + 
           age_groups[ FEMALE * 2 + HIGH ].size() ) {
      ++iterations;
      // Choose a high risk cpa
      // randomly select sex
      unsigned from_sex;
      if (age_groups[ MALE * 2 + HIGH ].size() && 
          age_groups[ FEMALE * 2 + HIGH ].size()) {
        from_sex = rand_int_to(1);
      } else {
        from_sex = age_groups[ MALE * 2 + HIGH ].size() ? MALE : FEMALE;
      }

      // randomly select age group
      unsigned from_age_group_index = 
        rand_int_to(age_groups[from_sex * 2 + HIGH].size() - 1);
      unsigned from_age_group = 
        age_groups[from_sex * 2 + HIGH][from_age_group_index];
      unsigned cpa_from = index(from_sex, HIGH, from_age_group);
      Indiv *ind_from = 
        (Indiv *) cpa_iterate(cpa[cpa_from], &cpa_iterator[cpa_from]);
      // Before finding partner, check if we have to update the non-empty CPAs
      if (cpa_all_found(cpa[cpa_from])) { // No people left in this CPA
        age_groups[from_sex * 2 + HIGH].
          erase(age_groups[from_sex * 2 + HIGH].begin() 
                + from_age_group_index);
      }
      assert(ind_from);
      // Now find partner
      unsigned to_sex = ~from_sex & 1;
      unsigned to_risk_group = 
        age_groups[to_sex * 2 + HIGH].size() ? HIGH : LOW;
      unsigned to_age_group_index = 
        select_age_group(age_groups[to_sex * 2 + to_risk_group], ind_from);
      unsigned to_age_group = 
        age_groups[to_sex * 2 + to_risk_group][to_age_group_index];
      unsigned cpa_to = index(to_sex, to_risk_group, to_age_group);
      double weight = rand_int_to_open(cpa[cpa_to]->cumulative_weight);
      Indiv* ind_to = (Indiv *) cpa_binary_search(cpa[cpa_to], weight);
      assert(ind_to);
      // Check if we have to update the non-empty CPAs
      if (cpa_all_found(cpa[cpa_to])) { // No people left in this CPA
        age_groups[to_sex * 2 + to_risk_group].
          erase(age_groups[to_sex * 2 + to_risk_group].begin() + 
                to_age_group_index);
      }      
      if(ind_from->partner) ind_from->partner->partner = NULL;
      ind_from->partner = ind_to;
      if(ind_to->partner) ind_to->partner->partner = NULL;
      ind_to->partner = ind_from;
    }
    for(size_t i = 0; i < NUM_CPA; ++i) cpa_free(cpa[i]);
  }

} // namespace
