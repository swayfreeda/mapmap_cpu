/**
 * Copyright (C) 2016, Daniel Thuerck
 * TU Darmstadt - Graphics, Capture and Massively Parallel Computing
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 */

#ifndef __MAPMAP_HEADER_MAPMAP_H_
#define __MAPMAP_HEADER_MAPMAP_H_

#include <vector>
#include <memory>
#include <set>
#include <stdexcept>
#include <chrono>

#include "header/defines.h"
#include "header/vector_types.h"
#include "header/tree_sampler.h"
#include "header/dynamic_programming.h"
#include "header/graph.h"
#include "header/multilevel.h"
#include "header/termination_criterion.h"
#include "header/tree.h"
#include "header/tree_optimizer.h"
#include "tbb/tick_count.h"

NS_MAPMAP_BEGIN

/* constants for printing */
const char * const UNIX_COLOR_BLACK = "\033[0;30m";
const char * const UNIX_COLOR_DARKBLUE = "\033[0;34m";
const char * const UNIX_COLOR_DARKGREEN = "\033[0;32m";
const char * const UNIX_COLOR_DARKTEAL = "\033[0;36m";
const char * const UNIX_COLOR_DARKRED = "\033[0;31m";
const char * const UNIX_COLOR_DARKPINK = "\033[0;35m";
const char * const UNIX_COLOR_DARKYELLOW = "\033[0;33m";
const char * const UNIX_COLOR_GRAY = "\033[0;37m";
const char * const UNIX_COLOR_DARKGRAY = "\033[1;30m";
const char * const UNIX_COLOR_BLUE = "\033[1;34m";
const char * const UNIX_COLOR_GREEN = "\033[1;32m";
const char * const UNIX_COLOR_TEAL = "\033[1;36m";
const char * const UNIX_COLOR_RED = "\033[1;31m";
const char * const UNIX_COLOR_PINK = "\033[1;35m";
const char * const UNIX_COLOR_YELLOW = "\033[1;33m";
const char * const UNIX_COLOR_WHITE = "\033[1;37m";
const char * const UNIX_COLOR_RESET = "\033[0m";

template<typename COSTTYPE, uint_t SIMDWIDTH, typename UNARY, typename PAIRWISE>
class mapMAP
{
public:
    /* constructor for passing all components as ready-made objects */
    mapMAP();
    /* constructor for allowing manual construction of graph and label set */
    mapMAP(const luint_t num_nodes, const luint_t num_labels);
    ~mapMAP();

    /* set graph and label set */
    void set_graph(const Graph<COSTTYPE> * graph) throw();
    void set_label_set(const LabelSet<COSTTYPE, SIMDWIDTH> * label_set)
        throw();

    /* alternatively - construct graph and label set */
    void add_edge(const luint_t node_a, const luint_t node_b,
        const _s_t<COSTTYPE, SIMDWIDTH> weight = 1.0) throw();
    void set_node_label_set(const luint_t node_id, const
        std::vector<_iv_st<COSTTYPE, SIMDWIDTH>>& label_set) throw();

    /* set MRF cost functions */
    void set_unaries(const UNARY * unaries);
    void set_pairwise(const PAIRWISE * pairwise);

    /* configuration */
    void set_multilevel_criterion(MultilevelCriterion<COSTTYPE, SIMDWIDTH> *
        criterion);
    void set_termination_criterion(TerminationCriterion<COSTTYPE,
        SIMDWIDTH> * criterion);

    /**
     * callback for external logging - outputs time in ms and energy after
     */
    void set_logging_callback(const std::function<void (const luint_t,
        const _s_t<COSTTYPE, SIMDWIDTH>)>& callback);

    /* start optimization */
    _s_t<COSTTYPE, SIMDWIDTH> optimize(std::vector<_iv_st<COSTTYPE, SIMDWIDTH>>&
        solution) throw();

protected:
    /* setup, sanity checks and tools */
    void create_std_modules();
    bool check_data_complete();
    void record_time_from_start();
    void print_status();

    /* encapsule optimization steps */
    _s_t<COSTTYPE, SIMDWIDTH> initial_labelling();
    _s_t<COSTTYPE, SIMDWIDTH> opt_step_spanning_tree();
    _s_t<COSTTYPE, SIMDWIDTH> opt_step_multilevel();
    _s_t<COSTTYPE, SIMDWIDTH> opt_step_acyclic();
    bool check_termination();

protected:
    /* status flags */
    bool m_construct_graph;

    bool m_set_graph;
    bool m_set_label_set;
    bool m_set_unaries;
    bool m_set_pairwise;
    bool m_set_multilevel_criterion;
    bool m_set_termination_criterion;

    /* graph statistics for construction */
    luint_t m_num_nodes;
    luint_t m_num_labels;

    /* cost functions */
    const UNARY * m_unaries;
    const PAIRWISE * m_pairwise;

    /* underlying graph */
    const Graph<COSTTYPE> * m_graph;
    const LabelSet<COSTTYPE, SIMDWIDTH> * m_label_set;

    /* configuration */
    MultilevelCriterion<COSTTYPE, SIMDWIDTH> *
        m_multilevel_criterion;
    TerminationCriterion<COSTTYPE, SIMDWIDTH> *
        m_termination_criterion;

    luint_t m_num_roots = 64u;

    /* storage for functional modules */
    std::unique_ptr<Multilevel<COSTTYPE, SIMDWIDTH, UNARY, PAIRWISE>>
        m_multilevel;
    std::unique_ptr<MultilevelCriterion<COSTTYPE, SIMDWIDTH>>
        m_storage_multilevel_criterion;
    std::unique_ptr<TerminationCriterion<COSTTYPE, SIMDWIDTH>>
        m_storage_termination_criterion;

    /* utilities */
    std::set<luint_t> m_label_set_check;
    std::chrono::system_clock::time_point m_time_start;

    /* current solution */
    std::vector<_iv_st<COSTTYPE, SIMDWIDTH>> m_solution;
    _s_t<COSTTYPE, SIMDWIDTH> m_objective;

    /* solver history data */
    std::vector<_s_t<COSTTYPE, SIMDWIDTH>> m_hist_energy;
    std::vector<luint_t> m_hist_time;
    std::vector<SolverMode> m_hist_mode;

    luint_t m_hist_acyclic_iterations;
    luint_t m_hist_spanningtree_iterations;
    luint_t m_hist_multilevel_iterations;

    /* callback data */
    bool m_use_callback;
    std::function<void (const luint_t, const _s_t<COSTTYPE, SIMDWIDTH>)>
        m_callback;
};

NS_MAPMAP_END

#include "source/mapmap.impl.h"

#endif /* __MAPMAP_HEADER_MAPMAP_H_ */
