#ifndef __ASYNC_CONFIG_HPP__
#define __ASYNC_CONFIG_HPP__

#include <cstddef>
#include <atomic>

namespace kors::async {
    struct Config {
        //! Total number of threads in the application
        //! that can interact through this infrastructure
        static size_t MAX_THREADS;

        //! NOTE The default value for the maximum number of threads
        //! a single channel instance can communicate in.
        //! A different value can be specified for a specific channel,
        //! but no more than MAX_THREADS.
        static size_t MAX_THREADS_PER_CHANNEL;

        //! NOTE The queue capacity, if there are more unprocessed messages,
        //! they will not be lost, but will be sent to the next process
        static size_t QUEUE_CAPACITY;

        //! NOTE Should be wait send pending messages on send
        static bool DO_WAIT_PENDINGS_ON_SEND;
        //! NOTE Should be warning on send pending messages timeout
        static bool DO_WARN_ON_PENDINGSSEND_TIMEOUT;
        //! NOTE Waiting time for sending pending messages
        static size_t WAIT_PENDINGS_MS;
        //! NOTE Maximum number of attempts to send pending messages
        static size_t MAX_SEND_PENDINGS_ATTEMPTS;

        //! NOTE Should be trigger assert on implicit subscription replace
        static bool DO_ASSERT_ON_IMPLICIT_REPLACE;

        //! NOTE When closing an application, we need to terminate.
        //! During the shutdown, various objects are destroyed, from different threads,
        //! especially if they are static objects—they may access a destroyed or non-functioning queue.
        static std::atomic<bool> terminated;
    };
}

#endif
