#include "standard.h"
#include "TrafficLights.h"
#include <gsl/gsl>
#include <numeric>
#include <GL/freeglut.h>

namespace {
using State = TrafficLight::State;
#define STRINGIFY(X) \
    case State::X:   \
        return #X
std::string to_string(State v)
{
    switch (v) {
        STRINGIFY(GREEN);
        STRINGIFY(AMBER);
        STRINGIFY(RED);
        STRINGIFY(RED_AMBER);
        default:
            return "BAD_VALUE";
    }
}
constexpr State next_state[Max<State>()] = {State::AMBER, State::RED,
                                            State::RED_AMBER, State::GREEN};
State Next(State prev)
{
    const auto current = static_cast<int>(prev);
    return next_state[current];
}

void Output(const std::string& v)
{
    glutStrokeString(GLUT_STROKE_ROMAN,
                     reinterpret_cast<const unsigned char*>(v.c_str()));
}

int Width(const std::string& v)
{
    const auto fn = [](int v, char c) {
        return v + glutStrokeWidth(GLUT_STROKE_ROMAN,
                                   static_cast<unsigned char>(c));
    };
    return std::accumulate(std::begin(v), std::end(v), 0, fn);
}
}  // namespace
TrafficLight::StateValues TrafficLight::targets = {{false, false, true},
                                                   {false, true, false},
                                                   {true, false, false},
                                                   {true, true, false}};

void TrafficLight::Render() const
{
    glPushMatrix();
    const GLfloat mat_amb_diff_colors[3][4] = {{1.0f, 0.0f, 0.0f, 0.5f},
                                               {0.7f, 0.5f, 0.0f, 0.5f},
                                               {0.0f, 1.0f, 0.0f, 0.5f}};
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

    glPushMatrix();
    glColor4f(1.f, 1.f, 1.f, 1.f);
    std::string text = to_string(current);
    glTranslatef(-Width(text) / (152.38 * 2), 8, 0);
    glScalef(1 / 152.38, 1 / 152.38, 1 / 152.38);
    Output(text);
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

void TrafficLight::Update()
{
    const State target_copy = target;
    using namespace std;
    using namespace std::chrono;
    const auto time = high_resolution_clock::now();
    const float step =
        prev_update
            ? duration_cast<milliseconds>(time - prev_update.value()).count() /
                  1000.f
            : 0.f;
    prev_update = time;
    const TargetStateValues tgt = ToVals(target_copy);
    const auto converge_fn = [this, step](float a, float b) {
        return a + clamp(b - a, -step * speed, step * speed);
    };
    transform(begin(state), end(state), begin(tgt), begin(state), converge_fn);
    if (tgt == state)
        current = target_copy;
}

TrafficLight::State TrafficLight::GetCurrent()
{
    return current;
}

void TrafficLight::Goto(State s)
{
    target = s;
}

TrafficLightSM::TrafficLightSM(TrafficLight& t) : controlled_object(t) {}

void TrafficLightSM::Update()
{
    // update changes, if one seen
    if (target && target.value() == controlled_object.GetCurrent()) {
        num_state_changes++;
        target.reset();
    }
    // select new target, if one requested
    if (!target && num_state_changes != num_button_presses) {
        target = Next(controlled_object.GetCurrent());
        controlled_object.Goto(target.value());
    }
}
