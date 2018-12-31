// requires : GL is initialised (before Update Called)

#include "glu_objs.h"
#include "Agent.h"
#include "Behaviors.h"
#include <array>
#include <chrono>
#include <optional>
#include <mutex>

template <class T>
constexpr int Max()
{
    return static_cast<int>(T::MAX);
}

class TrafficLight {
public:
    enum class State { GREEN, AMBER, RED, RED_AMBER, MAX };

    void Update();
    void Render() const;
    void Goto(State);
    State GetCurrent();
    void Deactivate() { deactivate = true; }

private:
    enum class Colour { RED, AMBER, GREEN, MAX };
    using TargetState = bool[Max<Colour>()];
    using StateValues = TargetState[Max<State>()];
    using TargetStateValues = std::array<float, Max<Colour>()>;
    static StateValues targets;
    float speed = .7f;
    GluSphere sphere;
    TargetStateValues state{0.f, 0.f, 1.f};
    State current{State::RED};
    std::atomic<State> target{State::RED};
    std::optional<std::chrono::high_resolution_clock::time_point> prev_update;
    bool deactivate{false};

    TargetStateValues ToVals(State state);
};

// Wait for something to do
using TrafficLightBehavior = Behavior<TrafficLight, int>;
using TrafficLightAll = All<TrafficLight, int>;
using TrafficLightAny = Any<TrafficLight, int>;

class Wait : public TrafficLightBehavior {
public:
    Result Update(int tgt, int& current, TrafficLight&) final;
    void Undo(int& current, TrafficLight&) final;
};

// Step SM forward
class Step : public TrafficLightBehavior {
    std::optional<TrafficLight::State> prev_state;
    TrafficLight::State target_state = TrafficLight::State::MAX;

public:
    Result Update(int tgt, int& current, TrafficLight&) final;
    void Undo(int& current, TrafficLight&) final;
};

class Enough : public TrafficLightBehavior {
public:
    Result Update(int tgt, int& current, TrafficLight&) final;
    void Undo(int& current, TrafficLight&) final;
};
// events
class ButtonPress {
};
class Undo {
};
// controller
class TrafficLightSM : AgentObject {
    TrafficLight& controlled_object;
    int num_button_presses{0};
    int num_state_changes{0};
    Wait wait_behavior;
    Step step_behavior;
    Enough enough_behavior;
    TrafficLightAll normal_behavior;
    TrafficLightAny complete_behavior;

public:
    TrafficLightSM(TrafficLight&);

    void Send(ButtonPress) { num_button_presses++; }
    void Send(Undo)
    {
        num_button_presses--;
        num_button_presses = std::max(num_button_presses, num_state_changes);
    }
    bool Update();
};

using TrafficLightController = Agent<TrafficLightSM>;