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

Result Set::Update(int tgt, int& crnt, TrafficLight& controlled_object)
{
    if (!sent) {
        controlled_object.Goto(TrafficLight::Colour::RED, true, [this]() { done = true; });
        crnt = tgt;
        sent = true;
    }
    if (done)
        return SUCCESS;
    return PENDING;
}

void Set::Undo(int& crnt, TrafficLight& controlled_object)
{
    controlled_object.Goto(TrafficLight::Colour::RED, false, nullptr);
}

Result Wait::Update(int, int& current, TrafficLight&)
{
    active = count == current;
    if (!active)
        return SUCCESS;

    if (timer) {
        timer.value()++;
        if (timer.value() > 20) {
            count++;
            timer.reset();
            return SUCCESS;
        }
    }
    return PENDING;
}

void Wait::OnEvent(const ButtonPress&)
{
    if (active && !timer) {
        std::cout << "Start my timer\n";
        timer = 0;
    }
}

Result SetColour::Update(int, int&, TrafficLight& controlled_object)
{
    const auto dist = to ^ sent;
    for (int i = 0; i < Max<TrafficLight::Colour>(); ++i) {
        if (dist.test(i)) {
            timer = 0;
            TrafficLight::Colour col = static_cast<TrafficLight::Colour>(i);
            bool tgt_val = to.test(i);
            std::cout << std::boolalpha << "Setting color " << i << "-" << tgt_val << "\n";
            controlled_object.Goto(col, tgt_val, [this, tgt_val, i]() { current.set(i, tgt_val); });
        }
    }
    sent = to;
    timer = std::min(timer + 1, time_max);
    if (timer < time_max)
        return PENDING;

    return (current != to) ? PENDING : SUCCESS;
}

void SetColour::Undo(int&, TrafficLight&)
{
    sent = from;
    current = from;
}

Result Done::Update(int, int& current, TrafficLight&)
{
    std::cout << "Done\n";
    current++;
    return SUCCESS;
}

TrafficLightSM::TrafficLightSM(TrafficLight& co) : controlled_object(co)
{
    all = set && wait && red2amberred && amberred2green && green2amber && amber2red && done;
}

void TrafficLightSM::Update()
{
    all.Update(num_button_presses, num_state_changes, controlled_object);
}