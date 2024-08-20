#include "internal.hpp"
#include <algorithm>
#include <functional>
#include <tuple>

double lit_score (CaDiCaL::Internal *internal, int pos_lit) {
  switch (internal->opts.litprintmode) {
  case 0:
    return static_cast<double> (internal->sum_occ (pos_lit));
  case 1:
    return static_cast<double> (internal->prod_occ (pos_lit));
  case 2:
    return internal->sum_weighted_occ (pos_lit);
  case 3:
    return internal->prod_weighted_occ (pos_lit);
  default:
    printf ("Fatal error\n");
    exit (1);
  }
}
static std::tuple<int, double> select_lit (CaDiCaL::Internal *internal) {
  auto get_score = [&internal] (int pos_lit) {
    return lit_score (internal, pos_lit);
  };
  std::vector<int> lits;
  for (auto const &x : internal->litprint_occ_cnts) {
    lits.push_back (x.first);
  }

  std::sort (lits.begin (), lits.end (), [get_score] (int lit1, int lit2) {
    return get_score (lit1) > get_score (lit2);
  });
  for (auto const &lit : lits) {
    if (internal->val (lit) == 0 &&
        !internal->litprint_printed_lits.count (lit)) {
      return {lit, get_score (lit)};
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
    double score = std::get<1> (selected);
    printf ("%d : %lf", lit, score);
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

void dump_json (CaDiCaL::Internal *internal) {
  FILE *fp = fopen ("litgraph.json", "w");
  fprintf (fp, "{\n");
  uint64_t ctr = 0;
  for (auto pair : internal->litprint_graph) {
    ctr += 1;
    int lit = pair.first;
    auto scores = pair.second;
    fprintf (fp, "\"%d\" : [", lit);
    for (uint64_t i = 0; i < scores.size (); i++) {
      int time = scores[i].first;
      double score = scores[i].second;
      fprintf (fp, "[%d, %2f]", time, score);
      if (i != scores.size () - 1) {
        fprintf (fp, ",");
      }
    }
    fprintf (fp, "]");
    if (ctr != internal->litprint_graph.size ()) {
      fprintf (fp, ",");
    }
    fprintf(fp, "\n");
  }

  fprintf (fp, "}\n");
}
