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

class ButtonPress {
};
class TrafficLightSM : AgentObject {
    TrafficLight& controlled_object;
    int num_button_presses{0};
    int num_state_changes{0};
    std::optional<TrafficLight::State> target;
public:
    TrafficLightSM(TrafficLight&);

    void Send(ButtonPress) { num_button_presses++; }
    void Update();
};

using TrafficLightController = Agent<TrafficLightSM>;