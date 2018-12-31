// requires : GL is initialised (before Update Called)

#include "glu_objs.h"
#include "Agent.h"
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
    TargetStateValues ToVals(State state);
};


class Behavior {
public:
    Behavior() noexcept = default;
    Behavior(Behavior&&) = default;
    Behavior(const Behavior&) = default;
    Behavior& operator=(Behavior&&) = default;
    Behavior& operator=(const Behavior&) = default;

    enum Result { FAIL, PENDING, SUCCESS };
    virtual Result Update(int tgt, int& current, TrafficLight&) = 0;
    virtual void Undo(int& current, TrafficLight&) = 0;
};

// Wait for something to do
class Wait : public Behavior {
public:
    Result Update(int tgt, int& current, TrafficLight&) final;
    void Undo(int& current, TrafficLight&) final;
};

// Step SM forward
class Step : public Behavior {
    std::optional<TrafficLight::State> prev_state;
    TrafficLight::State target_state = TrafficLight::State::MAX;

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
    bool was_stepping{false};
public:
    TrafficLightSM(TrafficLight&);

    void Send(ButtonPress) { num_button_presses++; }
    void Send(Undo)
    {
        num_button_presses--;
        num_button_presses = std::max(num_button_presses, num_state_changes);
    }
    void Update();
};

using TrafficLightController = Agent<TrafficLightSM>;