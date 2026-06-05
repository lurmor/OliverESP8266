#ifndef CIRCULAR_ARRAY_H
#define CIRCULAR_ARRAY_H

#include <cstddef>
#include <algorithm>
#include <utility>

template <typename T>
class CircularArray
{
private:
    T *data;
    size_t current_size;
    size_t lastset;

public:
    // 1. Создание с заданием размера
    explicit CircularArray(size_t size) : current_size(size)
    {
        if (size > 0)
        {
            data = new T[size](); // Выделяем память и инициализируем нулями/конструктором по умолчанию
        }
        else
        {
            data = nullptr;
        }
    }

    // Деструктор для очистки памяти
    ~CircularArray()
    {
        delete[] data;
    }

    // Запрещаем копирование во избежание двойного удаления памяти (Rule of Three)
    // При необходимости можно реализовать конструктор копирования и оператор присваивания
    CircularArray(const CircularArray &) = delete;
    CircularArray &operator=(const CircularArray &) = delete;

    void SetLast(size_t last)
    {
        lastset = last % current_size;
    }
    size_t getLast() { return lastset; }
    // 2. Изменение размера с переносом данных
    void resize(size_t new_size)
    {
        if (new_size == current_size)
        {
            return;
        }

        if (new_size == 0)
        {
            delete[] data;
            data = nullptr;
            current_size = 0;
            return;
        }

        T *new_data = new T[new_size]();

        if (data != nullptr)
        {
            // Переносим ровно столько элементов, сколько поместится
            size_t elements_to_copy = std::min(current_size, new_size);
            for (size_t i = 0; i < elements_to_copy; ++i)
            {
                // Используем std::move для эффективного переноса сложных объектов
                new_data[i] = std::move(data[i]);
            }
            delete[] data; // Удаляем старый массив
        }

        data = new_data;
        current_size = new_size;
    }

    // 3. Возврат указателя на элемент по индексу (зацикленный)
    T *get(size_t index)
    {
        if (current_size == 0)
        {
            return nullptr; // Защита от деления на 0
        }

        size_t actual_index = index % current_size;
        return &data[actual_index];
    }

    // Константная версия метода get для работы с const CircularArray
    const T *get(size_t index) const
    {
        if (current_size == 0)
        {
            return nullptr;
        }

        size_t actual_index = index % current_size;
        return &data[actual_index];
    }

    // Вспомогательный метод для получения текущего размера
    size_t size() const
    {
        return current_size;
    }

};

template <typename T>
class SlideWindow
{
private:
    CircularArray<T> window;
    size_t head;        // Индекс, куда будет записано следующее новое значение
    size_t added_count; // Сколько элементов реально добавлено (нужно, пока окно не заполнилось полностью)
    T current_sum;      // Текущая сумма элементов в окне для быстрого вычисления среднего за O(1)

public:
    // Конструктор: инициализируем зацикленный массив заданным размером окна
    explicit SlideWindow(size_t size)
        : window(size), head(0), added_count(0), current_sum(0) {}

    // Основной метод фильтра: добавляет новое значение в окно, выталкивая самое старое
    void push(T val)
    {
        if (window.size() == 0)
            return;

        // Если окно уже полностью заполнено, то элемент по индексу 'head' — это
        // самое старое значение, которое сейчас затрется. Вычитаем его из общей суммы.
        if (added_count >= window.size())
        {
            current_sum -= *window.get(head);
        }
        else
        {
            added_count++;
        }

        // Записываем новое значение в зацикленный массив
        *window.get(head) = val;
        current_sum += val;

        // Сдвигаем указатель head вперед. Благодаря зацикливанию внутри CircularArray,
        // нам не нужно здесь делать % window.size(), метод get() сам со всем разберется!
        head++;
    }

    // Твой метод Set: позволяет принудительно изменить значение в любой точке окна
    // (полезно, если фильтр используется как произвольный буфер данных)
    void Set(size_t point, T val)
    {
        T *ptr = window.get(point);
        if (ptr)
        {
            current_sum -= *ptr; // Вычитаем старое значение
            *ptr = val;          // Записываем новое
            current_sum += val;  // Добавляем новое значение к сумме
        }
    }

    // Возвращает текущее отфильтрованное (среднее арифметическое) значение окна
    double getAverage() const
    {
        if (added_count == 0)
            return 0.0;
        return static_cast<double>(current_sum) / added_count;
    }

    // Сброс фильтра
    void clear()
    {
        head = 0;
        added_count = 0;
        current_sum = 0;
        // Переинициализируем элементы нулями
        for (size_t i = 0; i < window.size(); ++i)
        {
            T *ptr = window.get(i);
            if (ptr)
                *ptr = T();
        }
    }

    size_t size() const
    {
        return window.size();
    }
};

#endif // CIRCULAR_ARRAY_H
