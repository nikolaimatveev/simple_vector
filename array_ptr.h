#pragma once

#include <stdexcept>
#include <algorithm>
#include <utility>

// Умный указатель, удаляющий связанный объект при своём разрушении.
// Параметр шаблона T задаёт тип объекта, на который ссылается указатель
template <typename Type>
class ArraydPtr {
public:
    // Конструктор по умолчанию создаёт нулевой указатель,
    // так как поле ptr_ имеет значение по умолчанию nullptr
    ArraydPtr() = default;

    // Создаёт указатель, ссылающийся на переданный raw_ptr.
    // raw_ptr ссылается либо на объект, созданный в куче при помощи new,
    // либо является нулевым указателем
    // Спецификатор noexcept обозначает, что метод не бросает исключений
    explicit ArraydPtr(Type* raw_ptr) noexcept {
        ptr_ = raw_ptr;
    }

    // Удаляем у класса конструктор копирования
    ArraydPtr(const ArraydPtr&) = delete ;
    // Конструктор копирования для move
    ArraydPtr(ArraydPtr&& other) noexcept : ptr_(std::exchange(other.ptr_, nullptr))
    {
    }

    ArraydPtr& operator=(ArraydPtr&& other) noexcept {
        if (this != &other) {
            ptr_ = std::exchange(other.ptr_, nullptr);
        }
        return *this;
    }

    // Деструктор. Удаляет объект, на который ссылается умный указатель.
    ~ArraydPtr() {
        delete [] ptr_;
    }

    // Возвращает указатель, хранящийся внутри ArraydPtr
    Type* GetRawPtr() const noexcept {
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

    // Оператор разыменования возвращает ссылку на объект
    // Выбрасывает исключение std::logic_error, если указатель нулевой
    Type& operator*() const {
        if (*this) {
            return *ptr_;
        }
        else {
            throw std::logic_error("Point to zero");
        }
    }

    // Оператор -> должен возвращать указатель на объект
    // Выбрасывает исключение std::logic_error, если указатель нулевой
    Type* operator->() const {
        if (*this) {
            return ptr_;
        }
        else {
            throw std::logic_error("Point to zero");
        }
    }

    Type& operator[](size_t index) {
        return ptr_[index];
    }

    const Type& operator[](size_t index) const {
        return ptr_[index];
    }

    ArraydPtr& operator=(ArraydPtr& right) {
        if (this != &right) {
            this->Swap(right);
        }
        return *this;
    }

    ArraydPtr& operator=(Type* right_ptr) noexcept {
        this->Swap(right_ptr);
        return *this;
    }

    // Обменивает состояние текущего объекта с other без выбрасывания исключений
    void Swap(Type* other_ptr) noexcept {
        std::swap(ptr_, other_ptr);
    }

    // Обменивает состояние текущего объекта с other без выбрасывания исключений
    void Swap(ArraydPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
    }

private:
    Type* ptr_ = nullptr;
};