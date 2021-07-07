#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <utility>
#include <stdexcept>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    size_t capacity_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) 
        : size_(size), 
        capacity_(size),
        items_(size)
    {
    }

    explicit SimpleVector(const ReserveProxyObj& reserve) 
        : capacity_(reserve.capacity_),
        items_(reserve.capacity_)
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) 
        : size_(size), 
        capacity_(size),
        items_(size)
    {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) 
        : size_(init.size()), 
        capacity_(init.size()),
        items_(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector& other) 
        : size_(other.size_),
        capacity_(other.capacity_),
        items_(other.capacity_)
    {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector rhs_copy(rhs);
            swap(rhs_copy);
        }
        return *this;
    }

    SimpleVector(SimpleVector&& other) noexcept :
        size_(std::exchange(other.size_, 0)),
        capacity_(std::exchange(other.capacity_, 0)),
        items_(std::move(other.items_))
    {
    }

    SimpleVector& operator=(SimpleVector&& other) noexcept {
        if (this != &other) {
            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
            items_ = std::move(other.items_);
        }
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index < size_) {
            return items_[index];
        }
        else {
            throw std::out_of_range("index >= size");
        }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index < size_) {
            return items_[index];
        }
        else {
            throw std::out_of_range("index >= size");
        }
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if(new_size > size_ && new_size <= capacity_) {
            std::fill(begin() + size_, begin() + new_size, Type());
        }
        else if(new_size > capacity_){
            auto new_capacity = std::max(new_size, 2 * capacity_);
            ArrayPtr<Type> copy(new_capacity);
            std::move(begin(), end(), copy.Get());
            items_.swap(copy);
            capacity_ = new_capacity;
        }
        size_ = new_size;
    }

    // Изменяет размер capacity
    //Если new_capacity > capacity_ нужно выделить новое место под массив и скопировать все элементы
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> copy(new_capacity);
            std::move(begin(), end(), copy.Get());
            items_.swap(copy);
            capacity_ = new_capacity;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return cbegin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return cend();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        Resize(size_ + 1);
        items_[size_ - 1] = item;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(Type&& item) {
        ResizeBeforeMove(size_ + 1);
        items_[size_ - 1] = std::move(item);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t index = pos - begin();
        Resize(size_ + 1);
        std::move_backward(begin() + index, end() - 1, end());
        items_[index] = value;
        return &items_[index];
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t index = pos - begin();
        ResizeBeforeMove(size_ + 1);
        std::move_backward(begin() + index, end() - 1, end());
        items_[index] = std::move(value);
        return &items_[index];
    }


    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos != end());
        size_t index = pos - begin();
        std::move(begin() + index + 1, end(), begin() + index);
        --size_;
        return &items_[index];
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }

private:
    void ResizeBeforeMove(size_t new_size) {
        if (new_size > capacity_) {
            auto new_capacity = std::max(new_size, 2 * capacity_);
            ArrayPtr<Type> copy(new_capacity);
            std::move(begin(), end(), copy.Get());
            items_.swap(copy);
            capacity_ = new_capacity;
        }
        size_ = new_size;
    }

    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> items_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() ? std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend()) : false;
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs == rhs ? true : lhs < rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}