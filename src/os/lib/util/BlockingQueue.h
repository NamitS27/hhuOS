#ifndef __BlockingQueue_include__
#define __BlockingQueue_include__

#include "Queue.h"
#include "ArrayList.h"

namespace Util {

    template <typename T>
    class BlockingQueue : public Queue<T> {

    public:

        BlockingQueue();

        explicit BlockingQueue(uint32_t capacity);

        ~BlockingQueue() = default;

        BlockingQueue(const BlockingQueue<T> &other) = delete;

        BlockingQueue<T> &operator=(const BlockingQueue<T> &other) = delete;

        void push(const T &element) override;

        T pop() override;

        bool add(const T &element) override;

        bool addAll(const Collection<T> &other) override;

        bool remove(const T &element) override;

        bool removeAll(const Collection<T> &other) override;

        bool contains(const T &element) const override;

        bool containsAll(const Collection<T> &other) const override;

        bool isEmpty() const override;

        void clear() override;

        Iterator<T> begin() const override;

        Iterator<T> end() const override;

        uint32_t size() const override;

        Array<T> toArray() const override;

    private:

        ArrayList<T> elements;

        static const uint32_t DEFAULT_CAPACITY = 16;

    };

    template<class T>
    BlockingQueue<T>::BlockingQueue() : elements(DEFAULT_CAPACITY) {

    }

    template<class T>
    BlockingQueue<T>::BlockingQueue(uint32_t capacity) : elements(capacity) {

    }

    template<class T>
    void BlockingQueue<T>::push(const T &element) {

        elements.add(element);
    }

    template<class T>
    T BlockingQueue<T>::pop() {

        while (isEmpty()) {}

        return elements.remove((uint32_t) 0);
    }

    template<class T>
    bool BlockingQueue<T>::add(const T &element) {

        return elements.add(element);
    }

    template<class T>
    bool BlockingQueue<T>::addAll(const Collection<T> &other) {

        return elements.addAll(other);
    }

    template<class T>
    bool BlockingQueue<T>::remove(const T &element) {

        return elements.remove(element);
    }

    template<class T>
    bool BlockingQueue<T>::removeAll(const Collection<T> &other) {

        return elements.removeAll(other);
    }

    template<class T>
    bool BlockingQueue<T>::contains(const T &element) const {

        return elements.contains(element);
    }

    template<class T>
    bool BlockingQueue<T>::containsAll(const Collection<T> &other) const {

        return elements.containsAll(other);
    }

    template<class T>
    bool BlockingQueue<T>::isEmpty() const {

        return elements.isEmpty();
    }

    template<class T>
    void BlockingQueue<T>::clear() {

        elements.clear();
    }

    template<class T>
    Iterator<T> BlockingQueue<T>::begin() const {

        return elements.begin();
    }

    template<class T>
    Iterator<T> BlockingQueue<T>::end() const {

        return elements.end();
    }

    template<class T>
    uint32_t BlockingQueue<T>::size() const {

        return elements.size();
    }

    template<class T>
    Array<T> BlockingQueue<T>::toArray() const {

        return elements.toArray();
    }
}


#endif