#pragma once

#include <cstddef>
#include <utility>

// Умный указатель, удаляющий связанный объект при своём разрушении.
// Параметр шаблона Type задаёт тип объекта, на который ссылается указатель
template <typename Type>
class ArrayPtr {
public:
    // Конструктор по умолчанию создаёт нулевой указатель,
    // так как поле ptr_ имеет значение по умолчанию nullptr
    ArrayPtr() = default;

    explicit ArrayPtr(std::size_t size) 
        : ptr_(new Type[size]()) 
    {
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept 
        : ptr_(raw_ptr) 
    {
    }

    // Удаляем у класса конструктор копирования
    ArrayPtr(const ArrayPtr&) = delete ;
    // Конструктор копирования для move
    ArrayPtr(ArrayPtr&& other) noexcept 
        : ptr_(std::exchange(other.ptr_, nullptr))
    {
    }

    ArrayPtr& operator=(ArrayPtr&) = delete;

    ArrayPtr& operator=(ArrayPtr&& other) noexcept {
        if (this != &other) {
            ArrayPtr<Type> copy(std::move(ptr_));
            ptr_ = std::exchange(other.ptr_, nullptr);
        }
        return *this;
    }

    // Деструктор. Удаляет объект, на который ссылается умный указатель.
    ~ArrayPtr() {
       delete [] ptr_;
    }

    // Возвращает указатель, хранящийся внутри ArraydPtr
    Type* Get() const noexcept {
        return ptr_;
    }

    // Прекращает владение объектом, на который ссылается умный указатель
    // Возвращает прежнее значение "сырого" указателя и устанавливает поле ptr_ в null
    Type* Release() noexcept {
        Type* result = ptr_;
        ptr_ = nullptr;
        return result;
    }

    // Оператор приведения к типу bool позволяет узнать, ссылается ли умный указатель
    // на какой-либо объект
    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    Type& operator[](std::size_t index) noexcept {
        return ptr_[index];
    }

    const Type& operator[](std::size_t index) const noexcept {
        return ptr_[index];
    }

    // Обменивает состояние текущего объекта с other без выбрасывания исключений
    void swap(ArrayPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
    }

private:
    Type* ptr_ = nullptr;
};