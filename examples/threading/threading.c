#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    //return thread_param;
    // Cast the parameter to thread_data structure
    struct thread_data* thread_func_args = (struct thread_data*)thread_param;

    // Hold on for x us before obtaining the mutex
    usleep(thread_func_args->wait_to_obtain_ms * 1000); // Convert ms to us
    if (pthread_mutex_lock(thread_func_args->mutex) != 0) {
        ERROR_LOG("Failed to obtain mutex");
        thread_func_args->thread_complete_success = false;
        return NULL;
    }

    // Wait before releasing the held mutex
    usleep(thread_func_args->wait_to_release_ms * 1000); // Convert ms to us

    // Release the mutex
    if (pthread_mutex_unlock(thread_func_args->mutex) != 0) {
        ERROR_LOG("Failed to release mutex");
        thread_func_args->thread_complete_success = false;
        return NULL;
    }

    thread_func_args->thread_complete_success = true; // Mark the thread as successfully completed
    return NULL;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data* thread_data_alloc = malloc(sizeof(struct thread_data));
    if (!thread_data_alloc) {
        ERROR_LOG("Failed to allocate memory for thread_data");
        return false;
    }

    // Setting up the thread data
    thread_data_alloc->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_data_alloc->wait_to_release_ms = wait_to_release_ms;
    thread_data_alloc->mutex = mutex;
    thread_data_alloc->thread_complete_success = false;

    // Creating the thread
    if (pthread_create(thread, NULL, threadfunc, (void*)thread_data_alloc) != 0) {
        ERROR_LOG("Failed to create thread");
        free(thread_data_alloc);
        return false;
    }

    return true;
}

