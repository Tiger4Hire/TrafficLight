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

using LightSetting = std::bitset<Max<TrafficLight::Colour>()>;
struct LocalState {
    int target_count;
    LightSetting sent;
    LightSetting confirmed;
    int timer{0};
};
using TrafficLightBehavior = Behavior<TrafficLight, int, LocalState>;
using TrafficLightAll = All<TrafficLight, int, LocalState>;
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
    Result Update(int tgt, LocalState& crnt, TrafficLight&) final;
    void Undo(LocalState& crnt, TrafficLight&) final;
};

class Wait : public TrafficLightBehavior, Handle<ButtonPress> {
    bool active{false};
    bool pressed{false};
    int count{0};
    void OnEvent(const ButtonPress&) final;

public:
    Result Update(int tgt, LocalState& crnt, TrafficLight&) final;
    void Undo(LocalState& crnt, TrafficLight&) final {}
};

class WaitOnTimer : public TrafficLightBehavior {
public:
    Result Update(int tgt, LocalState& crnt, TrafficLight&) final;
    void Undo(LocalState& crnt, TrafficLight&) final {}
};

class WaitOnConfirmation : public TrafficLightBehavior, Handle<LightChange> {
    void OnEvent(const LightChange&) final;
    std::vector<LightChange> changes;

public:
    Result Update(int tgt, LocalState& crnt, TrafficLight&) final;
    void Undo(LocalState& crnt, TrafficLight&) final {}
};

class SetColour : public TrafficLightBehavior {
    const LightSetting to;
    const LightSetting from;
    const int time_max;

public:
    SetColour(LightSetting f, LightSetting t, int time) : to(t), from{f}, time_max{time} {}
    Result Update(int tgt, LocalState& crnt, TrafficLight&) final;
    void Undo(LocalState& crnt, TrafficLight&) final {}
};

class Done : public TrafficLightBehavior {
    Result Update(int tgt, LocalState& crnt, TrafficLight&) final;
    void Undo(LocalState& crnt, TrafficLight&) final {}
};
// controller
class TrafficLightSM : AgentObject {
    TrafficLight& controlled_object;
    int num_button_presses{0};
    LocalState local_state{0, 0, 0};
    Set set;
    Wait wait;
    WaitOnTimer timer;
    WaitOnConfirmation net_delay;
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
        local_state.target_count--;
        local_state.target_count = std::max(local_state.target_count, num_button_presses);
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
