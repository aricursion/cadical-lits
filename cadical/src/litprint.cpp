#include "internal.hpp"
#include <algorithm>
#include <functional>
#include <tuple>

static std::tuple<int, float> select_lit (CaDiCaL::Internal *internal) {
  std::function<float (CaDiCaL::Internal *, int)> get_score;
  switch (internal->opts.litprintmode) {
  case 0:
    get_score = [] (CaDiCaL::Internal *internal, int pos_lit) {
      return static_cast<float> (internal->sum_occ (pos_lit));
    };
    break;
  case 1:
    get_score = [] (CaDiCaL::Internal *internal, int pos_lit) {
      return static_cast<float> (internal->prod_occ (pos_lit));
    };
    break;
  case 2:
    get_score = [] (CaDiCaL::Internal *internal, int pos_lit) {
      return internal->sum_weighted_occ (pos_lit);
    };
    break;
  case 3:
    get_score = [] (CaDiCaL::Internal *internal, int pos_lit) {
      return internal->prod_weighted_occ (pos_lit);
    };
    break;
  default:
    printf ("fatal error\n");
    exit (1);
  }
  std::vector<int> lits;
  for (auto const &x : internal->litprint_occ_cnts) {
    lits.push_back (x.first);
  }

  std::sort (lits.begin (), lits.end (),
             [get_score, internal] (int lit1, int lit2) {
               return get_score (internal, lit1) >
                      get_score (internal, lit2);
             });
  for (auto const &lit : lits) {
    if (internal->val (lit) == 0 &&
        !internal->litprint_printed_lits.count (lit)) {
      return {lit, get_score (internal, lit)};
    }
  }

  return {0, 0.0};
}

void print_lit_set (CaDiCaL::Internal *internal) {
  int n = internal->opts.litsetsize;
  bool extra = internal->opts.litprintextra;
  printf ("c {");
  for (int i = 0; i < n; i++) {
    auto selected = select_lit (internal);
    int lit = std::get<0> (selected);
    if (lit <= 0)
      break;
    float score = std::get<1> (selected);
    printf ("%d : %f", lit, score);
    internal->litprint_print_cnt += 1;
    internal->litprint_printed_lits.insert (lit);
    if (i != n - 1)
      printf (", ");
  }
  if (internal->opts.litrecent)
    internal->litprint_occ_cnts = {};
  printf ("}");
  if (extra) {
    printf ("# time: %f", internal->process_time ());
  }
  printf ("\n");
  fflush (stdout);
}

void print_lit (CaDiCaL::Internal *internal) {
  auto selected = select_lit (internal);
  int lit = std::get<0> (selected);
  if (lit <= 0)
    return;
  printf ("c lit %d 0 # runtime: %lf # props: %lld\n", lit,
          internal->process_time (), internal->total_propagations ());
  internal->litprint_print_cnt++;
  internal->litprint_printed_lits.insert (lit);
  if (internal->opts.litrecent)
    internal->litprint_occ_cnts = {};

  int lit_mult =
      (internal->opts.litgapgrow > 1)
          ? internal->litprint_print_cnt * internal->opts.litgapgrow
          : 1;
  internal->litprint_next =
      internal->stats.learned.clauses + internal->opts.litgap * lit_mult;
}

bool should_print_lit (CaDiCaL::Internal *internal) {
  if (!internal->litprint_print_cnt &&
      internal->stats.learned.clauses >= internal->opts.litstart) {
    return true;
  }
  if (internal->litprint_print_cnt &&
      internal->stats.learned.clauses >= internal->litprint_next) {
    return true;
  }
  return false;
}
