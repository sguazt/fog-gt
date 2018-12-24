/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file dcs/fgt/simulator.hpp
 *
 * \brief Simple discrete-event simulator.
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

#ifndef DCS_FGT_SIMULATOR_HPP
#define DCS_FGT_SIMULATOR_HPP


#include <cstddef>
#include <memory>
#include <queue>
#include <vector>


namespace dcs { namespace fgt {

struct event_state_t
{
	virtual ~event_state_t() { }
}; // event_state_t

template <typename RealT>
struct event_t
{
	event_t(RealT fire_time, int tag)
	: fire_time(fire_time),
	  tag(tag)
	{
	}

	event_t(RealT fire_time, int tag, const std::shared_ptr<event_state_t>& p_state)
	: fire_time(fire_time),
	  tag(tag),
	  p_state(p_state)
	{
	}

	RealT fire_time;
	int tag;
	std::shared_ptr<event_state_t> p_state;
}; // event_t


/*
template <typename RealT>
struct simulation_context_t
{
	explicit simulation_context_t(std::size_t num_svcs = 0)
	: num_svcs(num_svcs),
	  svc_workload_bursts(num_svcs)
	{
	}

	void add_workload_burst(std::size_t service, RealT stop_time, RealT arr_rate)
	{
		assert( service < num_svcs );
		svc_workload_bursts[service].push_back(std::make_pair(stop_time, arr_rate));
	}

	void reset()
	{
		svc_workload_bursts.clear();
		svc_workload_bursts.resize(num_svcs);
	}

	std::size_t num_svcs; ///< Number of services
	std::vector<std::vector<std::pair<RealT,RealT>>> svc_workload_bursts; ///< Map each service to the sequence of workload bursts the service saw between two activations of the coalition formation algorithm (service -> [<stop time,rate>, ..., <stop time,rate>])
};
*/

template <typename RealT>
class simulator_t
{
public:
    explicit simulator_t(RealT replication_duration = 0)
    : max_rep_len_(replication_duration),
      max_num_rep_(std::numeric_limits<std::size_t>::max()),
      sim_time_(0),
      done_(false)
    {
    }

    virtual ~simulator_t()
    {
        // Empty
    }

//    void reset()
//    {
//        evt_queue_.clear();
//    }

    void schedule_event(RealT time, int tag, const std::shared_ptr<event_state_t>& p_state = nullptr)
    {
        DCS_DEBUG_TRACE("Scheduling event: <tag: " << tag << ", time: " << time << "> (time: " << sim_time_ << ")");

        auto p_event = std::make_shared<event_t<RealT>>(time, tag, p_state);
        evt_queue_.push(p_event);
    }

    void run()
    {
        initialize_simulation();

        while (!check_end_of_simulation())
        {
            initialize_replication();

            while (!check_end_of_replication())
            {
                fire_event();
            }

            finalize_replication();

            check_end_of_simulation();
        }

        finalize_simulation();
    }

    void max_replication_duration(RealT value)
    {
        max_rep_len_ = value;
    }

    RealT max_replication_duration() const
    {
        return max_rep_len_;
    }

    void max_num_replications(std::size_t value)
    {
        max_num_rep_ = value;
    }

    std::size_t max_num_replications() const
    {
        return max_num_rep_;
    }

    RealT simulated_time() const
    {
        return sim_time_;
    }

    std::size_t num_replications() const
    {
        return num_rep_;
    }

    bool done() const
    {
        return done_;
    }
/*
        done_ = do_end_of_simulation();
        for (auto const& stat: stats_)
        {
            if (!stat->done() && !stat->unstable())
            {
                return false;
            }
        }

        done_ = true;

        return true;
    }
*/


protected:
    virtual void do_initialize_simulation() = 0;

    virtual void do_finalize_simulation() = 0;

    virtual void do_initialize_replication() = 0;

    virtual void do_finalize_replication() = 0;

    virtual bool do_check_end_of_replication() const = 0;

    virtual bool do_check_end_of_simulation() const = 0;

    virtual void do_process_event(const std::shared_ptr<event_t<RealT>>& p_event) = 0;

private:
    void initialize_simulation()
    {
        DCS_DEBUG_TRACE("Initializing simulation (time: " << sim_time_ << ")");

        num_rep_ = 0;
        sim_time_ = 0;
        done_ = false;

        do_initialize_simulation();
    }

    void finalize_simulation()
    {
        DCS_DEBUG_TRACE("Finalizing simulation (time: " << sim_time_ << ")");

        done_ = true;

        do_finalize_simulation();
    }

    void initialize_replication()
    {
        DCS_DEBUG_TRACE("Initializing replication #" << (num_rep_+1) << " (time: " << sim_time_ << ")");

        ++num_rep_;
        sim_time_ = 0;

        while (!evt_queue_.empty())
        {
            evt_queue_.pop();
        }

        do_initialize_replication();
    }

    void finalize_replication()
    {
        DCS_DEBUG_TRACE("Finalizing replication #" << num_rep_ << " (time: " << sim_time_ << ")");

        do_finalize_replication();
    }

    bool check_end_of_replication() const
    {
        return sim_time_ >= max_rep_len_ || evt_queue_.empty() || do_check_end_of_replication();
    }

    bool check_end_of_simulation() const
    {
        return done_ || num_rep_ >= max_num_rep_ || do_check_end_of_simulation();
    }

    void fire_event()
    {
        if (evt_queue_.size() > 0)
        {
            auto p_event = evt_queue_.top();
            evt_queue_.pop();
            sim_time_ = p_event->fire_time;

            DCS_DEBUG_TRACE("Firing event: <tag: " << p_event->tag << ", fire-time: " << p_event->fire_time << "> (time: " << sim_time_ << ")");

            do_process_event(p_event);
        }
    }


private:
    struct event_comparator_t
    {
        constexpr bool operator()(const std::shared_ptr<event_t<double>>& e1, const std::shared_ptr<event_t<double>>& e2) const
        {
            return e1->fire_time > e2->fire_time;
        }
    };

private:
    RealT max_rep_len_;
    std::size_t max_num_rep_;
    std::size_t num_rep_;
    RealT sim_time_;
    bool done_;
	std::priority_queue<std::shared_ptr<event_t<RealT>>, std::vector<std::shared_ptr<event_t<RealT>>>, event_comparator_t> evt_queue_;
}; // simulator_t

}} // Namespace dcs::fgt


#endif // DCS_FGT_SIMULATOR_HPP
