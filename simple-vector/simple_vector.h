#pragma once

#include "array_ptr.h"
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <utility>

class ReserveProxyObj {
public:
	ReserveProxyObj(size_t capacity_to_reserve) : capacity(capacity_to_reserve) {
	}
	size_t Get() {
		return capacity;
	}
private:
	size_t capacity = 0;
};

template <typename Type>
class SimpleVector {
public:
	using Iterator = Type*;
	using ConstIterator = const Type*;

	SimpleVector() noexcept = default;

	// Создаёт вектор из size элементов, инициализированных значением по умолчанию
	explicit SimpleVector(size_t size) : mas_ptr(size) {
		std::generate(mas_ptr.Get(), (mas_ptr.Get() + size), []() { return Type(); });
		this->size = size;
		capacity = size;
	}

	// Создаёт вектор из size элементов, инициализированных значением value
	SimpleVector(size_t size, const Type& value) : mas_ptr(size) {
		std::fill(mas_ptr.Get(), mas_ptr.Get() + size, std::move(value));
		this->size = size;
		capacity = size;
	}

	SimpleVector(size_t size, Type&& value) : mas_ptr(size) {
		for (size_t i = 0; i < size; ++i) {
			mas_ptr[i] = std::move(value);
		}
		this->size = size;
		capacity = size;
	}

	// Создаёт вектор из std::initializer_list
	SimpleVector(std::initializer_list<Type> init) : mas_ptr(init.size()) {
		std::copy(init.begin(), init.end(), mas_ptr.Get());
		this->size = init.size();
		capacity = init.size();
	}

	SimpleVector(std::initializer_list<Type>&& init) : mas_ptr(init.size()) {
		std::move(init.begin(), init.end(), mas_ptr.Get());
		this->size = init.size();
		capacity = init.size();
	}

	explicit SimpleVector(ReserveProxyObj obj) {
		Reserve(obj.Get());
	}

	SimpleVector(const SimpleVector& other) {
		try {
			if (!other.IsEmpty()) {
				SimpleVector<Type> tmp(other.size);
				std::copy(other.begin(), other.end(), tmp.begin());
				swap(tmp);
			}
		}
		catch (...) {
			Clear();
			throw;
		}

	}

	SimpleVector(SimpleVector&& other) {
		try {
			if (!other.IsEmpty()) {
				SimpleVector<Type> tmp(std::move(other.size));
				std::copy(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), tmp.begin());
				swap(tmp);
				other.size = 0;
				other.capacity = 0;
			}
		}
		catch (...) {
			Clear();
			throw;
		}

	}

	SimpleVector& operator=(const SimpleVector& rhs) {
		try {
			if (mas_ptr.Get() != rhs.begin()) {
				SimpleVector<Type> v(rhs);
				swap(v);
			}
		}
		catch (...) {
			throw;
		}
		return *this;
	}

	SimpleVector& operator=(SimpleVector&& rhs) {
		try {
			if (mas_ptr.Get() != rhs.begin()) {
				SimpleVector<Type> v(std::move(rhs));
				swap(v);
			}
		}
		catch (...) {
			throw;
		}
		return *this;
	}

	// Обменивает значение с другим вектором
	void swap(SimpleVector& other) noexcept {
		mas_ptr.swap(other.mas_ptr);
		std::swap(size, other.size);
		std::swap(capacity, other.capacity);
	}

	// Возвращает количество элементов в массиве
	size_t GetSize() const noexcept {
		return size;
	}

	// Возвращает вместимость массива
	size_t GetCapacity() const noexcept {
		return capacity;
	}

	// Сообщает, пустой ли массив
	bool IsEmpty() const noexcept {
		return size == 0;
	}

	// Возвращает ссылку на элемент с индексом index
	Type& operator[](size_t index) noexcept {
		return mas_ptr[index];
	}

	// Возвращает константную ссылку на элемент с индексом index
	const Type& operator[](size_t index) const noexcept {
		return mas_ptr[index];
	}

	// Возвращает константную ссылку на элемент с индексом index
	// Выбрасывает исключение std::out_of_range, если index >= size
	Type& At(size_t index) {
		if (index >= size) {
			throw std::out_of_range("index out of range");
		}
		return mas_ptr[index];
	}

	// Возвращает константную ссылку на элемент с индексом index
	// Выбрасывает исключение std::out_of_range, если index >= size
	const Type& At(size_t index) const {
		if (index >= size) {
			throw std::out_of_range("index out of range");
		}
		return mas_ptr[index];
	}

	// Обнуляет размер массива, не изменяя его вместимость
	void Clear() noexcept {
		size = 0;
	}

	// Возвращает итератор на начало массива
	// Для пустого массива может быть равен (или не равен) nullptr
	Iterator begin() noexcept {
		return mas_ptr.Get();
	}

	// Возвращает итератор на элемент, следующий за последним
	// Для пустого массива может быть равен (или не равен) nullptr
	Iterator end() noexcept {
		return mas_ptr.Get() + size;
	}

	// Возвращает константный итератор на начало массива
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator begin() const noexcept {
		return mas_ptr.Get();
	}

	// Возвращает итератор на элемент, следующий за последним
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator end() const noexcept {
		return mas_ptr.Get() + size;
	}

	// Возвращает константный итератор на начало массива
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator cbegin() const noexcept {
		return begin();
	}

	// Возвращает итератор на элемент, следующий за последним
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator cend() const noexcept {
		return end();
	}

	void Reserve(size_t new_capacity) {
		if (new_capacity >= capacity) {
			ArrayPtr<Type> tmp(new_capacity);
			std::move(mas_ptr.Get(), (mas_ptr.Get() + size), tmp.Get());
			mas_ptr.swap(tmp);
			capacity = new_capacity;
		}
	}

	// Изменяет размер массива.
	// При увеличении размера новые элементы получают значение по умолчанию для типа Type
	void Resize(size_t new_size) {
		if (new_size > size) {
			// если емкость меньше нового размера резервируем новую память
			if (capacity <= new_size) {
				Reserve(new_size);
			}
			// заполняем пустые ячейки от последнего эл массива до конца
			std::generate(end(), (mas_ptr.Get() + new_size), []() {return Type(); });
		}
		size = new_size;
	}

	// Добавляет элемент в конец вектора
	// При нехватке места увеличивает вдвое вместимость вектора
	void PushBack(const Type& item) {
		if (capacity == 0) {
			SimpleVector tmp(1, item);
			swap(tmp);
			size = 1;
			capacity = 1;
		}
		else if (size == capacity) {
			size_t new_capacity = capacity * 2;
			SimpleVector<Type> tmp(new_capacity);
			std::copy(begin(), end(), tmp.begin());
			tmp.size = size;
			tmp[size] = item;
			swap(tmp);
			++size;
			capacity = new_capacity;
		}
		else {
			mas_ptr[size] = item;
			++size;
		}
	}

	void PushBack(Type&& item) {
		if (capacity == 0) {
			SimpleVector tmp(1, std::move(item));
			swap(tmp);
			size = 1;
			capacity = 1;
		}
		else if (size == capacity) {
			size_t new_capacity = capacity * 2;
			SimpleVector<Type> tmp(new_capacity);
			std::move(begin(), end(), tmp.begin());
			tmp.size = size;
			tmp[size] = std::move(item);
			swap(tmp);
			++size;
			capacity = new_capacity;
		}
		else {
			mas_ptr[size] = std::move(item);
			++size;
		}
	}

	// Вставляет значение value в позицию pos.
	// Возвращает итератор на вставленное значение
	// Если перед вставкой значения вектор был заполнен полностью,
	// вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
	Iterator Insert(ConstIterator pos, const Type& value) {
		auto dist = std::distance(cbegin(), pos);
		if (capacity == 0) {
			PushBack(value);
			return Iterator{ end() - 1 };
		}
		else if (size == capacity) {
			capacity *= 2;
			SimpleVector<Type> tmp(capacity);
			std::copy(begin(), (begin() + dist), tmp.begin());
			tmp.mas_ptr[dist] = value;
			std::copy_backward((begin() + dist), end(), tmp.end());
			swap(tmp);
			size = ++(tmp.size);
		}
		else {
			std::copy_backward((begin() + dist), (begin() + size), (begin() + size + 1));
			mas_ptr[dist] = value;
			++size;
		}
		return Iterator{ Iterator(begin() + dist) };
	}

	Iterator Insert(ConstIterator pos, Type&& value) {
		auto dist = std::distance(cbegin(), pos);
		if (capacity == 0) {
			PushBack(std::move(value));
			return Iterator{ end() - 1 };
		}
		else if (size == capacity) {
			capacity *= 2;
			SimpleVector<Type> tmp(capacity);
			std::move(begin(), (begin() + dist), tmp.begin());
			tmp.mas_ptr[dist] = std::move(value);
			std::move_backward((begin() + dist), end(), tmp.end());
			swap(tmp);
			size = ++(tmp.size);
		}
		else {
			std::move_backward((begin() + dist), (begin() + size), (begin() + size + 1));
			mas_ptr[dist] = std::move(value);
			++size;
		}
		return Iterator{ Iterator(begin() + dist) };
	}

	// "Удаляет" последний элемент вектора. Вектор не должен быть пустым
	void PopBack() noexcept {
		if (size != 0) {
			--size;
		}
	}

	// Удаляет элемент вектора в указанной позиции
	Iterator Erase(ConstIterator pos) {
		if (pos == end()) {
			return end();
		}
		std::move(Iterator(pos + 1), end(), Iterator(pos));
		--size;
		return Iterator(pos);
	}

private:
	ArrayPtr<Type> mas_ptr;
	size_t size = 0;
	size_t capacity = 0;
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	// Заглушка. Напишите тело самостоятельно
	return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	// Заглушка. Напишите тело самостоятельно
	return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	// Заглушка. Напишите тело самостоятельно
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	// Заглушка. Напишите тело самостоятельно
	return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	// Заглушка. Напишите тело самостоятельно
	return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	// Заглушка. Напишите тело самостоятельно
	return !(rhs > lhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
	return ReserveProxyObj(capacity_to_reserve);
}