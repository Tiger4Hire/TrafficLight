#pragma once
#include <gsl/gsl>
#include <unordered_map>
using MemoryBlock = gsl::span<const std::byte>;
class Dispatcher {
    using CallbackFn = std::function<void(MemoryBlock)>;
//    std::unordered_multimap<std::size_t, CallbackFn> callbacks;
    std::unordered_map<std::size_t, CallbackFn> callbacks;

public:
    static Dispatcher Inst;
    void Register(std::size_t tag, CallbackFn fn)
    {
        Expects(callbacks.find(tag) == callbacks.end());
        callbacks[tag] = fn;
    }
    void Unregister(std::size_t tag)
    {
        Expects(callbacks.find(tag) != callbacks.end());
        callbacks.erase(tag);
    }
    void Move(std::size_t tag, CallbackFn fn)
    {
        Expects(callbacks.find(tag) != callbacks.end());
        callbacks[tag] = fn;
    }
    template <class Msg>
    void Send(Msg msg)
    {
        Expects(callbacks.find(typeid(Msg).hash_code()) != callbacks.end());
        callbacks[typeid(Msg).hash_code()](gsl::as_bytes(gsl::make_span(&msg, 1)));
    }
    template <class Msg>
    bool HasHandler()
    {
        return callbacks.find(typeid(Msg).hash_code()) != callbacks.end();
    }
};

template <class Msg>
const Msg& Cast(MemoryBlock mem)
{
    Expects(mem.size() == sizeof(Msg));
    return *reinterpret_cast<const Msg*>(mem.data());
}

template <class Msg>
class Handle {
    void OnEvent(MemoryBlock msg) { OnEvent(Cast<Msg>(msg)); }
    bool moved{false};

public:
    Handle()
    {
        Dispatcher::Inst.Register(typeid(Msg).hash_code(), [this](MemoryBlock msg) { OnEvent(msg); });
    }
    ~Handle() { Dispatcher::Inst.Unregister(typeid(Msg).hash_code()); }
    // non-copyable
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    // moveable
    Handle(Handle&& other)
    {
        other.moved = true;
        Dispatcher::Inst.Move(typeid(Msg).hash_code(), [this](MemoryBlock msg) { OnEvent(msg); });
    }
    Handle& operator=(Handle&& other)
    {
        other.moved = true;
        Dispatcher::Inst.Move(typeid(Msg).hash_code(), [this](MemoryBlock msg) { OnEvent(msg); });
    }

    virtual void OnEvent(const Msg&) = 0;
};