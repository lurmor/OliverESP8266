#ifndef SIMPLE_LINKED_LIST_H
#define SIMPLE_LINKED_LIST_H

#include <Arduino.h>

template <typename T>
class List
{
private:
    struct Node
    {
        T value;
        Node *next;
        Node(T &v) : value(v), next(nullptr) {}
    };
    Node *head = nullptr;
    Node *tail = nullptr;
    size_t _size = 0;

public:
    List()
    {
        head = nullptr;
        tail = nullptr;
        _size = 0;
    }

    ~List()
    {
        clear();
    }

    size_t size() const
    {
        return _size;
    }

    bool isEmpty() const
    {
        return _size == 0;
    }

    // Добавление в конец списка
    void add(T &value)
    {
        Node *n = new Node(value);
        if (!head)
        {
            head = tail = n;
        }
        else
        {
            tail->next = n;
            tail = n;
        }
        _size++;
    }

    // Вставка по индексу
    bool insert(size_t index, const T &value)
    {
        if (index > _size)
            return false;

        Node *n = new Node(value);

        // вставка в начало
        if (index == 0)
        {
            n->next = head;
            head = n;
            if (_size == 0)
                tail = head;
            _size++;
            return true;
        }

        Node *prev = head;
        for (size_t i = 0; i < index - 1; i++)
        {
            prev = prev->next;
        }

        n->next = prev->next;
        prev->next = n;

        if (n->next == nullptr)
            tail = n;
        _size++;

        return true;
    }

    // Удаление по индексу
    bool removeAt(size_t index)
    {
        if (index >= _size)
            return false;

        Node *toDelete;

        // удаление из начала
        if (index == 0)
        {
            toDelete = head;
            head = head->next;
            if (_size == 1)
                tail = nullptr;
            delete toDelete;
            _size--;
            return true;
        }

        Node *prev = head;
        for (size_t i = 0; i < index - 1; i++)
        {
            prev = prev->next;
        }

        toDelete = prev->next;
        prev->next = toDelete->next;

        if (toDelete == tail)
            tail = prev;

        delete toDelete;
        _size--;
        return true;
    }

    // Безопасный доступ
    bool get(size_t index, T &out) const
    {
        if (index >= _size)
            return false;

        Node *cur = head;
        for (size_t i = 0; i < index; i++)
            cur = cur->next;

        out = cur->value;
        return true;
    }

    // Быстрый доступ (без проверки границ)
    T &operator[](size_t index)
    {
        Node *cur = head;
        for (size_t i = 0; i < index; i++)
        {
            cur = cur->next;
        }
        return cur->value;
    }

    // Очистка
    void clear()
    {
        Node *cur = head;
        while (cur)
        {
            Node *next = cur->next;
            delete cur;
            cur = next;
        }
        head = tail = nullptr;
        _size = 0;
    }
};

#endif
