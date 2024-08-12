#include "internal.hpp"

void print_lit_set (CaDiCaL::Internal *internal) {
  int n = internal->opts.litsetsize;
  bool extra = internal->opts.litprintextra;
  std::vector<std::pair<int, int>> sorted_litprint_counts (
      internal->litprint_occ_cnts.begin (),
      internal->litprint_occ_cnts.end ());
  std::sort (
      sorted_litprint_counts.begin (), sorted_litprint_counts.end (),
      [] (const std::pair<int, int> &a, const std::pair<int, int> &b) {
        return a.second > b.second;
      });
  printf ("c {");
  int idx = 0;
  int num_printed = 0;
  while (num_printed < n && (size_t) idx < sorted_litprint_counts.size ()) {
    auto p = sorted_litprint_counts[idx];
    if (internal->val (p.first) == 0 &&
        !internal->litprint_printed_lits.count (p.first)) {
      printf ("%d : %d", p.first, p.second);
      internal->litprint_printed_lits.insert (p.first);
      internal->litprint_print_cnt += 1;
      if (num_printed != n - 1)
        printf (", ");
      num_printed += 1;
    }
    idx += 1;
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

void print_lits (CaDiCaL::Internal *internal) {
  std::vector<std::pair<int, int>> sorted_litprint_counts (
      internal->litprint_occ_cnts.begin (),
      internal->litprint_occ_cnts.end ());

  std::sort (
      sorted_litprint_counts.begin (), sorted_litprint_counts.end (),
      [] (const std::pair<int, int> &a, const std::pair<int, int> &b) {
        return a.second > b.second;
      });

  for (std::pair<int, int> p : sorted_litprint_counts) {
    if (internal->val (p.first) == 0 &&
        !(internal->litprint_printed_lits.count (p.first))) {
      printf ("c lit %d 0 # runtime: %lf # props: %lld\n", p.first,
              internal->process_time (), internal->total_propagations ());
      internal->litprint_print_cnt++;
      internal->litprint_printed_lits.insert (p.first);
      if (internal->opts.litrecent)
        internal->litprint_occ_cnts = {};

      int lit_mult =
          (internal->opts.litgapgrow > 1)
              ? internal->litprint_print_cnt * internal->opts.litgapgrow
              : 1;
      internal->litprint_next = internal->stats.learned.clauses +
                                internal->opts.litgap * lit_mult;
      break;
    }
  }
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
