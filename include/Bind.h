#ifndef SIMPLE_BIND_NO_TUPLE_H
#define SIMPLE_BIND_NO_TUPLE_H

// ===============================================================
// 0 args
// ===============================================================
template <typename R, typename C>
class Binder0
{
public:
    using Method = R (C::*)();

    Binder0(C *obj, Method method)
        : obj(obj), method(method) {}

    R call()
    {
        return (obj->*method)();
    }

private:
    C *obj;
    Method method;
};

template <typename R, typename C>
Binder0<R, C> bind(R (C::*method)(), C *obj)
{
    return Binder0<R, C>(obj, method);
}

// ===============================================================
// 1 arg
// ===============================================================
template <typename R, typename C, typename A1>
class Binder1
{
public:
    using Method = R (C::*)(A1);

    Binder1(C *obj, Method method, A1 &a1)
        : obj(obj), method(method), a1(a1) {}

    R call()
    {
        return (obj->*method)(a1);
    }

private:
    C *obj;
    Method method;
    A1 &a1;
};

template <typename R, typename C, typename A1>
Binder1<R, C, A1> bind(R (C::*method)(A1), C *obj, A1 &a1)
{
    return Binder1<R, C, A1>(obj, method, a1);
}

// ===============================================================
// 2 args
// ===============================================================
template <typename R, typename C, typename A1, typename A2>
class Binder2
{
public:
    using Method = R (C::*)(A1, A2);

    Binder2(C *obj, Method method, A1 &a1, A2 &a2)
        : obj(obj), method(method), a1(a1), a2(a2) {}

    R call()
    {
        return (obj->*method)(a1, a2);
    }

private:
    C *obj;
    Method method;
    A1 &a1;
    A2 &a2;
};

template <typename R, typename C, typename A1, typename A2>
Binder2<R, C, A1, A2> bind(R (C::*method)(A1, A2), C *obj, A1 &a1, A2 &a2)
{
    return Binder2<R, C, A1, A2>(obj, method, a1, a2);
}

// ===============================================================
// 3 args
// ===============================================================
template <typename R, typename C, typename A1, typename A2, typename A3>
class Binder3
{
public:
    using Method = R (C::*)(A1, A2, A3);

    Binder3(C *obj, Method method, A1 &a1, A2 &a2, A3 &a3)
        : obj(obj), method(method), a1(a1), a2(a2), a3(a3) {}

    R call()
    {
        return (obj->*method)(a1, a2, a3);
    }

private:
    C *obj;
    Method method;
    A1 &a1;
    A2 &a2;
    A3 &a3;
};

template <typename R, typename C, typename A1, typename A2, typename A3>
Binder3<R, C, A1, A2, A3> bind(R (C::*method)(A1, A2, A3),
                               C *obj,
                               A1 &a1, A2 &a2, A3 &a3)
{
    return Binder3<R, C, A1, A2, A3>(obj, method, a1, a2, a3);
}

// ===============================================================
// 4 args (можно расширить)
// ===============================================================
template <typename R, typename C, typename A1, typename A2, typename A3, typename A4>
class Binder4
{
public:
    using Method = R (C::*)(A1, A2, A3, A4);

    Binder4(C *obj, Method method, A1 &a1, A2 &a2, A3 &a3, A4 &a4)
        : obj(obj), method(method), a1(a1), a2(a2), a3(a3), a4(a4) {}

    R call()
    {
        return (obj->*method)(a1, a2, a3, a4);
    }

private:
    C *obj;
    Method method;
    A1 &a1;
    A2 &a2;
    A3 &a3;
    A4 &a4;
};

template <typename R, typename C,
          typename A1, typename A2, typename A3, typename A4>
Binder4<R, C, A1, A2, A3, A4> bind(R (C::*method)(A1, A2, A3, A4),
                                   C *obj,
                                   A1 &a1, A2 &a2, A3 &a3, A4 &a4)
{
    return Binder4<R, C, A1, A2, A3, A4>(obj, method, a1, a2, a3, a4);
}

#endif
