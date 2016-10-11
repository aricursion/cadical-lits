#ifndef _ema_hpp_INCLUDED
#define _ema_hpp_INCLUDED

#include <cassert>

namespace CaDiCaL {

class Internal;

// We have a more complex generic exponential moving average struct here
// for more robust initialization (see comments before 'update' below).

struct EMA {
  double value;         // current average value
  double alpha;         // percentage contribution of new values
  double beta;          // current upper approximation of alpha
  long wait;            // count-down using 'beta' instead of 'alpha'
  long period;          // length of current waiting phase

  EMA () : value (0), alpha (0), beta (0), wait (0), period (0) { }

  EMA (double a) :
     value (0), alpha (a), beta (1.0), wait (0), period (0)
  {
    assert (0 <= alpha), assert (alpha <= beta), assert (beta <= 1);
  }

  operator double () const { return value; }
  void update (Internal *, double y, const char * name);
};

};

#endif
