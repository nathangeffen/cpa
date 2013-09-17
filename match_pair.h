/**
  (C) Nathan Geffen and Leigh Johnson 2013 under GPL version 3.0. 
  This is free software. See the file called COPYING for the license.

  # Iteratively match pairs of members in different cumulative probability 
  arrays

  This code is used to simulate heterosexual pairing in a microsimulation model
  of sexually transmitted infections.

*/

#ifndef MATCH_PAIR_H
#define MATCH_PAIR_H

#include <stdlib.h>
#include <vector>

#include "randomc.h"

using namespace std;

namespace mp {

  /** Maximum number of cumulative probability arrays. 

      There are 2 sexes and 2 risk groups. There are at most 24 5-year age 
      groups. 
      2 * 2 * 24 = 96, so we need no more than 96 cumulative probability arrays.
      These are indexed as follows: sex * 96/2 + risk * 96/4 + age_group.
      E.g. If sex = female (1) and risk = high (1) and age_group = 4 (20-24), then
      index = 0 * 48 + 1 * 24 + 4 = 28.
   */
  static const size_t NUM_CPA = 96;

  static const unsigned MALE = 0;
  static const unsigned FEMALE = 1;
  static const unsigned HIGHEST_AGE_GROUP = 24;
  static const unsigned LOW = 0;
  static const unsigned HIGH = 1;


  /** This is a much simplified version of Leigh's Indiv. Integration 
      will involve either including Leigh's Indiv definition or using 
      a template or void pointer.
   */

  struct indiv_s {
    unsigned sex;
    unsigned age;
    unsigned age_group;
    unsigned risk_group;
    bool eligible;
    struct indiv_s *partner; 
    struct indiv_s *secondary_partner;
  };

  typedef struct indiv_s Indiv;

  // Seed to the Mersenne Twister is arbitrarily chosen.
  static TRandomMersenne randGen(31279);

  /** Random number convenience functions.
   */

  inline int rand_int_range(int from, int to)  { 
    return randGen.IRandom(from, to);
  }

  inline int rand_int_range_open(int from, int to)
  {
    return randGen.IRandom(from, to - 1);
  }

  inline int rand_int_to(int to) { 
    return randGen.IRandom(0, to);
  }

  inline int rand_int_to_open(int to) { 
    return randGen.IRandom(0, to - 1);
  }


  /** Convenience function for calculating the index of the cumulative 
      probability array to use. 

      age should be called age_group because it's the age_group and not
      the age that is used.
  */
  inline size_t index(const size_t sex, const size_t risk, const size_t age) 
  {
    return sex * (NUM_CPA / 2) + risk * (NUM_CPA / 4) + age;
  }

  /** Place holder function to determine if an individual can pair.
      Leigh will have to write a more sophisticated version and 
      pass it as a parameter to pair_match.
   */
  bool can_pair_default(const Indiv *indiv);

  /** Place holder function to determine the age group to match to.
      Leigh will have to write a more sophisticated version and 
      pass it as a parameter to pair_match.
   */
  unsigned select_age_group_default(const vector< unsigned > age_group, 
                                    const Indiv *ind);

  /** Place holder function to determine the weight of an Individual.
      Leigh will have to write a more sophisticated version and 
      pass it as a parameter to pair_match.

      Although the default function is not good enough, its replacement 
      will have to be modelled on it, by calling search_age_groups
      with the ideal age group to match to as a parameter.
      search_age_groups finds the closest age group that still have 
      individuals in it that have not been matched.
   */
  unsigned generate_weight_default(const Indiv *ind);

  /** Used for debugging */
  void print_partners(const vector<Indiv> &population);

  /** This is the implementation of the main algorithm. 
      
      Input parameters:

      can_pair: function to determine if an individual can pair.
      Defaults to can_pair_default.

      select_age_group: function to detemine ideal age_group 
      to match person to. Defaults to select_age_group_default.

      generate_weight: function to determine weight of individual in 
      cumulative probability array. Defaults to generate_weight_default
   */

  void match_pair(vector<Indiv> &population, 
                  bool (can_pair)(const Indiv*) = can_pair_default, 
                  unsigned (select_age_group) 
                  (const vector< unsigned >, const Indiv*) = 
                  select_age_group_default,
                  unsigned (generate_weight)(const Indiv*) = 
                  generate_weight_default);
}
#endif /* MATCH_PAIR_H */

