#include <iostream>
#include <cassert>
#include "ThreadPool.hpp"


CompletionToken::CompletionToken()
    : m_count( 0 )
{
    pthread_mutex_init( &m_mutex, NULL );
    pthread_cond_init( &m_complete, NULL );
}

CompletionToken::~CompletionToken()
{
    pthread_mutex_destroy( &m_mutex );
    pthread_cond_destroy( &m_complete );
}


ThreadPool::ThreadPool( int threads )
    : m_done( false )
{
    pthread_mutex_init( &m_mutex, NULL );
    pthread_cond_init( &m_notify, NULL );
    m_workers.resize( threads );
    for(int i=0; i<threads; i++ ) {
        assert( pthread_create( m_workers.data() + i,
                                NULL,
                                workerMain, this ) == 0 );
    }
}

ThreadPool::~ThreadPool()
{
    pthread_mutex_lock( &m_mutex );
    m_done = true;
    pthread_cond_broadcast( &m_notify );
    pthread_mutex_unlock( &m_mutex );

    for(size_t i=0; i<m_workers.size(); i++ ) {
        void* foo;
        assert( pthread_join( m_workers[i], &foo ) == 0 );
    }

    assert( pthread_mutex_destroy( &m_mutex ) == 0);
    assert( pthread_cond_destroy( &m_notify ) == 0);
}

void
ThreadPool::addJob( JobInterface* job, CompletionToken* token )
{
    if( token != NULL ) {
        assert( pthread_mutex_lock( &token->m_mutex ) == 0 );
        token->m_count++;
        assert( pthread_mutex_unlock( &token->m_mutex ) == 0 );
    }

    assert( pthread_mutex_lock( &m_mutex ) == 0 );
    m_jobs.push_back( std::make_tuple(job,token) );
    pthread_cond_signal( &m_notify );
    assert( pthread_mutex_unlock( &m_mutex ) == 0 );
}

void
ThreadPool::wait( CompletionToken* )
{

}


void*
ThreadPool::workerMain( void* arg )
{
    ThreadPool* that = (ThreadPool*)arg;

    std::cerr << "Worker thread initialized.\n";
    assert( pthread_mutex_lock( &that->m_mutex ) == 0 );
    while(1) {
        if( that->m_done ) {
            break;
        }
        if( !that->m_jobs.empty() ) {
            Job job = that->m_jobs.front();
            that->m_jobs.pop_front();
            assert( pthread_mutex_unlock( &that->m_mutex ) == 0 );

            if( std::get<0>( job ) != NULL ) {
                std::get<0>( job )->run();
            }


            assert( pthread_mutex_lock( &that->m_mutex ) == 0 );
        }


        // check for new work item

        assert( pthread_cond_wait( &that->m_notify, &that->m_mutex ) == 0 );
    }
    assert( pthread_mutex_unlock( &that->m_mutex ) == 0 );

    std::cerr << "Worker thread done.\n";

    return NULL;
}
