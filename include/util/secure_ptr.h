#ifndef __SECURE_PTR_H__
#define __SECURE_PTR_H__
#include <cstring>
#include <memory>
#include <functional>
#include <type_traits> 
#include "node/binary.h"
#include "cell.h"

using BwtFS::Node::Binary;

class Encryptor {
public:
    constexpr static const char* value = "Encryptor";
    virtual ~Encryptor() = default;
    virtual void encrypt(void* data, size_t size) = 0;
    virtual void decrypt(void* data, size_t size) = 0;
};

// 简单的XOR加密实现（仅用于演示）
class XOREncryptor : public Encryptor {
    public:
        constexpr static const char* value = "XOREncryptor";
        XOREncryptor(unsigned char key = 0x55) : m_key(key) {}
    
        void encrypt(void* data, size_t size) override {
            process(data, size);
        }
    
        void decrypt(void* data, size_t size) override {
            process(data, size);
        }
    
    private:
        void process(void* data, size_t size) {
            unsigned char* p = static_cast<unsigned char*>(data);
            for(size_t i = 0; i < size; ++i) {
                p[i] ^= m_key;
            }
        }
    
        unsigned char m_key;
};

using BwtFS::Util::RCA;
class RCAEncryptor : public RCA, public Encryptor{
    public:
        constexpr static const char* value = "RCAEncryptor";
        RCAEncryptor() = default;
        RCAEncryptor(unsigned seed, Binary& binary) : RCA(seed, binary) {}
        RCAEncryptor(RCAEncryptor const& other) = delete;
        RCAEncryptor& operator=(RCAEncryptor const& other) = delete;
        RCAEncryptor(RCAEncryptor&& other) = delete;
        RCAEncryptor& operator=(RCAEncryptor&& other) = delete;
    
        void encrypt(void* data, size_t size) override {
            RCA::forward();
        }
    
        void decrypt(void* data, size_t size) override {
            RCA::backward();
        }

        void setSeed(unsigned seed) {
            RCA::setSeed(seed);
        }

        void setBinary(Binary& binary) {
            RCA::setBinary(binary);
        }
};

class EncryptedObject {
    public:
        EncryptedObject(void* ptr, size_t size, Encryptor& encryptor)
            : m_ptr(ptr), m_size(size), m_encryptor(encryptor), m_locked(true) {
        }
    
        ~EncryptedObject() {
            if(!m_locked) {
                lock();
            }
        }
    
        void* access() {
            if(m_locked) {
                m_encryptor.decrypt(m_ptr, m_size);
                m_locked = false;
            }
            return m_ptr;
        }
    
        void lock() {
            if(!m_locked) {
                m_encryptor.encrypt(m_ptr, m_size);
                m_locked = true;
            }
        }
    
    private:
        void* m_ptr;
        size_t m_size;
        Encryptor& m_encryptor;
        bool m_locked;
};

template <typename T>
class EncryptedAccessProxy {
public:
    EncryptedAccessProxy(EncryptedObject& block, T* obj)
        : m_block(block), m_obj(obj) {
    }

    ~EncryptedAccessProxy() {
        m_block.lock();
    }

    T* operator->() {
        m_block.access();
        return m_obj;
    }

    EncryptedAccessProxy(const EncryptedAccessProxy&) = delete;
    EncryptedAccessProxy& operator=(const EncryptedAccessProxy&) = delete;

private:
    EncryptedObject& m_block;
    T* m_obj;
};

template <typename T, typename E = Encryptor>
class SecureAllocator {
public:
    explicit SecureAllocator(std::unique_ptr<Encryptor> encryptor = std::make_unique<E>())
        : m_encryptor(std::move(encryptor)) {}

    void* allocate(size_t size) {
        // void* p = ::operator new(size);
        // if constexpr (E::value == "RCAEncryptor") {
        if constexpr (std::is_same<E, RCAEncryptor>::value) {
            auto data = std::make_shared<std::vector<std::byte>>(size);
            m_data = new Binary(data);
            // static_cast<RCAEncryptor*>(this->m_encryptor.get())->setSeed(reinterpret_cast<uintptr_t>(&data->data()[0]));
            // static_cast<RCAEncryptor*>(this->m_encryptor.get())->setBinary(*m_data);

            this->m_encryptor = std::make_unique<RCAEncryptor>(reinterpret_cast<uintptr_t>(&data->data()[0]), *m_data);
            void* p = m_data->data();
            return p;
        // }else if constexpr (E::value == "XOREncryptor") {
        } else if constexpr (std::is_same<E, XOREncryptor>::value) {
            auto data = std::make_shared<std::vector<std::byte>>(size);
            m_data = new Binary(data);
            void* p = m_data->data();
            return p;
        }else {
            static_assert(std::is_base_of<Encryptor, E>::value, "E must be a subclass of Encryptor");
            auto data = std::make_shared<std::vector<std::byte>>(size);
            m_data = new Binary(data);
            void* p = m_data->data();
            return p;
        }
    }

    void deallocate(void* p, size_t size) {
        if(p == nullptr) {
            return;
        }
        m_encryptor->decrypt(p, size);
        ::operator delete(p);
    }

    template <typename... Args>
    T* construct(void* p, Args&&... args) {
        T* obj = new(p) T(std::forward<Args>(args)...);
        m_encryptor->encrypt(p, sizeof(T));
        return obj;
    }

    void destroy(T* p) {
        m_encryptor->decrypt(p, sizeof(T));
        p->~T();
    }
    Binary *m_data;
    std::unique_ptr<Encryptor> m_encryptor;
};

template <typename T>
class secure_ptr {
public:
    template <typename... Args>
    explicit secure_ptr(std::unique_ptr<Encryptor> encryptor, Args&&... args) 
        : m_allocator(std::move(encryptor)) {
        void* mem = m_allocator.allocate(sizeof(T));
        m_obj = m_allocator.construct(mem, std::forward<Args>(args)...);
        m_memBlock = std::make_unique<EncryptedObject>(
            mem, sizeof(T), *m_allocator.m_encryptor);
    }

    ~secure_ptr() {
        if(m_obj) {
            m_allocator.destroy(m_obj);
            m_allocator.deallocate(m_obj, sizeof(T));
        }
    }

    EncryptedAccessProxy<T> operator->() {
        return EncryptedAccessProxy<T>(*m_memBlock, m_obj);
    }

    secure_ptr(const secure_ptr&) = delete;
    secure_ptr& operator=(const secure_ptr&) = delete;

private:
    SecureAllocator<T, RCAEncryptor> m_allocator;
    T* m_obj = nullptr;
    std::unique_ptr<EncryptedObject> m_memBlock;
};

template <typename T, typename E,  typename... Args>
static secure_ptr<T> make_secure(std::unique_ptr<E> encryptor, Args&&... args) {
    return secure_ptr<T>(std::move(encryptor), std::forward<Args>(args)...);
}

template <typename T, typename E = RCAEncryptor, typename... Args>
static secure_ptr<T> make_secure(Args&&... args) {
    return secure_ptr<T>(std::move(std::make_unique<E>()), std::forward<Args>(args)...);
}

#endif