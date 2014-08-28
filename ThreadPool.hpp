#pragma once
#include <pthread.h>
#include <vector>
#include <list>
#include <tuple>


class JobInterface
{
public:
    virtual
    ~JobInterface();

    virtual
    void
    run() = 0;
};

class CompletionToken
{
    friend class ThreadPool;
public:
    explicit CompletionToken();

    ~CompletionToken();

protected:
    int             m_count;
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_complete;
};

class ThreadPool
{
public:

    ThreadPool( int threads = 5 );

    ~ThreadPool();

    void
    addJob( JobInterface* job, CompletionToken* token );

    void
    wait( CompletionToken* );

    int
    workers() const { return m_workers.size(); }

protected:
    typedef std::pair<JobInterface*, CompletionToken*> Job;

    bool                    m_done;
    pthread_mutex_t         m_mutex;
    pthread_cond_t          m_notify;
    std::vector<pthread_t>  m_workers;
    std::list< Job >        m_jobs;

    static
    void*
    workerMain( void* );


};
