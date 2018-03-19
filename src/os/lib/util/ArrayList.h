#ifndef __ArrayList_include__
#define __ArrayList_include__

#include <kernel/Cpu.h>
#include "List.h"

extern "C" {
    #include "lib/libc/string.h"
    #include <lib/libc/stdlib.h>
};

namespace Util {

    template <typename T>
    class ArrayList : public List<T> {

    public:

        ArrayList();

        explicit ArrayList(uint32_t capacity);

        ArrayList(const ArrayList<T> &other) = delete;

        ArrayList<T> &operator=(const ArrayList<T> &other) = delete;

        ~ArrayList();

        bool add(const T &element) override;

        void add(uint32_t index, const T &element) override;

        bool addAll(const Collection<T> &other) override;

        T get(uint32_t index) const override;

        void set(uint32_t index, const T &element) override;

        bool remove(const T &element) override;

        bool removeAll(const Collection<T> &other) override;

        T remove(uint32_t index) override;

        bool contains(const T &element) const override;

        bool containsAll(const Collection<T> &other) const override;

        uint32_t indexOf(const T &element) const override;

        bool isEmpty() const override;

        void clear() override;

        Iterator<T> begin() const override;

        Iterator<T> end() const override;

        uint32_t size() const override;

        Array<T> toArray() const override;

    private:

        T *elements = nullptr;

        uint32_t capacity = 0;

        uint32_t length = 0;

        static const uint32_t   DEFAULT_CAPACITY    = 8;

        void ensureCapacity(uint32_t newCapacity) override;

    };

    template <class T>
    ArrayList<T>::ArrayList() {

        this->length = 0;

        this->capacity = 0;

        this->elements = nullptr;
    }

    template <class T>
    ArrayList<T>::ArrayList(uint32_t capacity) {

        this->length = 0;

        this->capacity = capacity;

        this->elements = new T[capacity];
    }

    template <class T>
    ArrayList<T>::~ArrayList() {

        delete[] elements;
    }

    template <class T>
    T ArrayList<T>::get(uint32_t index) const {

        if (index >= length) {
            Cpu::throwException(Cpu::Exception::OUTOFBOUNDS);
        }

        return elements[index];
    }

    template <class T>
    bool ArrayList<T>::add(const T &element) {

        ensureCapacity(length + 1);

        elements[length] = element;

        length++;

        return true;
    }

    template <class T>
    bool ArrayList<T>::addAll(const Collection<T> &other) {

        for (const T &element : other) {

            add(element);
        }

        return true;
    }

    template <class T>
    void ArrayList<T>::add(uint32_t index, const T &element) {

        if (index > length) {
            return;
        }

        ensureCapacity(length + 1);

        memcpy(&elements[index + 1], &elements[index], capacity - index);

        elements[index] = element;

        length++;
    }

    template <class T>
    bool ArrayList<T>::remove(const T &element) {

        uint32_t index = indexOf(element);

        if (index >= capacity) {
            return false;
        }

        remove(index);

        return true;
    }

    template <class T>
    T ArrayList<T>::remove(uint32_t index) {

        if (index >= length) {
            Cpu::throwException(Cpu::Exception::OUTOFBOUNDS);
        }

        T tmp = elements[index];

        uint32_t numMoved = length - index - 1;

        if (numMoved != 0) {

            memcpy(&elements[index], &elements[index + 1], numMoved * sizeof(T));
        }

        length--;

        return tmp;
    }

    template <class T>
    bool ArrayList<T>::removeAll(const Collection<T> &other) {

        bool changed = false;

        for (const T &element : other) {

            if (remove(element)) {

                changed = true;
            }
        }

        return changed;
    }

    template <class T>
    uint32_t ArrayList<T>::indexOf(const T &element) const {

        uint32_t index;

        for (index = 0; elements[index] != element && index < capacity; index++);

        return index == capacity ? UINT32_MAX : index;
    }

    template <class T>
    bool ArrayList<T>::contains(const T &element) const {

        return indexOf(element) < capacity;
    }

    template <class T>
    bool ArrayList<T>::containsAll(const Collection<T> &other) const {

        for (const T &element : other) {

            if (!contains(element)) {

                return false;
            }
        }

        return true;
    }

    template <class T>
    void ArrayList<T>::clear() {

        length = 0;
    }

    template <class T>
    uint32_t ArrayList<T>::size() const {

        return length;
    }

    template <class T>
    void ArrayList<T>::ensureCapacity(uint32_t newCapacity) {

        if (capacity == 0) {

            capacity = DEFAULT_CAPACITY;

            elements = new T[capacity];
        }

        if (newCapacity <= capacity) {
            return;
        }

        uint32_t oldCapacity = capacity;

        while (capacity < newCapacity) {
            capacity *= 2;
        }

        T* tmp = elements;

        elements = new T[capacity];

        for (uint32_t i = 0; i < oldCapacity; i++) {

            elements[i] = tmp[i];
        }

        delete[] tmp;
    }

    template <class T>
    Iterator<T> ArrayList<T>::begin() const {

        return Iterator<T>(toArray(), 0);
    }

    template <class T>
    Iterator<T> ArrayList<T>::end() const {

        return Iterator<T>(toArray(), length);
    }

    template <class T>
    bool ArrayList<T>::isEmpty() const {

        return length == 0;
    }

    template <class T>
    void ArrayList<T>::set(uint32_t index, const T &element) {

        if (index >= length) {
            return;
        }

        elements[index] = element;
    }

    template <class T>
    Array<T> ArrayList<T>::toArray() const {

        Array<T> array(length);

        for (uint32_t i = 0; i < length; i++) {

            array[i] = elements[i];
        }

        return array;
    }

}

#endif