#pragma once

#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>
#include <algorithm>

template<typename T>
class RawMemory {
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity)
            : buffer_(Allocate(capacity)), capacity_(capacity) {
    }

    RawMemory(const RawMemory &) = delete;

    RawMemory &operator=(const RawMemory &rhs) = delete;

    RawMemory(RawMemory &&other) noexcept {
        this->Swap(other);
    }

    RawMemory &operator=(RawMemory &&rhs) noexcept {
        if (this != &rhs) {
            this->Swap(rhs);
            Deallocate(rhs.buffer_);
            rhs.buffer_ = nullptr;
            rhs.capacity_ = 0;
        }
        return *this;
    }

    ~RawMemory() {
        Deallocate(buffer_);
    }

    T *operator+(size_t offset) noexcept {
        // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
        assert(offset <= capacity_);
        return buffer_ + offset;
    }

    const T *operator+(size_t offset) const noexcept {
        return const_cast<RawMemory &>(*this) + offset;
    }

    const T &operator[](size_t index) const noexcept {
        return const_cast<RawMemory &>(*this)[index];
    }

    T &operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

    void Swap(RawMemory &other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    const T *GetAddress() const noexcept {
        return buffer_;
    }

    T *GetAddress() noexcept {
        return buffer_;
    }

    size_t Capacity() const {
        return capacity_;
    }

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T *Allocate(size_t n) {
        return n != 0 ? static_cast<T *>(operator new(n * sizeof(T))) : nullptr;
    }

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T *buf) noexcept {
        operator delete(buf);
    }

    T *buffer_ = nullptr;
    size_t capacity_ = 0;
};

template<typename T>
class Vector {
public:
    using iterator = T *;
    using const_iterator = const T *;

    Vector() = default;

    explicit Vector(size_t size)
            : data_(size), size_(size)  //
    {
        std::uninitialized_value_construct_n(data_.GetAddress(), size);
    }

    Vector(const Vector &other)
            : data_(other.size_), size_(other.size_)  //
    {
        std::uninitialized_copy_n(other.data_.GetAddress(), size_, data_.GetAddress());
    }

    Vector(Vector &&other) noexcept {
        this->Swap(other);
    }

    Vector &operator=(const Vector &rhs) {
        if (this != &rhs) {
            if (rhs.size_ > data_.Capacity()) {
                /* Применить copy-and-swap */
                Vector rhs_copy(rhs);
                this->Swap(rhs_copy);
            } else {
                if (rhs.size_ < size_) {
                    std::copy_n(rhs.data_.GetAddress(), rhs.size_, data_.GetAddress());
                    std::destroy_n(data_.GetAddress() + rhs.size_, size_ - rhs.size_);
                    size_ = rhs.size_;

                    // Размер вектора-источника больше или равен размеру вектора-приёмника
                    // Тогда нужно присвоить существующим элементам приёмника значения соответствующих элементов источника,
                    // а оставшиеся скопировать в свободную область, используя функцию uninitialized_copy или uninitialized_copy_n
                } else {
                    std::copy_n(rhs.data_.GetAddress(), size_, data_.GetAddress());
                    std::uninitialized_copy_n(rhs.data_.GetAddress() + size_, rhs.size_ - size_,
                                              data_.GetAddress() + size_);
                    size_ = rhs.size_;
                }
            }
        }
        return *this;
    }

    Vector &operator=(Vector &&rhs) noexcept {
        this->Swap(rhs);
        return *this;
    }

    void Swap(Vector &other) noexcept {
        std::swap(size_, other.size_);
        data_.Swap(other.data_);
    }

    ~Vector() {
        std::destroy_n(data_.GetAddress(), size_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= data_.Capacity()) {
            return;
        }
        RawMemory<T> new_data(new_capacity);
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
        } else {
            std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
    }

    size_t Size() const noexcept {
        return size_;
    }

    size_t Capacity() const noexcept {
        return data_.Capacity();
    }

    const T &operator[](size_t index) const noexcept {
        return const_cast<Vector &>(*this)[index];
    }

    T &operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

    void Resize(size_t n) {
        if (size_ < n) {
            Reserve(n);
            std::uninitialized_value_construct_n(data_ + size_, n - size_);
        } else if (size_ > n) {
            std::destroy_n(data_ + n, size_ - n);
        }
        size_ = n;
    }

    //воизбежание дублирования кода двумя версиями PushBack
    //для константной ссылки и rvalue
    //сделаем универсальную ссылку
    template<typename E>
    void PushBack(E &&elem) {
        if (size_ == data_.Capacity()) {
            RawMemory<T> new_data(size_ == 0 ? 1 : 2 * size_);
            new(new_data + size_) T(std::forward<E>(elem));
            if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
            } else {
                std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
            }
            std::destroy_n(data_.GetAddress(), size_);
            data_.Swap(new_data);
        } else {
            new(data_ + size_) T(std::forward<E>(elem));
        }
        ++size_;
    }

    template<typename... Args>
    T &EmplaceBack(Args &&... args) {
        if (size_ == data_.Capacity()) {
            RawMemory<T> new_data(size_ == 0 ? 1 : 2 * size_);
            new(new_data.GetAddress() + size_)  T(std::forward<Args>(args)...);
            if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
            } else {
                std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
            }
            std::destroy_n(data_.GetAddress(), size_);
            data_.Swap(new_data);
        } else {
            new(data_ + size_)  T(std::forward<Args>(args)...);
        }
        ++size_;
        return data_[size_ - 1];
    }


    void PopBack() {
        if (size_ > 0) {
            std::destroy_at(data_ + size_ - 1);
            --size_;
        }
    }

    iterator begin() noexcept {
        return data_.GetAddress();
    }

    iterator end() noexcept {
        return data_.GetAddress() + size_;
    }

    const_iterator begin() const noexcept {
        return data_.GetAddress();
    }

    const_iterator end() const noexcept {
        return data_.GetAddress() + size_;
    }

    const_iterator cbegin() const noexcept {
        return data_.GetAddress();
    }

    const_iterator cend() const noexcept {
        return data_.GetAddress() + size_;
    }

    template<typename... Args>
    iterator Emplace(const_iterator pos, Args &&... args) {
        assert(pos >= begin() && pos <= end());
        const size_t pos_num = pos - this->begin();
        if (size_ < data_.Capacity()) { //вместимость позволяет вставить элемент
            //если вектор пустой вставляем в конец и все
            if (pos == this->end()) {
                return &EmplaceBack(std::forward<Args>(args)...);
            }
            //вектор был не пуст
            //Сначала скопируйте или переместите значение во временный объект в зависимости от версии метода Insert.
            // Так вы убережёте значение от перезаписывания, когда вставляется элемент из этого же вектора.
            T tmp(std::forward<Args>(args)...);
            //Сначала в неинициализированной области, следующей за последним элементом,
            // создайте копию или переместите значение последнего элемента вектора
            new(this->end()) T(std::move(*(std::prev(this->end()))));
            //Затем переместите элементы диапазона [pos, end()-1) вправо на один элемент.
            std::move_backward(this->begin() + pos_num, this->end() - 1, this->end());
            //нужно переместить временное созданное значение во вставляемую позицию
            *(data_+pos_num) = std::move(tmp);
            //
            ++size_;
            return (this->begin() + pos_num);

        } else {
            //нужно выделить новый блок сырой памяти с удвоенной вместимостью
            RawMemory<T> new_data(size_ == 0 ? 1 : 2 * size_);
            //сконструировать в ней вставляемый элемент в нужной позиции,
            // используя конструктор копирования или перемещения
            new(new_data.GetAddress() + pos_num)  T(std::forward<Args>(args)...);
            if (size_ > 0) {
                //Затем копируются либо перемещаются элементы, которые предшествуют вставленному элементу
                if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                    std::uninitialized_move_n(this->begin(), pos_num, new_data.GetAddress());
                } else {
                    std::uninitialized_copy_n(this->begin(), pos_num, new_data.GetAddress());
                }
                //Затем копируются либо перемещаются элементы, которые следуют за вставляемым
                if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                    std::uninitialized_move_n(this->begin() + pos_num, size_ - pos_num,
                                              new_data.GetAddress() + pos_num + 1);
                } else {
                    std::uninitialized_copy_n(this->begin() + pos_num, size_ - pos_num,
                                              new_data.GetAddress() + pos_num + 1);
                }
            }
            //
            std::destroy_n(data_.GetAddress(), size_);
            data_.Swap(new_data);
            ++size_;
            return (this->begin() + pos_num);
        }
    }

    iterator Erase(const_iterator pos) /*noexcept(std::is_nothrow_move_assignable_v<T>)*/ {
        assert(pos >= begin() && pos <= end());
        if (size_ > 0) {
            // на место удаляемого элемента нужно переместить следующие за ним элементы
            const size_t offset = pos - this->begin();
            std::move(this->begin() + offset + 1, this->end(), this->begin() + offset);
            //После перемещения элементов в конце вектора останется «пустой» элемент
            std::destroy_at(std::prev(this->end()));
            --size_;
            return (this->begin() + offset);
        }
        return this->end();
    }

    template <typename Arg>
    iterator Insert(const_iterator pos, Arg&& arg) {
        assert(pos >= begin() && pos <= end());
        return Emplace(pos, std::forward<Arg>(arg));
    }

private:
    RawMemory<T> data_;
    size_t size_ = 0;
};