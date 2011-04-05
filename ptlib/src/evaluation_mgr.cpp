/*
 * evaluation_mgr.cpp
 *
 *  Created on: Mar 20, 2011
 *      Author: pawel
 */

#include "evaluation_mgr.h"

#include <boost/bind.hpp>

namespace ptlib { namespace parallel {

evaluation_mgr * evaluation_mgr::m_p_instance = NULL;

void
evaluation_mgr::init(unsigned threads_num_)
{
	if (m_p_instance == NULL)
		m_p_instance = new evaluation_mgr(threads_num_);
}

void
evaluation_mgr::close()
{
	delete m_p_instance;
}

void
evaluation_mgr::add_for_evaluation(deferred_expression_base * const p_def_exp_)
{
	{// CRITICAL_SECTION
		boost::lock_guard<boost::mutex> lock(m_p_instance->m_tasks_mutex);
		m_p_instance->m_tasks.push(std::unique_ptr<deferred_expression_base>(p_def_exp_));
	}
	m_p_instance->m_threads_cond.notify_one();
}

evaluation_mgr::evaluation_mgr(unsigned threads_num_) :
		m_working_threads(0)
{
	m_threads.reserve(threads_num_);
	for (unsigned i = 0; i < threads_num_; i++)
		m_threads[i] = boost::thread(&evaluation_mgr::evaluation_loop_wrapper);
}

evaluation_mgr::~evaluation_mgr()
{}

bool boo() {return false;}

void
evaluation_mgr::evaluation_loop()
{
	std::cout << "Entering evaluation loop" << std::endl;
	while(m_continue)
	{
		task_ptr_type p_task;
		{// CRITICAL_SECTION
			boost::mutex::scoped_lock lock(m_tasks_mutex);
			if (m_foo.empty())
				m_threads_cond.wait(lock, boost::bind(&evaluation_mgr::if_stop_waiting, this));
			if (!m_continue)
				break;
			else // m_tasks is not empty
			{
				p_task = std::move(m_tasks.front());
				m_tasks.pop();
			}
		}
		std::cout << "Evaluation from eval loop" << std::endl;
		p_task->evaluate();
	}
}

} }
