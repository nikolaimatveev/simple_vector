#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <utility>

#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj() = default;

    ReserveProxyObj(size_t capacity) : capacity_(capacity) {
    }
    size_t capacity_ = 0;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : size_(size), capacity_(size){
        if (capacity_ > 0) {
            items_ = new Type[capacity_]();
        }
    }
    explicit SimpleVector(const ReserveProxyObj& reserve) : SimpleVector(reserve.capacity_) {
        size_ = 0;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : size_(size), capacity_(size) {
        if (capacity_ > 0) {
            items_ = new Type[capacity_];
            std::fill(begin(), end(), value);
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : size_(init.size()), capacity_(init.size()){
        if (size_ > 0) {
            items_ = new Type[capacity_];
            std::copy(init.begin(), init.end(), begin());
        }
    }

    SimpleVector(const SimpleVector& other) : size_(other.size_), capacity_(other.capacity_) {
        if (size_ > 0) {
            items_ = new Type[capacity_];
            std::copy(other.begin(), other.end(), begin());
        }
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector rhs_copy(rhs);
            this->swap(rhs_copy);
        }
        return *this;
    }

    SimpleVector(SimpleVector&& other) noexcept :
        size_(std::exchange(other.size_, 0)),
        capacity_(std::exchange(other.capacity_, 0)),
        items_(std::exchange(other.items_, nullptr))
    {
    }

    SimpleVector& operator=(SimpleVector&& other) noexcept {
        if (this != &other) {
            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
            items_ = std::exchange(other.items_, nullptr);
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
        return size_ == 0 ? true : false;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
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
            std::fill(&items_[size_], &items_[new_size], Type());
        }
        else if(new_size > capacity_){
            SimpleVector copy(std::max(new_size, 2 * capacity_));
            std::copy(begin(), end(), copy.begin());
            this->swap(copy);
        }
        size_ = new_size;
    }

    // Изменяет размер capacity
    //Если new_capacity > capacity_ нужно выделить новое место под массив и скопировать все элементы
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            SimpleVector copy(new_capacity);
            std::copy(begin(), end(), copy.begin());
            items_.Swap(copy.items_);
            capacity_ = new_capacity;
        }
        size_ = std::min(size_, new_capacity);
    }


    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return &items_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return &items_[size_];
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
        return &items_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return &items_[size_];
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
        items_[size_ - 1] = std::exchange(item,Type());
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t index = pos - begin();
        Resize(size_ + 1);
        std::copy_backward(&items_[index], &items_[size_ - 1], end());
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
        std::copy_backward(std::make_move_iterator(&items_[index]), std::make_move_iterator(&items_[size_ - 1]), end());
        items_[index] = std::exchange(value, Type());
        return &items_[index];
    }


    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        size_t index = pos - begin();
        if (pos != end()) {
            std::copy(std::make_move_iterator(&items_[index] + 1), std::make_move_iterator(&items_[size_]), &items_[index]);
            --size_;
        }
        return &items_[index];
    }

    void swap(SimpleVector& other) noexcept {
        items_.Swap(other.items_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }

private:
    void ResizeBeforeMove(size_t new_size) {
        if (new_size > size_ && new_size <= capacity_) {
            SimpleVector tmp(new_size - size_);
            std::copy(std::make_move_iterator(tmp.begin()), std::make_move_iterator(tmp.end()), &items_[size_]);
        }
        else if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, 2 * capacity_);
            SimpleVector copy(new_capacity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), copy.begin());
            items_.Swap(copy.items_);
            capacity_ = new_capacity;
        }
        size_ = new_size;
    }

    size_t size_ = 0;
    size_t capacity_ = 0;
    ArraydPtr<Type> items_;
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
    return lhs < rhs || lhs == rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}