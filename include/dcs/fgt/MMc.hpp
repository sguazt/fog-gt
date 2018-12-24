/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file dcs/fgt/MMc.hpp
 *
 * \brief Model for an M/M/c queue.
 *
 * <hr/>
 *
 * Copyright 2017 Paolo Castagno (castagno@di.unito.it)
 *                Marco Guazzone (marco.guazzone@gmail.com)
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

#ifndef DCS_FGT_MMC_HPP
#define DCS_FGT_MMC_HPP


#include <cfloat>
#include <cmath>
#include <dcs/assert.hpp>
#include <dcs/exception.hpp>
#include <dcs/math/traits/float.hpp>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


namespace dcs { namespace fgt {

template <typename RealT>
class MMc
{
public:
        MMc(RealT lambda, RealT mu, RealT delay_max, RealT precision = std::numeric_limits<RealT>::epsilon());
        int computeQueueParameters(bool iterative);
        void getDelays(std::vector<RealT>* vec);
        RealT getDelay(std::size_t c);

private:
        RealT lambda;
        RealT mu;
        RealT d_max;
        RealT prec;
        std::vector<RealT> delay;
        RealT recursiveErlang(int k, RealT rho);
        void solveErlangC(int c, std::pair<RealT,RealT>*);
        void Erlang(int c, std::pair<RealT,RealT>*);
};



template <typename RealT>
MMc<RealT>::MMc(RealT lambda, RealT mu, RealT delay_max, RealT precision)
{
    this -> lambda = lambda;
    this ->  mu = mu;
    this -> prec = precision;
    if ( dcs::math::float_traits<RealT>::definitely_less(delay_max, 1.0 / mu, prec) )
    {
        std::cout << "Unfeasable minimum delay. Setting minimum delay equal to station service time, i.e. no queue is admitted. \n";
    delay_max = 1.0 / mu;
    }
    this -> d_max = delay_max;
}

template <typename RealT>
int MMc<RealT>::computeQueueParameters(bool iterative)
{
    RealT delay = std::numeric_limits<RealT>::infinity();
    int n = 0;
    while(dcs::math::float_traits<RealT>::definitely_greater(delay, this -> d_max, prec))
    {
        n++;
        std::pair<RealT,RealT> tmp = std::make_pair(0.0,0.0);
        if(iterative)
            Erlang(n,&tmp);
        else
            solveErlangC(n,&tmp);
        delay = tmp.second;
        this -> delay.push_back(tmp.second);
    }
    return n;
}

template <typename RealT>
void MMc<RealT>::Erlang(int c, std::pair<RealT,RealT>* ret)
{
    RealT rho = this -> lambda / ( this -> mu );
    if( dcs::math::float_traits<RealT>::essentially_greater_equal(rho / c, 1.0, prec) )
    {
        ret -> first = 1.0;
        ret -> second = std::numeric_limits<RealT>::infinity();
    }
    else
    {
        RealT pb = 1;
        for(int j = 1; j <= c; j++)
        {
            pb = ( rho * pb )/ ( j + pb * rho);
        }
        rho = rho / c;
        ret -> first = pb / (1 - rho + (rho * pb));
        ret -> second = ret -> first / ( c * this -> mu - this -> lambda ) + ( 1.0 / mu );
        //std::cout << "Pq : " << ret -> first << " delay " << ret -> second << std::endl;
    }
}

//Compute the term c!/(c*rho) and the summation of the queuing probability formula 
template <typename RealT>
RealT MMc<RealT>::recursiveErlang(int c, RealT a)
{
    RealT e_last;
    if (c == 0)
        return(1.0);
    else
    {
        e_last = recursiveErlang(c - 1, a);
        return((a * e_last) / (c + a * e_last));
    }
}

//Compute performance delay of the Station
template <typename RealT>
void MMc<RealT>::solveErlangC(int c, std::pair<RealT,RealT>* ret)
{
    RealT rho = this -> lambda / ( this -> mu); 
    if( dcs::math::float_traits<RealT>::essentially_greater_equal(rho / c, 1.0, prec) )
    {
        ret -> first = 1.0;
        ret -> second = std::numeric_limits<RealT>::infinity();
    }
    else
    {
        RealT pb = recursiveErlang( c, rho);
        rho = rho / c;
        ret -> first = pb / (1 - rho + (rho * pb));
        ret -> second = ret -> first / ( c * this -> mu  - this -> lambda) + ( 1.0 / mu );
        //std::cout << "Pq : " << ret -> first << " delay " << ret -> second << std::endl;
    }
}

template <typename RealT>
void MMc<RealT>::getDelays(std::vector<RealT>* vec)
{
    // check: queue parameters have been already computed
    DCS_ASSERT(delay.size() > 0,
               DCS_EXCEPTION_THROW(std::logic_error, "Queue parameters have not been computed yet"));

    //vec -> reserve(delay.size());
    //std::copy(std::begin(delay),std::end(delay),std::back_inserter(*vec) );
    vec -> reserve(delay.size()+1);
    vec->push_back(std::numeric_limits<RealT>::infinity());
    std::copy(std::begin(delay),std::end(delay),std::back_inserter(*vec) );
}

template <typename RealT>
RealT MMc<RealT>::getDelay(std::size_t c)
{
    // check: queue parameters have been already computed
    DCS_ASSERT(delay.size() > 0,
               DCS_EXCEPTION_THROW(std::logic_error, "Queue parameters have not been computed yet"));
    DCS_ASSERT(c >= 0,
               DCS_EXCEPTION_THROW(std::logic_error, "Number of stations must be greater than or equal to zero"));

    if (c == 0)
    {
        return std::numeric_limits<RealT>::infinity();
    }
    else if (c > delay.size())
    {
        c = delay.size();
    }

    return delay[c-1];
}

}} // Namespace dcs::fgt


#endif // MMC_H
