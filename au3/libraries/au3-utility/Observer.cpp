#include "Observer.hpp"

namespace Observer {
    namespace detail {
        void RecordBase::Unlink() noexcept {
            auto pPrev = preview.lock();
            assert(pPrev);
            if (auto& pNext = (pPrev->next = next)) {
                pNext->preview = std::move(preview);
            }
        }

        RecordList::RecordList(ExceptionPolicy* pPolicy, Visitor visitor)
            : m_pPolicy{pPolicy}, m_visitor{visitor} {
            assert(m_visitor);
        }

        RecordList::~RecordList() noexcept {
            auto pRecord = std::move(next);
            while (pRecord) {
                pRecord = std::move(pRecord->next);
            }
        }

        Subscription RecordList::Subscribe(std::shared_ptr<RecordBase> pRecord) {
            assert(pRecord);
            auto result = Subscription{ pRecord };
            if (auto& pNext = (pRecord->next = std::move(next))) {
                pNext->preview = pRecord;
            }

            pRecord->preview = weak_from_this();
            next = std::move(pRecord);

            return result;
        }

        bool RecordList::Visit(const void* arg) {
            assert(m_visitor);
            if (m_pPolicy) {
                m_pPolicy->OnBeginPublish();
            }

            bool result = false;
            for (auto pRecord = next; pRecord; pRecord = pRecord->next) {
                try {
                    if (m_visitor(*pRecord, arg)) {
                        result = true;
                        break;
                    }
                } catch (...) {
                    if (m_pPolicy && m_pPolicy->OnEachFailedCallback()) {
                        result = true;
                        break;
                    }
                }
            }

            if (m_pPolicy) {
                m_pPolicy->OnEndPublish();
            }

            return result;
        }
    }

    ExceptionPolicy::~ExceptionPolicy() noexcept = default;

    Subscription::Subscription() = default;

    Subscription::Subscription(std::weak_ptr<detail::RecordBase> pRecord)
        : m_wRecord{std::move(pRecord)} {}

    Subscription::Subscription(Subscription&&) = default;

    Subscription& Subscription::operator=(Subscription&& other) {
        const bool inequivalent = m_wRecord.owner_before(other.m_wRecord)
            || other.m_wRecord.owner_before(m_wRecord);
        if (inequivalent) {
            Reset();
            m_wRecord = std::move(other.m_wRecord);
        }

        return *this;
    }
    
    void Subscription::Reset() noexcept
    {
        if (auto pRecord = m_wRecord.lock()) {
            pRecord->Unlink();
        }

        m_wRecord.reset();
    }
}
