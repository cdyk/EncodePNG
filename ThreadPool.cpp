#include <iostream>
#include <cassert>
#include "ThreadPool.hpp"

JobInterface:: ~JobInterface()
{
}

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
    m_jobs.emplace_back( job, token );
    pthread_cond_signal( &m_notify );
    assert( pthread_mutex_unlock( &m_mutex ) == 0 );
}

void
ThreadPool::wait( CompletionToken* token )
{
    if( token == NULL ) {
        return;
    }
    
    assert( pthread_mutex_lock( &token->m_mutex ) == 0 );
    while( token->m_count > 0 ) {

        // Not finished, see if there is any work left         
        Job job( NULL, NULL );
        assert( pthread_mutex_lock( &m_mutex ) == 0 );
        for( auto it=m_jobs.begin(); it!=m_jobs.end(); ++it ) {
            if( it->second == token ) {
                job = *it;
                m_jobs.erase( it );
                break;
            }
        }
        assert( pthread_mutex_unlock( &m_mutex ) == 0 );
    
        if( job.second == token ) {
            // yep work, unlock token and do one work item
            assert( pthread_mutex_unlock( &token->m_mutex ) == 0 );
            if( job.first ) {
                job.first->run();
            }
            
            // re-aquire lock and decrease count
            assert( pthread_mutex_lock( &token->m_mutex ) == 0 );
            token->m_count--;
            if( token->m_count < 1 ) {
                assert( pthread_cond_broadcast( &token->m_complete ) == 0 );
            }
        }
        else {
            // nope, no work, some thread already working on it, just wait
            assert( pthread_cond_wait( &token->m_complete, &token->m_mutex) == 0 );
        }
    }    
    assert( pthread_mutex_unlock( &token->m_mutex ) == 0 );
}


void*
ThreadPool::workerMain( void* arg )
{
    ThreadPool* that = (ThreadPool*)arg;

    std::cerr << "Worker thread initialized.\n";
    assert( pthread_mutex_lock( &that->m_mutex ) == 0 );
    while( !that->m_done ) {

        // Check if there is work for me
        if( !that->m_jobs.empty() ) {

            Job job = that->m_jobs.front();
            that->m_jobs.pop_front();
            
            // do work, unlocking thread pool mutex in the mean time
            assert( pthread_mutex_unlock( &that->m_mutex ) == 0 );

            if( job.first ) {
                job.first->run();
            }
            
            // notify
            if( job.second ) {
                assert( pthread_mutex_lock( &job.second->m_mutex ) == 0 );
                job.second->m_count--;
                if( job.second->m_count < 1 ) {
                    assert( pthread_cond_broadcast( &job.second->m_complete ) == 0 );
                }
                assert( pthread_mutex_unlock( &job.second->m_mutex ) == 0 );
            }

            // re-aquire lock
            assert( pthread_mutex_lock( &that->m_mutex ) == 0 );
        }
        else {
            // otherwise, there is currently no work to be done, so we just
            // wait
            assert( pthread_cond_wait( &that->m_notify, &that->m_mutex ) == 0 );
        }
    }
    assert( pthread_mutex_unlock( &that->m_mutex ) == 0 );

    std::cerr << "Worker thread done.\n";

    return NULL;
}
