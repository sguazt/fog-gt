/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file dcs/fgt/vm_allocation_solvers.hpp
 *
 * \brief Solvers for the VM allocation problem
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright 2017 Marco Guazzone (marco.guazzone@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DCS_FGT_VM_ALLOCATION_SOLVERS_HPP
#define DCS_FGT_VM_ALLOCATION_SOLVERS_HPP


#include <cmath>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/exception.hpp>
#include <dcs/fgt/io.hpp>
#include <dcs/fgt/vm_allocation.hpp>
#include <dcs/logging.hpp>
#include <dcs/math/traits/float.hpp>
#include <ilconcert/iloalg.h>
#include <ilconcert/iloenv.h>
#include <ilconcert/iloexpression.h>
#include <ilconcert/ilomodel.h>
#include <ilcp/cp.h>
#include <ilcplex/ilocplex.h>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>


namespace dcs { namespace fgt {

/**
 * \brief Optimal solver for the VM allocation problem.
 */
template <typename RealT>
class optimal_vm_allocation_solver_t
{
public:
    explicit optimal_vm_allocation_solver_t(RealT relative_tolerance = 0,
                                            RealT time_limit = -1)
    : rel_tol_(relative_tolerance),
      time_lim_(time_limit)
    {
    }

    vm_allocation_t<RealT> operator()(const std::vector<std::size_t>& fns, // Holds the identity of FNs in FN' (i.e., fns[i]=k -> FN k \in FN')
                                      const std::vector<std::size_t>& vms, // Holds the identity of VMs in VM' (i.e., vms[i]=k -> VM k \in VM')
                                      const std::vector<std::size_t>& fn_to_fps, // Maps every FN to its FP
                                      const std::vector<std::size_t>& fn_categories, // Maps every FN to its FN category
                                      const std::vector<bool>& fn_power_states, // The power status of each FN
                                      const std::vector<RealT>& fn_cat_min_powers, // The min power consumption of FNs by FN category
                                      const std::vector<RealT>& fn_cat_max_powers, // The max power consumption of FNs by FN category
                                      const std::vector<std::size_t>& vm_to_svcs, // Maps every VM to its service
                                      const std::vector<std::size_t>& svc_cat_vm_categories, // Maps every service category to its VM category, by service category
                                      const std::vector<std::vector<RealT>>& vm_cpu_specs, // The CPU requirement of VMs by VM category
                                      const std::vector<std::vector<RealT>>& vm_ram_specs, // The RAM requirement of VMs by VM category
                                      const std::vector<std::size_t>& svc_to_fps, // Maps every service to its FP
                                      const std::vector<std::size_t>& svc_categories, // Maps every service to its FP
                                      const std::vector<RealT>& svc_cat_max_delays, // QoS delay by service
                                      const std::vector<std::vector<RealT>>& svc_predicted_delays, // Achieved delay by service and number of VMs
                                      const std::vector<std::vector<RealT>>& fp_svc_cat_penalties, // Monetary penalties by FP and service
                                      const std::vector<RealT>& fp_electricity_costs, // Electricty cost (in $/Wh) of each FP
                                      const std::vector<std::vector<RealT>>& fp_fn_cat_asleep_costs, // Cost to power-off a FN by FP and FN category
                                      const std::vector<std::vector<RealT>>& fp_fn_cat_awake_costs) const // Cost to power-on a FN by FP and FN category
                                      //const std::vector<std::vector<std::vector<RealT>>>& fp_to_fp_vm_migration_costs) const // Cost to migrate a VM by source FP, destination FP and VM category
    {
        DCS_DEBUG_TRACE("Finding optimal VM allocation:");
        DCS_DEBUG_TRACE("- Number of FNs: " << fns.size());
        DCS_DEBUG_TRACE("- Number of VMs: " << vms.size());
        DCS_DEBUG_TRACE("- FNs: " << fns);
        DCS_DEBUG_TRACE("- VMs: " << vms);
        DCS_DEBUG_TRACE("- FN to FP Mapping: " << fn_to_fps);
        DCS_DEBUG_TRACE("- FN Categories: " << fn_categories);
        DCS_DEBUG_TRACE("- FN Power States: " << fn_power_states);
        DCS_DEBUG_TRACE("- FN Mininimum Power Consumption by FN Category: " << fn_cat_min_powers);
        DCS_DEBUG_TRACE("- FN Maximum Power Consumption by FN Category: " << fn_cat_max_powers);
        DCS_DEBUG_TRACE("- VM to Service Mapping: " << vm_to_svcs);
        DCS_DEBUG_TRACE("- VM CPU requirements by VM Category and FN Category: " << vm_cpu_specs);
        DCS_DEBUG_TRACE("- VM RAM requirements by VM Category and FN Category: " << vm_ram_specs);
        DCS_DEBUG_TRACE("- Service to FP Mapping: " << svc_to_fps);
        DCS_DEBUG_TRACE("- Service Categories: " << svc_categories);
        DCS_DEBUG_TRACE("- VM Categories by Service Category: " << svc_cat_vm_categories);
        DCS_DEBUG_TRACE("- Service Max Delays by Service Category: " << svc_cat_max_delays);
        DCS_DEBUG_TRACE("- Service Predicted Delays: " << svc_predicted_delays);
        DCS_DEBUG_TRACE("- FP Service Penalties by Service Category: " << fp_svc_cat_penalties);
        DCS_DEBUG_TRACE("- FP Energy Costs: " << fp_electricity_costs);
        DCS_DEBUG_TRACE("- FN On->Off Cost by FP and FN Category: " << fp_fn_cat_asleep_costs);
        DCS_DEBUG_TRACE("- FN Off->On Cost by FP and FN Category: " << fp_fn_cat_awake_costs);
        DCS_DEBUG_TRACE("- Relative Tolerance: " << rel_tol_);
        DCS_DEBUG_TRACE("- Time Limit: " << time_lim_);

        return by_native_cp(fns,
                            vms,
                            fn_to_fps,
                            fn_categories,
                            fn_power_states,
                            fn_cat_min_powers,
                            fn_cat_max_powers,
                            vm_to_svcs,
                            svc_cat_vm_categories,
                            vm_cpu_specs,
                            vm_ram_specs,
                            svc_to_fps,
                            svc_categories,
                            svc_cat_max_delays,
                            svc_predicted_delays,
                            fp_svc_cat_penalties,
                            fp_electricity_costs,
                            fp_fn_cat_asleep_costs,
                            fp_fn_cat_awake_costs);
                            //fp_to_fp_vm_migration_costs);
    }


private:
    vm_allocation_t<RealT> by_native_cp(const std::vector<std::size_t>& fns, // Holds the identity of FNs in FN' (i.e., fns[i]=k -> FN k \in FN')
                                        const std::vector<std::size_t>& vms, // Holds the identity of VMs in VM' (i.e., vms[i]=k -> VM k \in VM')
                                        const std::vector<std::size_t>& fn_to_fps, // Maps every FN to its FP
                                        const std::vector<std::size_t>& fn_categories, // Maps every FN to its FN category
                                        const std::vector<bool>& fn_power_states, // The power status of each FN
                                        const std::vector<RealT>& fn_cat_min_powers, // The min power consumption of FNs by FN category
                                        const std::vector<RealT>& fn_cat_max_powers, // The max power consumption of FNs by FN category
                                        const std::vector<std::size_t>& vm_to_svcs, // Maps every VM to its service
                                        const std::vector<std::size_t>& svc_cat_vm_categories, // Maps every VM to its VM category
                                        const std::vector<std::vector<RealT>>& vm_cpu_specs, // The CPU requirement of VMs by VM category
                                        const std::vector<std::vector<RealT>>& vm_ram_specs, // The RAM requirement of VMs by VM category
                                        const std::vector<std::size_t>& svc_to_fps, // Maps every service to its FP
                                        const std::vector<std::size_t>& svc_categories, // Maps every service to its service category
                                        const std::vector<RealT>& svc_cat_max_delays, // Max tolerated service delays, by service category
                                        const std::vector<std::vector<RealT>>& svc_predicted_delays, // Achieved delay by service and number of VMs
                                        const std::vector<std::vector<RealT>>& fp_svc_cat_penalties, // Monetary penalties by FP and service
                                        const std::vector<RealT>& fp_electricity_costs, // Electricty cost (in $/Wh) of each FP
                                        const std::vector<std::vector<RealT>>& fp_fn_cat_asleep_costs, // Cost to power-off a FN by FP and FN category
                                        const std::vector<std::vector<RealT>>& fp_fn_cat_awake_costs) const // Cost to power-on a FN by FP and FN category
                                        //const std::vector<std::vector<std::vector<RealT>>>& fp_to_fp_vm_migration_costs) const // Cost to migrate a VM by source FP, destination FP and VM category
    {
        vm_allocation_t<RealT> solution;

        std::vector<std::size_t> svcs; // Holds the identity of services in S' (i.e., svcs.count(k)>0 -> service k \in S')

        // Build the services collection
        {
            // Constructs a set to remove any duplicate service from those associated to the input VMs
            //const std::set<std::size_t> svc_set(vm_to_svcs.begin(), vm_to_svcs.end());
            std::set<std::size_t> svc_set;
            for (auto vm : vms)
            {
                svc_set.insert(vm_to_svcs[vm]);
            }

            // Copy the content of the set to the vector
            svcs.assign(svc_set.begin(), svc_set.end());
        }

        const std::size_t nfns = fns.size();
        const std::size_t nvms = vms.size();
        const std::size_t nsvcs = svcs.size();

/*
        // Build the set of VMs by service
        std::vector<std::set<std::size_t>> svc_vms(nsvcs);
        for (std::size_t k = 0; k < nvms; ++k)
        {
            const std::size_t vm = vms[k];
            const std::size_t svc = vm_to_svcs[vm];

            svc_vms[svc].insert(vm);
        }
*/

        // Setting up the optimization model
        try
        {
            // Initialize the Concert Technology app
            IloEnv env;

            IloModel model(env);

            model.setName("Min-Cost Optimization");

            // Decision Variables

            // Variables x_i \in \{0,1\}: 1 if FN i is to be powered on, 0 otherwise.
            IloBoolVarArray x(env, nfns);
            for (std::size_t i = 0; i < nfns; ++i)
            {
                std::ostringstream oss;
                oss << "x[" << i << "]";
                x[i] = IloBoolVar(env, oss.str().c_str());
                model.add(x[i]);
            }

            // Variables y_{ij} \in \{0,1\}: 1 iif VM j is on FN i, 0 otherwise.
            IloArray<IloBoolVarArray> y(env, nfns);
            for (std::size_t i = 0; i < nfns; ++i)
            {
                y[i] = IloBoolVarArray(env, nvms);

                for (std::size_t j = 0 ; j < nvms ; ++j)
                {
                    std::ostringstream oss;
                    oss << "y[" << i << "][" << j << "]";
                    y[i][j] = IloBoolVar(env, oss.str().c_str());
                    model.add(y[i][j]);
                }
            }

            // Decision expressions

            // Expression u_i \in [0,1]: total fraction of CPU of FN i allocated to VMs
            //   u_i = \sum_{j \in VM'} y_{ij}*U_{vmcat(j),fncat(i)}, \forall i \in FN'
            IloArray<IloNumExpr> u(env, nfns);
            for (std::size_t i = 0; i < nfns; ++i)
            {
                const std::size_t fn = fns[i];
                const std::size_t fn_cat = fn_categories[fn];

                u[i] = IloNumExpr(env);

                for (std::size_t j = 0; j < nvms; ++j)
                {
                    const std::size_t vm = vms[j];
                    const std::size_t svc = vm_to_svcs[vm];
                    const std::size_t svc_cat = svc_categories[svc];
                    const std::size_t vm_cat = svc_cat_vm_categories[svc_cat];

                    u[i] += y[i][j]*vm_cpu_specs[vm_cat][fn_cat];
                }

                std::ostringstream oss;
                oss << "u[" << i << "]";
                u[i].setName(oss.str().c_str());

                model.add(u[i]);
            }

/*
            // Expression d_k \in R:
            //   d_k = q_k(\sum_{j \in VM'} (serv(j) == k) \sum_{i \in FN'} y_{ij})
            IloArray<IloNumExpr> d(env, nsvc);
            for (std::size_t k = 0; k < nsvc; ++k)
            {
                d[k] = IloNumExpr(env);

                for (std::size_t j = 0; j < nvms; ++j)
                {
                    const std::size_t vm = vms[j];
                    const std::size_t svc = vm_to_svc[vm],

                    IloIntExpr vm_assigned(env);
                    for (std::size_t i = 0; i < nfns; ++i)
                    {
                        vm_assigned += y[i][j];
                    }

                    d[k] += svc_predicted_delays_aux[k][(svc == k)*vm_assigned];
                }

                std::ostringstream oss;
                oss << "d[" << k << "]";
                d[k].setName(oss.str().c_str());

                model.add(z[i]);
            }
*/

            // Constraints

            std::size_t cc = 0; // Constraint counter

            // A VM cannot be allocated on a powered off FN and the number of
            // VMs allocated on a given FN i cannot exceed the total number of VMs:
            //   \forall i \in FN': \sum_{j \in VM'} y_{ij} \le |VM'|*x_{i}
            ++cc;
            for (std::size_t i = 0; i < nfns; ++i)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << i << "}";

                IloConstraint cons(IloSum(y[i]) <= IloInt(nvms)*x[i]);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }

            // The same VM cannot be allocated to multiple FNs
            //   \forall j \in VM': \sum_{i \in FN'} y_{ij} <= 1
            ++cc;
            for (std::size_t j = 0; j < nvms; ++j)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << j << "}";

                IloIntExpr lhs_expr(env);
                for (std::size_t i = 0; i < nfns; ++i)
                {
                    lhs_expr += y[i][j];
                }

                IloConstraint cons(lhs_expr <= 1);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }

            // Cannot allocate (a fraction of) CPU of a given FN if it is powered off:
            //    \forall i \in FN': u_{i} \le x_{i}
            ++cc;
            for (std::size_t i = 0; i < nfns; ++i)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << i << "}";

                IloConstraint cons(u[i] <= x[i]);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }

/*
            // The amount of CPU allocated to each VM must exactly its CPU
            // requirement:
            //   \forall i \in FN': \sum_{j \in VM'} y_{ij}U_{j,i} == u_{i}
            ++cc;
            for (std::size_t i = 0; i < nfns; ++i)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << i << "}";

                IloNumExpr lhs_expr(env);
                for (std::size_t j = 0; j < nvms; ++j)
                {
                    const std::size_t fn = fns[i];
                    const std::size_t fn_cat = fn_categories[fn];
                    const std::size_t vm = vms[j];
                    //const std::size_t vm_cat = vm_categories[vm];
                    const std::size_t svc = vm_to_svcs[vm];
                    const std::size_t svc_cat = svc_categories[svc];
                    const std::size_t vm_cat = svc_cat_vm_categories[svc_cat];
                    const RealT req = vm_cpu_specs[vm_cat][fn_cat];

                    lhs_expr += y[i][j]*req;
                }

                // Theoretically, we should use an equality constraint but since
                // both LHS and RHS are real values it is probably better to use
                // an inequality constraint.
                // The final result result should be the same since we are
                // minimizing and the larger is u_i the higher is the value of
                // the objective function.
                //
                //IloConstraint cons(lhs_expr == u[i]);
                IloConstraint cons(lhs_expr >= u[i]);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }
*/

            // The fraction of RAM allocated to VMS of a given FN must not
            // exceed the physical RAM of that FN:
            //   \forall i \in FN': \sum_{j \in VM'} y_{ij}M_{j,i)} \le x_{i}
            ++cc;
            for (std::size_t i = 0; i < nfns; ++i)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << i << "}";

                IloNumExpr lhs_expr(env);
                for (std::size_t j = 0; j < nvms; ++j)
                {
                    const std::size_t fn = fns[i];
                    const std::size_t fn_cat = fn_categories[fn];
                    const std::size_t vm = vms[j];
                    const std::size_t svc = vm_to_svcs[vm];
                    const std::size_t svc_cat = svc_categories[svc];
                    const std::size_t vm_cat = svc_cat_vm_categories[svc_cat];
                    const RealT req = vm_ram_specs[vm_cat][fn_cat];

                    lhs_expr += y[i][j]*req;
                }

                IloConstraint cons(lhs_expr <= x[i]);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }

//XXX: currently VM storage requirements are not used
/*
            // The fraction of storage allocated to VMS of a given FN must not
            // exceed the physical storage of that FN:
            //   \forall i \in FN': \sum_{j \in VM'} y_{ij}D_{j,i)} \le x_{i}
            ++cc;
            for (std::size_t i = 0; i < nfns; ++i)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << i << "}";

                IloNumExpr lhs_expr(env);
                for (std::size_t j = 0; j < nvms; ++j)
                {
                    const std::size_t fn = fns[i]
                    const std::size_t fn_cat = fn_categories[i];
                    const std::size_t vm = vms[j];
                    const std::size_t svc = vm_to_svcs[vm];
                    const std::size_t svc_cat = svc_categories[svc];
                    const std::size_t vm_cat = svc_cat_vm_categories[svc_cat];
                    const RealT req = vm_disk_specs[vm_cat][fn_cat];

                    lhs_expr += y[i][j]*req;
                }

                IloConstraint cons(lhs_expr <= x[i]);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }
*/

/*
            // The fraction of storage allocated to VMS of a given FN must not
            // exceed the physical storage of that FN:
            //   \forall j \in VM': z_j = \sum_{i \in FN'} y_{ij}
            ++cc;
            for (std::size_t j = 0; j < nvms; ++j)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << j << "}";

                IloIntExpr lhs_expr(env);
                for (std::size_t i = 0; i < nfns; ++i)
                {
                    lhs_expr += y[i][j];
                }

                IloConstraint cons(lhs_expr == z[j]);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }
*/

            // Set objective
            IloObjective obj;
            {
                // Prepare an auxiliary matrix (a clone of svc_predicted_delays) to use in
                // the objective function
                const std::size_t tot_nsvcs = svc_predicted_delays.size();
                IloArray<IloNumArray> svc_predicted_delays_aux(env, tot_nsvcs);
                //for (std::size_t k = 0; k < nsvcs; ++k)
                for (std::size_t svc = 0; svc < tot_nsvcs; ++svc)
                {
                    //const std::size_t svc = svcs[k];
                    const std::size_t svc_nvms = svc_predicted_delays[svc].size();

                    //svc_predicted_delays_aux[k] = IloNumArray(env, svc_nvms);
                    svc_predicted_delays_aux[svc] = IloNumArray(env, svc_nvms);

                    for (std::size_t j = 0; j < svc_nvms; ++j)
                    {
                        //svc_predicted_delays_aux[k][j] = std::isinf(svc_predicted_delays[k][j]) ? IloInfinity : svc_predicted_delays[k][j];
                        svc_predicted_delays_aux[svc][j] = std::isinf(svc_predicted_delays[svc][j]) ? IloInfinity : svc_predicted_delays[svc][j];
                    }
                }

                // Prepare an auxiliary decision expression used for checking if a VM has been allocated
                IloArray<IloIntExpr> allocated_vms_expr(env, nvms);
                for (std::size_t j = 0; j < nvms; ++j)
                {
                    allocated_vms_expr[j] = IloIntExpr(env);

                    for (std::size_t i = 0; i < nfns; ++i)
                    {
                        allocated_vms_expr[j] += y[i][j];
                    }

                    std::ostringstream oss;
                    oss << "allocated_vms[" << j << "]";
                    allocated_vms_expr[j].setName(oss.str().c_str());
                }

                IloNumExpr obj_expr(env);
                for (std::size_t i = 0; i < nfns; ++i)
                {
                    const std::size_t fn = fns[i];
                    const std::size_t fn_fp = fn_to_fps[fn];
                    const std::size_t fn_cat = fn_categories[fn];
                    const int fn_power_state = fn_power_states[fn];
                    const RealT dC = fn_cat_max_powers[fn_cat]-fn_cat_min_powers[fn_cat];
                    const RealT wcost = fp_electricity_costs[fn_fp];

                    // Add FN electricity costs
                    obj_expr += (x[i]*fn_cat_min_powers[fn_cat]+dC*u[i])*wcost;

                    // Add FN switch-on/off costs
                    obj_expr += x[i]*(1-fn_power_state)*fp_fn_cat_awake_costs[fn_fp][fn_cat]
                             +  (1-x[i])*fn_power_state*fp_fn_cat_asleep_costs[fn_fp][fn_cat];

/*FIXME: VM migration not handled for now
                    // Add VM migration costs
                    for (std::size_t j = 0; j < nvms; ++j)
                    {
                        const std::size_t vm = vms[j];
                        const std::size_t vm_fp = vm_hosted_by_fps[vm]; // Retrieve the FP where this VM is currently hosted
                        const std::size_t svc = vm_to_svcs[vm];
                        const std::size_t svc_cat = svc_categories[svc];
                        const std::size_t vm_cat = svc_cat_vm_categories[svc_cat];

                        obj_expr += y[i][j]*fp_to_fp_vm_migration_costs[vm_fp][fn_fp][vm_cat];
                    }
*/
                }

                // Add SLA violation costs
                for (std::size_t k = 0; k < nsvcs; ++k)
                {
                    const std::size_t svc = svcs[k];
                    const std::size_t fp = svc_to_fps[svc];
                    const std::size_t svc_cat = svc_categories[svc];

                    IloIntExpr num_vms_expr(env);
                    for (std::size_t j = 0; j < nvms; ++j)
                    {
                        const std::size_t vm = vms[j];

                        ////num_vms_expr += allocated_vms_expr[j]*IloBool(svc_vms[svc].count(vm) > 0);
                        //num_vms_expr += allocated_vms_expr[j]*IloBool(vm_to_svcs[vm] == k);
                        num_vms_expr += allocated_vms_expr[j]*IloBool(vm_to_svcs[vm] == svc);
                    }

//DCS_DEBUG_TRACE("Adding SLA costs for service #" << k << ": " << svc << ", fp: "<< fp << ", cat: " << svc_cat);
                    obj_expr += (IloMax(svc_predicted_delays_aux[svc][num_vms_expr]/svc_cat_max_delays[svc_cat], 1.0) - 1.0)*fp_svc_cat_penalties[fp][svc_cat];
                }

                obj = IloMinimize(env, obj_expr);
            }
            model.add(obj);


            // Create the CPLEX solver and make 'model' the active ("extracted") model
            IloCP solver(model);

            //write model
#ifndef DCS_DEBUG
            solver.setOut(env.getNullStream());
            solver.setWarning(env.getNullStream());
#else // DCS_DEBUG
            solver.exportModel("cplex-model.cpo");
            solver.dumpModel("cplex-model_dump.cpo");
#endif // DCS_DEBUG

            // Set Relative Optimality Tolerance to (rel_tol_*100)%: CP will stop as soon as it has found a feasible solution proved to be within (rel_tol_*100)% of optimal.
            if (math::float_traits<RealT>::definitely_greater(rel_tol_, 0))
            {
                //solver.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, rel_tol_);
                solver.setParameter(IloCP::RelativeOptimalityTolerance, rel_tol_);
            }
            if (math::float_traits<RealT>::definitely_greater(time_lim_, 0))
            {
                //solver.setParam(IloCplex::Param::TimeLimit, time_lim_);
                solver.setParameter(IloCP::TimeLimit, time_lim_);
            }

            solver.propagate();
            solution.solved = solver.solve();
            solution.optimal = false;

            IloAlgorithm::Status status = solver.getStatus();
            switch (status)
            {
                case IloAlgorithm::Optimal: // The algorithm found an optimal solution.
                    solution.objective_value = static_cast<RealT>(solver.getObjValue());
                    solution.optimal = true;
                    break;
                case IloAlgorithm::Feasible: // The algorithm found a feasible solution, though it may not necessarily be optimal.

                    solution.objective_value = static_cast<RealT>(solver.getObjValue());
                    ::dcs::log_warn(DCS_LOGGING_AT, "Optimization problem solved but non-optimal");
                    break;
                case IloAlgorithm::Infeasible: // The algorithm proved the model infeasible (i.e., it is not possible to find an assignment of values to variables satisfying all the constraints in the model).
                case IloAlgorithm::Unbounded: // The algorithm proved the model unbounded.
                case IloAlgorithm::InfeasibleOrUnbounded: // The model is infeasible or unbounded.
                case IloAlgorithm::Error: // An error occurred and, on platforms that support exceptions, that an exception has been thrown.
                case IloAlgorithm::Unknown: // The algorithm has no information about the solution of the model.
                {
                    // Possible CP status (obtained with getInfo):
                    // - IloCP::SearchHasNotFailed: indicates that the search has not failed.
                    // - IloCP::SearchHasFailedNormally: indicates that the search has failed because it has searched the entire search space.
                    // - IloCP::SearchStoppedByLimit: indicates that the search was stopped by a limit, such as a time limit (see IloCP::TimeLimit) or a fail limit (see IloCP::FailLimit ).
                    // - IloCP::SearchStoppedByLabel: indicates that the search was stopped via a fail label which did not exist on any choice point (advanced use).
                    // - IloCP::SearchStoppedByExit: indicates that the search was exited using IloCP::exitSearch.
                    // - IloCP::SearchStoppedByAbort: indicates that the search was stopped by calling IloCP::abortSearch.
                    // - IloCP::UnknownFailureStatus: indicates that the search failed for some other reason.
                    std::ostringstream oss;
                    oss << "Optimization was stopped with status = " << status << " (CP status = " << solver.getInfo(IloCP::FailStatus) << ")";
                    dcs::log_warn(DCS_LOGGING_AT, oss.str());
                    return solution;
                }
            }

#ifdef DCS_DEBUG
            DCS_DEBUG_TRACE( "-------------------------------------------------------------------------------[" );
            DCS_DEBUG_TRACE( "- Objective value: " << solution.objective_value );

            DCS_DEBUG_TRACE( "- Decision variables: " );

            // Output x_i
            for (std::size_t i = 0; i < nfns; ++i)
            {
                DCS_DEBUG_STREAM << x[i].getName() << " = " << solver.getValue(x[i]) << " (" << static_cast<int>(solver.getValue(x[i])) << ")" << std::endl;
            }

            // Output y_{ij}
            for (std::size_t i = 0; i < nfns; ++i)
            {
                for (std::size_t j = 0; j < nvms; ++j)
                {
                    DCS_DEBUG_STREAM << y[i][j].getName() << " = " << solver.getValue(y[i][j]) << " (" << static_cast<bool>(solver.getValue(y[i][j])) << ")" << std::endl;
                }
            }

            // Output u_i
            for (std::size_t i = 0; i < nfns; ++i)
            {
                DCS_DEBUG_STREAM << u[i].getName() << " = " << solver.getValue(u[i]) << std::endl;
            }

            DCS_DEBUG_TRACE( "- Derived variables: " );
            // Output d_{i}
            for (std::size_t k = 0; k < nsvcs; ++k)
            {
                std::size_t svc_nvms = 0; // Number of allocated VMs

                for (std::size_t j = 0; j < nvms; ++j)
                {
                    if (vm_to_svcs[j] == k)
                    {
                        for (std::size_t i = 0; i < nfns; ++i)
                        {
                            if (static_cast<bool>(solver.getValue(y[i][j])) != 0)
                            {
                                ++svc_nvms;
                            }
                        }
                    }
                }

                DCS_DEBUG_STREAM << "d[" << k << "] = " << svc_predicted_delays[k][svc_nvms] << std::endl;
            }

            DCS_DEBUG_TRACE( "]-------------------------------------------------------------------------------" );
#endif // DCS_DEBUG

            solution.fn_vm_allocations.resize(nfns);
            solution.fn_power_states.resize(nfns, 0);
            //solution.cost = solution.objective_value;
            //solution.watts = 0;
            for (std::size_t i = 0; i < nfns; ++i)
            {
                //const std::size_t fn = fns[i];

                solution.fn_power_states[i] = static_cast<bool>(solver.getValue(x[i]));
                solution.fn_vm_allocations[i].resize(nvms);
                for (std::size_t j = 0; j < nvms; ++j)
                {
                    //const std::size_t vm = vms[j];

                    solution.fn_vm_allocations[i][j] = static_cast<bool>(solver.getValue(y[i][j]));
                }
            }

            obj.end();
            u.end();
            y.end();
            x.end();

            // Close the Concert Technology app
            env.end();
        }
        catch (const IloException& e)
        {
            std::ostringstream oss;
            oss << "Got exception from CP Optimizer: " << e.getMessage();
            DCS_EXCEPTION_THROW(std::runtime_error, oss.str());
        }
        catch (...)
        {
            DCS_EXCEPTION_THROW(std::runtime_error,
                                "Unexpected error during the optimization");
        }

        return solution;
    }

#if 0 // BEGIN HACK
    vm_allocation_t<RealT> HACK_by_native_cp(const std::vector<std::size_t>& fns, // Holds the identity of FNs in FN' (i.e., fns[i]=k -> FN k \in FN')
                                        const std::vector<std::size_t>& vms, // Holds the identity of VMs in VM' (i.e., vms[i]=k -> VM k \in VM')
                                        const std::vector<std::size_t>& fn_to_fps, // Maps every FN to its FP
                                        const std::vector<std::size_t>& fn_categories, // Maps every FN to its FN category
                                        const std::vector<bool>& fn_power_states, // The power status of each FN
                                        const std::vector<RealT>& fn_cat_min_powers, // The min power consumption of FNs by FN category
                                        const std::vector<RealT>& fn_cat_max_powers, // The max power consumption of FNs by FN category
                                        const std::vector<std::size_t>& vm_to_svcs, // Maps every VM to its service
                                        const std::vector<std::size_t>& svc_cat_vm_categories, // Maps every VM to its VM category
                                        const std::vector<std::vector<RealT>>& vm_cpu_specs, // The CPU requirement of VMs by VM category
                                        const std::vector<std::vector<RealT>>& vm_ram_specs, // The RAM requirement of VMs by VM category
                                        const std::vector<std::size_t>& svc_to_fps, // Maps every service to its FP
                                        const std::vector<std::size_t>& svc_categories, // Maps every service to its service category
                                        const std::vector<RealT>& svc_cat_max_delays, // Max tolerated service delays, by service category
                                        const std::vector<std::vector<RealT>>& svc_predicted_delays, // Achieved delay by service and number of VMs
                                        const std::vector<std::vector<RealT>>& fp_svc_cat_penalties, // Monetary penalties by FP and service
                                        const std::vector<RealT>& fp_electricity_costs, // Electricty cost (in $/Wh) of each FP
                                        const std::vector<std::vector<RealT>>& fp_fn_cat_asleep_costs, // Cost to power-off a FN by FP and FN category
                                        const std::vector<std::vector<RealT>>& fp_fn_cat_awake_costs) const // Cost to power-on a FN by FP and FN category
                                        //const std::vector<std::vector<std::vector<RealT>>>& fp_to_fp_vm_migration_costs) const // Cost to migrate a VM by source FP, destination FP and VM category
    {
DCS_DEBUG_TRACE("BEGIN HACK");

        vm_allocation_t<RealT> solution;

        by_native_cp_phase1(fns,
                            vms,
                            fn_to_fps,
                            fn_categories,
                            fn_power_states,
                            fn_cat_min_powers,
                            fn_cat_max_powers,
                            vm_to_svcs,
                            svc_cat_vm_categories,
                            vm_cpu_specs,
                            vm_ram_specs,
                            svc_to_fps,
                            svc_categories,
                            svc_cat_max_delays,
                            svc_predicted_delays,
                            fp_svc_cat_penalties,
                            fp_electricity_costs,
                            fp_fn_cat_asleep_costs,
                            fp_fn_cat_awake_costs);
                            //fp_to_fp_vm_migration_costs);
        solution = by_native_cp_phase2(fns,
                            vms,
                            fn_to_fps,
                            fn_categories,
                            fn_power_states,
                            fn_cat_min_powers,
                            fn_cat_max_powers,
                            vm_to_svcs,
                            svc_cat_vm_categories,
                            vm_cpu_specs,
                            vm_ram_specs,
                            svc_to_fps,
                            svc_categories,
                            svc_cat_max_delays,
                            svc_predicted_delays,
                            fp_svc_cat_penalties,
                            fp_electricity_costs,
                            fp_fn_cat_asleep_costs,
                            fp_fn_cat_awake_costs);
                            //fp_to_fp_vm_migration_costs);
DCS_DEBUG_TRACE("END HACK");
std::abort();
        return solution;
    }

    // Build and export a model to be solved on phase 2
    vm_allocation_t<RealT> by_native_cp_phase1(const std::vector<std::size_t>& fns, // Holds the identity of FNs in FN' (i.e., fns[i]=k -> FN k \in FN')
                                        const std::vector<std::size_t>& vms, // Holds the identity of VMs in VM' (i.e., vms[i]=k -> VM k \in VM')
                                        const std::vector<std::size_t>& fn_to_fps, // Maps every FN to its FP
                                        const std::vector<std::size_t>& fn_categories, // Maps every FN to its FN category
                                        const std::vector<bool>& fn_power_states, // The power status of each FN
                                        const std::vector<RealT>& fn_cat_min_powers, // The min power consumption of FNs by FN category
                                        const std::vector<RealT>& fn_cat_max_powers, // The max power consumption of FNs by FN category
                                        const std::vector<std::size_t>& vm_to_svcs, // Maps every VM to its service
                                        const std::vector<std::size_t>& svc_cat_vm_categories, // Maps every VM to its VM category
                                        const std::vector<std::vector<RealT>>& vm_cpu_specs, // The CPU requirement of VMs by VM category
                                        const std::vector<std::vector<RealT>>& vm_ram_specs, // The RAM requirement of VMs by VM category
                                        const std::vector<std::size_t>& svc_to_fps, // Maps every service to its FP
                                        const std::vector<std::size_t>& svc_categories, // Maps every service to its service category
                                        const std::vector<RealT>& svc_cat_max_delays, // Max tolerated service delays, by service category
                                        const std::vector<std::vector<RealT>>& svc_predicted_delays, // Achieved delay by service and number of VMs
                                        const std::vector<std::vector<RealT>>& fp_svc_cat_penalties, // Monetary penalties by FP and service
                                        const std::vector<RealT>& fp_electricity_costs, // Electricty cost (in $/Wh) of each FP
                                        const std::vector<std::vector<RealT>>& fp_fn_cat_asleep_costs, // Cost to power-off a FN by FP and FN category
                                        const std::vector<std::vector<RealT>>& fp_fn_cat_awake_costs) const // Cost to power-on a FN by FP and FN category
                                        //const std::vector<std::vector<std::vector<RealT>>>& fp_to_fp_vm_migration_costs) const // Cost to migrate a VM by source FP, destination FP and VM category
    {
        vm_allocation_t<RealT> solution;

        std::vector<std::size_t> svcs; // Holds the identity of services in S' (i.e., svcs.count(k)>0 -> service k \in S')

        // Build the services collection
        {
            // Constructs a set to remove any duplicate service from those associated to the input VMs
            //const std::set<std::size_t> svc_set(vm_to_svcs.begin(), vm_to_svcs.end());
            std::set<std::size_t> svc_set;
            for (auto vm : vms)
            {
                svc_set.insert(vm_to_svcs[vm]);
            }

            // Copy the content of the set to the vector
            svcs.assign(svc_set.begin(), svc_set.end());
        }

        const std::size_t nfns = fns.size();
        const std::size_t nvms = vms.size();
        const std::size_t nsvcs = svcs.size();

/*
        // Build the set of VMs by service
        std::vector<std::set<std::size_t>> svc_vms(nsvcs);
        for (std::size_t k = 0; k < nvms; ++k)
        {
            const std::size_t vm = vms[k];
            const std::size_t svc = vm_to_svcs[vm];

            svc_vms[svc].insert(vm);
        }
*/

        // Setting up the optimization model
        try
        {
            // Initialize the Concert Technology app
            IloEnv env;

            IloModel model(env);

            model.setName("Min-Cost Optimization");

            // Decision Variables

            // Variables x_i \in \{0,1\}: 1 if FN i is to be powered on, 0 otherwise.
            IloBoolVarArray x(env, nfns);
            for (std::size_t i = 0; i < nfns; ++i)
            {
                std::ostringstream oss;
                oss << "x[" << i << "]";
                x[i] = IloBoolVar(env, oss.str().c_str());
                model.add(x[i]);
            }

            // Variables y_{ij} \in \{0,1\}: 1 iif VM j is on FN i, 0 otherwise.
            IloArray<IloBoolVarArray> y(env, nfns);
            for (std::size_t i = 0; i < nfns; ++i)
            {
                y[i] = IloBoolVarArray(env, nvms);

                for (std::size_t j = 0 ; j < nvms ; ++j)
                {
                    std::ostringstream oss;
                    oss << "y[" << i << "][" << j << "]";
                    y[i][j] = IloBoolVar(env, oss.str().c_str());
                    model.add(y[i][j]);
                }
            }

            // Decision expressions

            // Expression u_i \in [0,1]: total fraction of CPU of FN i allocated to VMs
            //   u_i = \sum_{j \in VM'} y_{ij}*U_{vmcat(j),fncat(i)}, \forall i \in FN'
            IloArray<IloNumExpr> u(env, nfns);
            for (std::size_t i = 0; i < nfns; ++i)
            {
                const std::size_t fn = fns[i];
                const std::size_t fn_cat = fn_categories[fn];

                u[i] = IloNumExpr(env);

                for (std::size_t j = 0; j < nvms; ++j)
                {
                    const std::size_t vm = vms[j];
                    const std::size_t svc = vm_to_svcs[vm];
                    const std::size_t svc_cat = svc_categories[svc];
                    const std::size_t vm_cat = svc_cat_vm_categories[svc_cat];

                    u[i] += y[i][j]*vm_cpu_specs[vm_cat][fn_cat];
                }

                std::ostringstream oss;
                oss << "u[" << i << "]";
                u[i].setName(oss.str().c_str());

                model.add(u[i]);
            }

/*
            // Expression d_k \in R:
            //   d_k = q_k(\sum_{j \in VM'} (serv(j) == k) \sum_{i \in FN'} y_{ij})
            IloArray<IloNumExpr> d(env, nsvc);
            for (std::size_t k = 0; k < nsvc; ++k)
            {
                d[k] = IloNumExpr(env);

                for (std::size_t j = 0; j < nvms; ++j)
                {
                    const std::size_t vm = vms[j];
                    const std::size_t svc = vm_to_svc[vm],

                    IloIntExpr vm_assigned(env);
                    for (std::size_t i = 0; i < nfns; ++i)
                    {
                        vm_assigned += y[i][j];
                    }

                    d[k] += svc_predicted_delays_aux[k][(svc == k)*vm_assigned];
                }

                std::ostringstream oss;
                oss << "d[" << k << "]";
                d[k].setName(oss.str().c_str());

                model.add(z[i]);
            }
*/

            // Constraints

            std::size_t cc = 0; // Constraint counter

            // A VM cannot be allocated on a powered off FN and the number of
            // VMs allocated on a given FN i cannot exceed the total number of VMs:
            //   \forall i \in FN': \sum_{j \in VM'} y_{ij} \le |VM'|*x_{i}
            ++cc;
            for (std::size_t i = 0; i < nfns; ++i)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << i << "}";

                IloConstraint cons(IloSum(y[i]) <= IloInt(nvms)*x[i]);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }

            // The same VM cannot be allocated to multiple FNs
            //   \forall j \in VM': \sum_{i \in FN'} y_{ij} <= 1
            ++cc;
            for (std::size_t j = 0; j < nvms; ++j)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << j << "}";

                IloIntExpr lhs_expr(env);
                for (std::size_t i = 0; i < nfns; ++i)
                {
                    lhs_expr += y[i][j];
                }

                IloConstraint cons(lhs_expr <= 1);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }

            // Cannot allocate (a fraction of) CPU of a given FN if it is powered off:
            //    \forall i \in FN': u_{i} \le x_{i}
            ++cc;
            for (std::size_t i = 0; i < nfns; ++i)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << i << "}";

                IloConstraint cons(u[i] <= x[i]);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }

/*
            // The amount of CPU allocated to each VM must exactly its CPU
            // requirement:
            //   \forall i \in FN': \sum_{j \in VM'} y_{ij}U_{j,i} == u_{i}
            ++cc;
            for (std::size_t i = 0; i < nfns; ++i)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << i << "}";

                IloNumExpr lhs_expr(env);
                for (std::size_t j = 0; j < nvms; ++j)
                {
                    const std::size_t fn = fns[i];
                    const std::size_t fn_cat = fn_categories[fn];
                    const std::size_t vm = vms[j];
                    //const std::size_t vm_cat = vm_categories[vm];
                    const std::size_t svc = vm_to_svcs[vm];
                    const std::size_t svc_cat = svc_categories[svc];
                    const std::size_t vm_cat = svc_cat_vm_categories[svc_cat];
                    const RealT req = vm_cpu_specs[vm_cat][fn_cat];

                    lhs_expr += y[i][j]*req;
                }

                // Theoretically, we should use an equality constraint but since
                // both LHS and RHS are real values it is probably better to use
                // an inequality constraint.
                // The final result result should be the same since we are
                // minimizing and the larger is u_i the higher is the value of
                // the objective function.
                //
                //IloConstraint cons(lhs_expr == u[i]);
                IloConstraint cons(lhs_expr >= u[i]);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }
*/

            // The fraction of RAM allocated to VMS of a given FN must not
            // exceed the physical RAM of that FN:
            //   \forall i \in FN': \sum_{j \in VM'} y_{ij}M_{j,i)} \le x_{i}
            ++cc;
            for (std::size_t i = 0; i < nfns; ++i)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << i << "}";

                IloNumExpr lhs_expr(env);
                for (std::size_t j = 0; j < nvms; ++j)
                {
                    const std::size_t fn = fns[i];
                    const std::size_t fn_cat = fn_categories[fn];
                    const std::size_t vm = vms[j];
                    const std::size_t svc = vm_to_svcs[vm];
                    const std::size_t svc_cat = svc_categories[svc];
                    const std::size_t vm_cat = svc_cat_vm_categories[svc_cat];
                    const RealT req = vm_ram_specs[vm_cat][fn_cat];

                    lhs_expr += y[i][j]*req;
                }

                IloConstraint cons(lhs_expr <= x[i]);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }

//XXX: currently VM storage requirements are not used
/*
            // The fraction of storage allocated to VMS of a given FN must not
            // exceed the physical storage of that FN:
            //   \forall i \in FN': \sum_{j \in VM'} y_{ij}D_{j,i)} \le x_{i}
            ++cc;
            for (std::size_t i = 0; i < nfns; ++i)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << i << "}";

                IloNumExpr lhs_expr(env);
                for (std::size_t j = 0; j < nvms; ++j)
                {
                    const std::size_t fn = fns[i]
                    const std::size_t fn_cat = fn_categories[i];
                    const std::size_t vm = vms[j];
                    const std::size_t svc = vm_to_svcs[vm];
                    const std::size_t svc_cat = svc_categories[svc];
                    const std::size_t vm_cat = svc_cat_vm_categories[svc_cat];
                    const RealT req = vm_disk_specs[vm_cat][fn_cat];

                    lhs_expr += y[i][j]*req;
                }

                IloConstraint cons(lhs_expr <= x[i]);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }
*/

/*
            // The fraction of storage allocated to VMS of a given FN must not
            // exceed the physical storage of that FN:
            //   \forall j \in VM': z_j = \sum_{i \in FN'} y_{ij}
            ++cc;
            for (std::size_t j = 0; j < nvms; ++j)
            {
                std::ostringstream oss;
                oss << "C" << cc << "_{" << j << "}";

                IloIntExpr lhs_expr(env);
                for (std::size_t i = 0; i < nfns; ++i)
                {
                    lhs_expr += y[i][j];
                }

                IloConstraint cons(lhs_expr == z[j]);
                cons.setName(oss.str().c_str());
                model.add(cons);
            }
*/

            // Set objective
            IloObjective obj;
            {
                // Prepare an auxiliary matrix (a clone of svc_predicted_delays) to use in
                // the objective function
//                IloArray<IloNumArray> svc_predicted_delays_aux(env, nsvcs);
//                for (std::size_t k = 0; k < nsvcs; ++k)
//                {
//                    const std::size_t svc = svcs[k];
//                    const std::size_t svc_nvms = svc_predicted_delays[svc].size();
//
//                    svc_predicted_delays_aux[k] = IloNumArray(env, svc_nvms);
//
//                    for (std::size_t j = 0; j < svc_nvms; ++j)
//                    {
//                        svc_predicted_delays_aux[k][j] = std::isinf(svc_predicted_delays[k][j]) ? IloInfinity : svc_predicted_delays[k][j];
//                    }
//                }
                const std::size_t tot_nsvcs = svc_predicted_delays.size();
                IloArray<IloNumArray> svc_predicted_delays_aux(env, tot_nsvcs);
                for (std::size_t svc = 0; svc < tot_nsvcs; ++svc)
                {
                    const std::size_t svc_nvms = svc_predicted_delays[svc].size();

                    svc_predicted_delays_aux[svc] = IloNumArray(env, svc_nvms);

                    for (std::size_t j = 0; j < svc_nvms; ++j)
                    {
                        svc_predicted_delays_aux[svc][j] = std::isinf(svc_predicted_delays[svc][j]) ? IloInfinity : svc_predicted_delays[svc][j];
                    }
                }

                // Prepare an auxiliary decision expression used for checking if a VM has been allocated
                IloArray<IloIntExpr> allocated_vms_expr(env, nvms);
                for (std::size_t j = 0; j < nvms; ++j)
                {
                    allocated_vms_expr[j] = IloIntExpr(env);

                    for (std::size_t i = 0; i < nfns; ++i)
                    {
                        allocated_vms_expr[j] += y[i][j];
                    }

                    std::ostringstream oss;
                    oss << "allocated_vms[" << j << "]";
                    allocated_vms_expr[j].setName(oss.str().c_str());
                }

                IloNumExpr obj_expr(env);
                for (std::size_t i = 0; i < nfns; ++i)
                {
                    const std::size_t fn = fns[i];
                    const std::size_t fn_fp = fn_to_fps[fn];
                    const std::size_t fn_cat = fn_categories[fn];
                    const int fn_power_state = fn_power_states[fn];
                    const RealT dC = fn_cat_max_powers[fn_cat]-fn_cat_min_powers[fn_cat];
                    const RealT wcost = fp_electricity_costs[fn_fp];

                    // Add FN electricity costs
                    obj_expr += (x[i]*fn_cat_min_powers[fn_cat]+dC*u[i])*wcost;

                    // Add FN switch-on/off costs
                    obj_expr += x[i]*(1-fn_power_state)*fp_fn_cat_awake_costs[fn_fp][fn_cat]
                             +  (1-x[i])*fn_power_state*fp_fn_cat_asleep_costs[fn_fp][fn_cat];

/*FIXME: VM migration not handled for now
                    // Add VM migration costs
                    for (std::size_t j = 0; j < nvms; ++j)
                    {
                        const std::size_t vm = vms[j];
                        const std::size_t vm_fp = vm_hosted_by_fps[vm]; // Retrieve the FP where this VM is currently hosted
                        const std::size_t svc = vm_to_svcs[vm];
                        const std::size_t svc_cat = svc_categories[svc];
                        const std::size_t vm_cat = svc_cat_vm_categories[svc_cat];

                        obj_expr += y[i][j]*fp_to_fp_vm_migration_costs[vm_fp][fn_fp][vm_cat];
                    }
*/
                }

                // Add SLA violation costs
                for (std::size_t k = 0; k < nsvcs; ++k)
                {
                    const std::size_t svc = svcs[k];
                    const std::size_t fp = svc_to_fps[svc];
                    const std::size_t svc_cat = svc_categories[svc];

                    IloIntExpr num_vms_expr(env);
                    for (std::size_t j = 0; j < nvms; ++j)
                    {
                        const std::size_t vm = vms[j];

                        ////num_vms_expr += allocated_vms_expr[j]*IloBool(svc_vms[svc].count(vm) > 0);
                        //num_vms_expr += allocated_vms_expr[j]*IloBool(vm_to_svcs[vm] == k);
                        num_vms_expr += allocated_vms_expr[j]*IloBool(vm_to_svcs[vm] == svc);
                    }

//DCS_DEBUG_TRACE("Adding SLA costs for service #" << k << ": " << svc << ", fp: "<< fp << ", cat: " << svc_cat);
                    obj_expr += (IloMax(svc_predicted_delays_aux[svc][num_vms_expr]/svc_cat_max_delays[svc_cat], 1.0) - 1.0)*fp_svc_cat_penalties[fp][svc_cat];
                }

                obj = IloMinimize(env, obj_expr);
            }
            model.add(obj);


            // Create the CPLEX solver and make 'model' the active ("extracted") model
            IloCP solver(model);

            //write model
#ifndef DCS_DEBUG
            solver.setOut(env.getNullStream());
            solver.setWarning(env.getNullStream());
#else // DCS_DEBUG
            solver.exportModel("cplex-model.cpo");
            solver.dumpModel("cplex-model_dump.cpo");
#endif // DCS_DEBUG

            obj.end();
            u.end();
            y.end();
            x.end();

            // Close the Concert Technology app
            env.end();
        }
        catch (const IloException& e)
        {
            std::ostringstream oss;
            oss << "Got exception from CP Optimizer: " << e.getMessage();
            DCS_EXCEPTION_THROW(std::runtime_error, oss.str());
        }
        catch (...)
        {
            DCS_EXCEPTION_THROW(std::runtime_error,
                                "Unexpected error during the optimization");
        }

        return solution;
    }

    // Load the model exported on phase 1 and solve
    vm_allocation_t<RealT> by_native_cp_phase2(const std::vector<std::size_t>& fns, // Holds the identity of FNs in FN' (i.e., fns[i]=k -> FN k \in FN')
                                        const std::vector<std::size_t>& vms, // Holds the identity of VMs in VM' (i.e., vms[i]=k -> VM k \in VM')
                                        const std::vector<std::size_t>& fn_to_fps, // Maps every FN to its FP
                                        const std::vector<std::size_t>& fn_categories, // Maps every FN to its FN category
                                        const std::vector<bool>& fn_power_states, // The power status of each FN
                                        const std::vector<RealT>& fn_cat_min_powers, // The min power consumption of FNs by FN category
                                        const std::vector<RealT>& fn_cat_max_powers, // The max power consumption of FNs by FN category
                                        const std::vector<std::size_t>& vm_to_svcs, // Maps every VM to its service
                                        const std::vector<std::size_t>& svc_cat_vm_categories, // Maps every VM to its VM category
                                        const std::vector<std::vector<RealT>>& vm_cpu_specs, // The CPU requirement of VMs by VM category
                                        const std::vector<std::vector<RealT>>& vm_ram_specs, // The RAM requirement of VMs by VM category
                                        const std::vector<std::size_t>& svc_to_fps, // Maps every service to its FP
                                        const std::vector<std::size_t>& svc_categories, // Maps every service to its service category
                                        const std::vector<RealT>& svc_cat_max_delays, // Max tolerated service delays, by service category
                                        const std::vector<std::vector<RealT>>& svc_predicted_delays, // Achieved delay by service and number of VMs
                                        const std::vector<std::vector<RealT>>& fp_svc_cat_penalties, // Monetary penalties by FP and service
                                        const std::vector<RealT>& fp_electricity_costs, // Electricty cost (in $/Wh) of each FP
                                        const std::vector<std::vector<RealT>>& fp_fn_cat_asleep_costs, // Cost to power-off a FN by FP and FN category
                                        const std::vector<std::vector<RealT>>& fp_fn_cat_awake_costs) const // Cost to power-on a FN by FP and FN category
                                        //const std::vector<std::vector<std::vector<RealT>>>& fp_to_fp_vm_migration_costs) const // Cost to migrate a VM by source FP, destination FP and VM category
    {
        vm_allocation_t<RealT> solution;

        std::vector<std::size_t> svcs; // Holds the identity of services in S' (i.e., svcs.count(k)>0 -> service k \in S')

        // Build the services collection
        {
            // Constructs a set to remove any duplicate service from those associated to the input VMs
            const std::set<std::size_t> svc_set(vm_to_svcs.begin(), vm_to_svcs.end());

            // Copy the content of the set to the vector
            svcs.assign(svc_set.begin(), svc_set.end());
        }

        const std::size_t nfns = fns.size();
        const std::size_t nvms = vms.size();
        const std::size_t nsvcs = svcs.size();

        try
        {
            IloEnv env;
            IloCP solver(env);
            solver.importModel("cplex-model.cpo");
            solution.solved = solver.solve();
            solution.optimal = false;

            IloAlgorithm::Status status = solver.getStatus();
            switch (status)
            {
                case IloAlgorithm::Optimal: // The algorithm found an optimal solution.
                    solution.objective_value = static_cast<RealT>(solver.getObjValue());
                    solution.optimal = true;
                    break;
                case IloAlgorithm::Feasible: // The algorithm found a feasible solution, though it may not necessarily be optimal.

                    solution.objective_value = static_cast<RealT>(solver.getObjValue());
                    ::dcs::log_warn(DCS_LOGGING_AT, "Optimization problem solved but non-optimal");
                    break;
                case IloAlgorithm::Infeasible: // The algorithm proved the model infeasible (i.e., it is not possible to find an assignment of values to variables satisfying all the constraints in the model).
                case IloAlgorithm::Unbounded: // The algorithm proved the model unbounded.
                case IloAlgorithm::InfeasibleOrUnbounded: // The model is infeasible or unbounded.
                case IloAlgorithm::Error: // An error occurred and, on platforms that support exceptions, that an exception has been thrown.
                case IloAlgorithm::Unknown: // The algorithm has no information about the solution of the model.
                {
                    // Possible CP status (obtained with getInfo):
                    // - IloCP::SearchHasNotFailed: indicates that the search has not failed.
                    // - IloCP::SearchHasFailedNormally: indicates that the search has failed because it has searched the entire search space.
                    // - IloCP::SearchStoppedByLimit: indicates that the search was stopped by a limit, such as a time limit (see IloCP::TimeLimit) or a fail limit (see IloCP::FailLimit ).
                    // - IloCP::SearchStoppedByLabel: indicates that the search was stopped via a fail label which did not exist on any choice point (advanced use).
                    // - IloCP::SearchStoppedByExit: indicates that the search was exited using IloCP::exitSearch.
                    // - IloCP::SearchStoppedByAbort: indicates that the search was stopped by calling IloCP::abortSearch.
                    // - IloCP::UnknownFailureStatus: indicates that the search failed for some other reason.
                    std::ostringstream oss;
                    oss << "Optimization was stopped with status = " << status << " (CP status = " << solver.getInfo(IloCP::FailStatus) << ")";
                    dcs::log_warn(DCS_LOGGING_AT, oss.str());
                    return solution;
                }
            }

            solution.fn_vm_allocations.resize(nfns);
            solution.fn_power_states.resize(nfns, 0);
            //solution.cost = solution.objective_value;
            //solution.watts = 0;
            for (std::size_t i = 0; i < nfns; ++i)
            {
                //const std::size_t fn = fns[i];

                std::ostringstream oss;
                oss << "x[" << i << "]";
                solution.fn_power_states[i] = static_cast<bool>(solver.getValue(solver.getIloIntVar(oss.str().c_str())));
                solution.fn_vm_allocations[i].resize(nvms);
                for (std::size_t j = 0; j < nvms; ++j)
                {
                    //const std::size_t vm = vms[j];

                    oss.str("");
                    oss << "y[" << i << "][" << j << "]";
                    solution.fn_vm_allocations[i][j] = static_cast<bool>(solver.getValue(solver.getIloIntVar(oss.str().c_str())));
                }
            }
            env.end();
        }
        catch (const IloException& e)
        {
            std::ostringstream oss;
            oss << "Got exception from CP Optimizer: " << e.getMessage();
            DCS_EXCEPTION_THROW(std::runtime_error, oss.str());
        }
        catch (...)
        {
            DCS_EXCEPTION_THROW(std::runtime_error,
                                "Unexpected error during the optimization");
        }

        return solution;
    }
#endif // END HACK


private:
    RealT rel_tol_; ///< Relative optimality tolerance used to define optimality (a solution is considered optimal if there does not exist a solution with a better objective function with respect to a relative optimality tolerance).
    RealT time_lim_; ///< Time limit (in seconds) used to set the maximum time the optimizare can spend in search for the best solution.
}; // optimal_vm_alllocation_solver

}} // Namespace dcs::fgt


#endif // DCS_FGT_VM_ALLOCATION_SOLVERS_HPP
