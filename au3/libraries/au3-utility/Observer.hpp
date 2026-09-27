#ifndef __AUDACITY_OBSERVER_HPP__
#define __AUDACITY_OBSERVER_HPP__

#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>

namespace Observer {
    template<typename Message, bool NotifyAll> class Publisher;

    struct Message {};
};

#endif
