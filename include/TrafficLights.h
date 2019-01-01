// requires : GL is initialised (before Update Called)

#include "glu_objs.h"
#include "Agent.h"
#include "Behaviors.h"
#include "Dispatcher.h"
#include <array>
#include <chrono>
#include <optional>
#include <bitset>

template <class T>
constexpr int Max()
{
    return static_cast<int>(T::MAX);
}

class TrafficLight {
public:
    enum class Colour { RED, AMBER, GREEN, MAX };
    using CallbackFn = std::function<void()>;
    void Update();
    void Render() const;
    void Goto(Colour, bool, CallbackFn);

private:
    using TargetState = bool[Max<Colour>()];
    using Callbacks = CallbackFn[Max<Colour>()];
    using Lights = std::array<float, Max<Colour>()>;

    float speed = .7f;
    GluSphere sphere;
    Lights state{0.f, 0.f, 0.f};
    TargetState target{false, false, false};
    Callbacks callbacks;
    std::optional<std::chrono::high_resolution_clock::time_point> prev_update;
};

// Wait for something to do
using TrafficLightBehavior = Behavior<TrafficLight, int>;
using TrafficLightAll = All<TrafficLight, int>;
// events
struct ButtonPress {
};
struct LightChange {
    TrafficLight::Colour colour;
    bool new_state;
};

struct Undo {
};

class Set : public TrafficLightBehavior {
    bool done{false};
    bool sent{false};

public:
    Result Update(int tgt, int& current, TrafficLight&) final;
    void Undo(int& current, TrafficLight&) final;
};

class Wait : public TrafficLightBehavior, Handle<ButtonPress> {
    std::optional<int> timer;
    bool active{false};
    int count{0};

public:
    Result Update(int tgt, int& current, TrafficLight&) final;
    void Undo(int& current, TrafficLight&) final {}
    void OnEvent(const ButtonPress&) final;
};

using LightSetting = std::bitset<Max<TrafficLight::Colour>()>;
class SetColour : public TrafficLightBehavior {
    const LightSetting to;
    const LightSetting from;
    const int time_max;
    LightSetting current;
    LightSetting sent;
    int timer{0};

public:
    SetColour(LightSetting f, LightSetting t, int time) : to(t), from{f}, current{f}, sent{f}, time_max{time} {}
    Result Update(int tgt, int& current, TrafficLight&) final;
    void Undo(int& current, TrafficLight&) final;
};
class Done : public TrafficLightBehavior {
    Result Update(int tgt, int& current, TrafficLight&) final;
    void Undo(int& current, TrafficLight&) final {}
};
// controller
class TrafficLightSM : AgentObject {
    TrafficLight& controlled_object;
    int num_button_presses{0};
    int num_state_changes{0};
    Set set;
    Wait wait;
    SetColour red2amberred{1, 3, 100};
    SetColour amberred2green{3, 4, 500};
    SetColour green2amber{4, 2, 100};
    SetColour amber2red{2, 1, 100};
    Done done;
    TrafficLightAll all;

public:
    TrafficLightSM(TrafficLight&);

    void Send(Undo)
    {
        num_button_presses--;
        num_button_presses = std::max(num_button_presses, num_state_changes);
    }
    template <class Msg>
    void Send(Msg msg)
    {
        if (Dispatcher::Inst.HasHandler<Msg>())
            Dispatcher::Inst.Send(msg);
    }
    void Update();
};

using TrafficLightController = Agent<TrafficLightSM>;
