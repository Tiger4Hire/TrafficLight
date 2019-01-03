#include "standard.h"
#include "TrafficLights.h"
#include <gsl/gsl>
#include <numeric>
#include <GL/freeglut.h>

Dispatcher Dispatcher::Inst;

void TrafficLight::Render() const
{
    glPushMatrix();
    const GLfloat mat_amb_diff_colors[3][4] = {
        {1.0f, 0.0f, 0.0f, 0.5f}, {0.7f, 0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f, 0.5f}};
    GLfloat target_cols[3][4];
    using namespace std;
    static_assert(Max<Colour>() == size(mat_amb_diff_colors));
    for (int idx = 0; idx < size(state); ++idx)
        for (int col = 0; col < size(state); ++col)
            target_cols[idx][col] = mat_amb_diff_colors[idx][col] * state[idx];

    glPushMatrix();
    glTranslatef(0.0, 9.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    for (const auto& col : target_cols) {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);
        glTranslatef(0.0, -4.5, 0.0);
        sphere.Render(2.f, 100);
    }
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glPopMatrix();
}

void TrafficLight::Update()
{
    using namespace std;
    using namespace std::chrono;
    const auto time = high_resolution_clock::now();
    const float step = prev_update ? duration_cast<milliseconds>(time - prev_update.value()).count() / 1000.f : 0.f;
    prev_update = time;
    Lights tgt;
    transform(begin(target), end(target), begin(tgt), [](bool b) { return b ? 1.f : 0.f; });
    const auto converge_fn = [this, step](float a, float b) { return a + clamp(b - a, -step * speed, step * speed); };
    transform(begin(state), end(state), begin(tgt), begin(state), converge_fn);

    for (int idx = 0; idx < state.size(); ++idx) {
        if (state[idx] == tgt[idx] && callbacks[idx]) {
            callbacks[idx]();
            callbacks[idx] = nullptr;
        }
    }
}

void TrafficLight::Goto(Colour c, bool onOff, CallbackFn cb)
{
    int idx = static_cast<int>(c);
    callbacks[idx] = cb;
    target[idx] = onOff;
}

using Result = TrafficLightBehavior::Result;

void Send(TrafficLight& controlled_object, TrafficLight::Colour col, bool onOff)
{
    controlled_object.Goto(col, onOff, [=]() { Dispatcher::Inst.Send(LightChange{col, onOff}); });
}

Result Set::Update(int tgt, LocalState& crnt, TrafficLight& controlled_object)
{
    if (!sent) {
        Send(controlled_object, TrafficLight::Colour::RED, true);
        crnt.target_count = tgt;
    }
    sent = true;  // one shot behavior
    return SUCCESS;
}

void Set::Undo(LocalState& crnt, TrafficLight& controlled_object)
{
    controlled_object.Goto(TrafficLight::Colour::RED, false, nullptr);
}

Result Wait::Update(int tgt, LocalState& crnt, TrafficLight&)
{
    if (pressed)
        crnt.target_count++;
    pressed = false;
    active = !crnt.target_count;
    if (!active)
        return SUCCESS;

    std::cout << "waiting\n";
    crnt.timer = 20;
    return PENDING;
}

void Wait::OnEvent(const ButtonPress&)
{
    if (active) {
        std::cout << "Start sequence timer\n";
        pressed = true;
    }
}

Result WaitOnTimer::Update(int, LocalState& crnt, TrafficLight&)
{
    if (crnt.timer) {
        crnt.timer--;
        return PENDING;
    }
    else
        return SUCCESS;
}

void WaitOnConfirmation::OnEvent(const LightChange& change)
{
    changes.push_back(change);
}

Result WaitOnConfirmation::Update(int, LocalState& crnt, TrafficLight&)
{
    std::cout << crnt.confirmed.to_string() << "\n";

    for (const auto& change : changes)
        crnt.confirmed[gsl::narrow<int>(change.colour)] = change.new_state;
    changes.clear();
    return (crnt.sent != crnt.confirmed) ? SUCCESS : PENDING;
}

Result SetColour::Update(int tgt, LocalState& crnt, TrafficLight& controlled_object)
{
    if (crnt.confirmed == from) {
        const auto dist = to ^ from;
        for (int i = 0; i < Max<TrafficLight::Colour>(); ++i) {
            if (dist.test(i)) {
                TrafficLight::Colour col = static_cast<TrafficLight::Colour>(i);
                bool tgt_val = to.test(i);
                std::cout << std::boolalpha << "Setting color " << i << "-" << tgt_val << "\n";
                Send(controlled_object, col, tgt_val);
                crnt.timer = time_max;
            }
        }
        return PENDING;
    }
    else
        return SUCCESS;
}

Result Done::Update(int, LocalState& crnt, TrafficLight&)
{
    std::cout << "Done\n";
    crnt.target_count--;
    return SUCCESS;
}

TrafficLightSM::TrafficLightSM(TrafficLight& co) : controlled_object(co)
{
    all = set && timer && net_delay && wait && red2amberred && amberred2green && green2amber && amber2red && done;
}

void TrafficLightSM::Update()
{
    all.Update(num_button_presses, local_state, controlled_object);
}