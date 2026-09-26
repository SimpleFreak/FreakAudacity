#ifndef __MUSE_GLOBAL_BYTE_ARRAY_HPP__
#define __MUSE_GLOBAL_BYTE_ARRAY_HPP__

#include <cstdint>
#include <memory>
#include <vector>

#ifndef NO_QT_SUPPORT
#include <QByteArray>
#endif

namespace muse {
    class ByteArray {
    public:
        using value_type = uint8_t;

        ByteArray();
        ByteArray(const uint8_t* data, size_t size);
        ByteArray(const char* str, size_t size = static_cast<size_t>(-1));
        ByteArray(size_t size);

        static ByteArray fromRawData(const uint8_t* data, size_t size);
        static ByteArray fromRawData(const char* data, size_t size);

        bool operator==(const ByteArray& other) const;
        bool operator!=(const ByteArray& other) const;

        uint8_t* data();
        std::vector<uint8_t>& vdata();
        const uint8_t* constData() const;
        const char* constChar() const;
        const std::vector<uint8_t>& constVData() const;
        size_t size() const;
        bool empty() const;

        ByteArray& insert(size_t pos, uint8_t b);
        void push_back(uint8_t b);
        void push_back(const uint8_t* b, size_t len);
        void push_back(const ByteArray& ba);

        uint8_t at(size_t pos) const;
        uint8_t operator[](size_t pos) const;
        uint8_t& operator[](size_t pos);

        void reserve(size_t nsize);
        void resize(size_t nsize);
        void truncate(size_t pos);
        void clear();

        ByteArray left(size_t len) const;
        ByteArray right(size_t len) const;

#ifndef NO_QT_SUPPORT
        static ByteArray fromQByteArray(const QByteArray& qArray) {
            return ByteArray(reinterpret_cast<const uint8_t*>(qArray.constData()), qArray.size());
        }

        static ByteArray fromQByteArrayNoCopy(const QByteArray& qArray) {
            return fromRawData(reinterpret_cast<const uint8_t*>(qArray.constData()), qArray.size());
        }

        QByteArray toQByteArray() const {
            return QByteArray(reinterpret_cast<const char*>(constData()), static_cast<int>(size()));
        }

        QByteArray toQByteArrayNoCopy() const {
            return QByteArray::fromRawData(reinterpret_cast<const char*>(constData()), static_cast<int>(size()));
        }
#endif

    private:
        using Data = std::vector<uint8_t>;
        struct RawData {
            const uint8_t* data = nullptr;
            size_t size = 0;
        };

        void detach();

        std::shared_ptr<Data> m_data{};
        RawData m_raw{};
    };
}

#endif
