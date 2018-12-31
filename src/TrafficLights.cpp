#include "TrafficLights.h"
#include <gsl/gsl>
#include <numeric>

namespace {
constexpr TrafficLight::State next_state[Max<TrafficLight::State>()] = {
    TrafficLight::State::AMBER, TrafficLight::State::RED,
    TrafficLight::State::RED_AMBER, TrafficLight::State::GREEN};
}  // namespace
TrafficLight::StateValues TrafficLight::targets = {{false, false, true},
                                                   {false, true, false},
                                                   {true, false, false},
                                                   {true, true, false}};

void TrafficLight::Render() const
{
    glPushMatrix();
    glTranslatef(0.0, 9.0, 0.0);
    const GLfloat mat_amb_diff_colors[3][4] = {{1.0f, 0.0f, 0.0f, 0.5f},
                                               {0.7f, 0.5f, 0.0f, 0.5f},
                                               {0.0f, 1.0f, 0.0f, 0.5f}};
    GLfloat target_cols[3][4];
    using namespace std;
    static_assert(Max<Colour>() == size(mat_amb_diff_colors));
    for (int idx = 0; idx < size(state); ++idx)
        for (int col = 0; col < size(state); ++col)
            target_cols[idx][col] = mat_amb_diff_colors[idx][col] * state[idx];

    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    for (const auto& col : target_cols) {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);
        glTranslatef(0.0, -4.5, 0.0);
        sphere.Render(2.f, 100);
    }
    glPopMatrix();
}

TrafficLight::TargetStateValues TrafficLight::ToVals(State state)
{
    TargetStateValues retval;
    using namespace std;
    const auto to_float = [](bool v) { return v ? 1.f : 0.f; };
    const auto target_span = gsl::make_span(targets);
    const auto colour_span =
        gsl::make_span(target_span[static_cast<int>(state)]);
    transform(begin(colour_span), end(colour_span), begin(retval), to_float);
    return retval;
}

void TrafficLight::Step()
{
    target = next_state[static_cast<int>(target)];
}

void TrafficLight::Update()
{
    using namespace std;
    using namespace std::chrono;
    const auto time = high_resolution_clock::now();
    const float step =
        prev_update
            ? duration_cast<milliseconds>(time - prev_update.value()).count() /
                  1000.f
            : 0.f;
    prev_update = time;
    const TargetStateValues tgt = ToVals(target);
    const auto converge_fn = [this, step](float a, float b) {
        return a + clamp(b - a, -step * speed, step * speed);
    };
    transform(begin(state), end(state), begin(tgt), begin(state), converge_fn);
    if (tgt == state)
        current = target;
}
