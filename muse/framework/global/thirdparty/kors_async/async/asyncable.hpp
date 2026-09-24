#ifndef __ASYNCABLE_HPP__
#define __ASYNCABLE_HPP__

#include <set>
#include <thread>
#include <mutex>
#include <cassert>
#include <algorithm>

namespace kors::async {
    class Asyncable {
    public:
        enum class Mode {
            SetOnce = 0,
            SetReplace = 1
        };

        virtual ~Asyncable() {
            async_disconnectAll();
        }

        struct IConnectable {
            std::set<Asyncable*> asyncables;
            virtual ~IConnectable() {
                assert(asyncables.empty());
            }

            virtual void disconnectAsyncable(Asyncable* async, const std::thread::id& connectThreadId) = 0;
        };

        struct ConnectData {
            std::thread::id threadId;
            IConnectable* connection = nullptr;

            bool operator<(const ConnectData& other) const {
                return connection < other.connection;
            }
        };

        bool async_isConnected() const {
            std::scoped_lock lock(m_async_mutex);
            return !m_async_connects.empty();
        }

        bool async_isConnected(IConnectable* connect) const {
            return async_connectData(connect).connection != nullptr;
        }

        std::thread::id async_connectThread(IConnectable* connect) const {
            return async_connectData(connect).threadId;
        }

        void async_connect(IConnectable *connect) {
            assert(connect);
            if (!connect) {
                return;
            }

            const std::thread::id threadId = std::this_thread::get_id();
            std::scoped_lock lock(m_async_mutex);
            auto it = std::find_if(m_async_connects.begin(), m_async_connects.end(), [connect](const ConnectData& data)
                                   { return data.connection == connect; });

            if (it != m_async_connects.end()) {
                assert(it->threadId == threadId && "more than one connection in different threads");
                return;
            }

            m_async_connects.emplace(ConnectData{threadId, connect});
            connect->asyncables.insert(this);
        }

        void async_disconnect(IConnectable* connect) {
            std::scoped_lock lock(m_async_mutex);
            auto it = std::find_if(m_async_connects.begin(), m_async_connects.end(), [connect](const ConnectData &data)
                                   { return data.connection == connect; });

            if (it != m_async_connects.end()) {
                m_async_connects.erase(it);
                connect->asyncables.erase(this);
            }
        }

        void async_disconnectAll() {
            std::set<ConnectData> copy; {
                std::scoped_lock lock(m_async_mutex);
                m_async_connects.swap(copy);
            }

            for (const ConnectData& data : copy) {
                data.connection->disconnectAsyncable(this, data.threadId);
            }

            {
                std::scoped_lock lock(m_async_mutex);
                for (const ConnectData& data : copy) {
                    data.connection->asyncables.erase(this);
                }
            }
        }

        const ConnectData &async_connectData(IConnectable* connect) const {
            std::scoped_lock lock(m_async_mutex);
            auto iterator = std::find_if(m_async_connects.begin(), m_async_connects.end(), [connect](const ConnectData &data)
                                         { return data.connection == connect; });

            if (iterator != m_async_connects.end()) {
                return *iterator;
            }

            static ConnectData dummy;
            return dummy;
        }

    private:
        mutable std::mutex m_async_mutex;
        std::set<ConnectData> m_async_connects;
    };
}

#endif
