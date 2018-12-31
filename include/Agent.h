#pragma once
#include <boost/asio.hpp>
#include <iostream>
class AgentObject {
};  // concept placeholder
class QuitAgent {
};

template <class Obj>
class Agent {
    static_assert(std::is_base_of_v<AgentObject, Obj>);

    boost::asio::io_context context;
    boost::asio::deadline_timer t{context};
    Obj obj;
    bool quit{false};
    void HandleTick(boost::system::error_code)
    {
        obj.Update();
        std::cout.flush();
        context.post([this]() { Wait(); });
    }
    void Wait()
    {
        if (!quit) {
            t.expires_from_now(boost::posix_time::milliseconds(20));
            t.async_wait(
                [this](boost::system::error_code ec) { HandleTick(ec); });
        }
    }

public:
    Agent() {}
    template <class... Args>
    Agent(Args&&... args) : obj(std::forward<Args>(args)...)
    {
    }

    void Run()
    {
        context.post([this]() { Wait(); });
        context.run();
    }
    // use this to change thread context
    // THREAD SAFE!
    template <class Fn>
    void Post(Fn fn)
    {
        context.post([this, fn]() { fn(obj); });
    }

    template <class Msg>
    void Send(Msg msg)
    {
        context.post([this, msg]() { obj.Send(msg); });
    }

    void Send(QuitAgent)
    {
        context.post([this]() { quit = true; });
    }

    Obj& get_obj() { return obj; }
    const Obj& get_obj() const { return obj; }
};
