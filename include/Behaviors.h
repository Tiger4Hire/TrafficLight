#pragma once
#include <vector>
#include <gsl/gsl>
/*
Templates for behaviors. Behaviors do something, until they suceed or fail.
All is a container for a collection of behaviors which succeed when they all
succeed. (&&) Any is a container for a collection of behaviors which succeed
when the first succeeds. (||)
*/

template <class Model, class Target, class Current = Target>
class Behavior {
public:
    Behavior() noexcept = default;
    Behavior(Behavior&&) = default;
    Behavior(const Behavior&) = default;
    Behavior& operator=(Behavior&&) = default;
    Behavior& operator=(const Behavior&) = default;

    enum Result { FAIL, PENDING, SUCCESS };
    virtual Result Update(Target tgt, Current& current, Model&) = 0;
    virtual void Undo(Current& current, Model&) = 0;
};

template <class Model, class Target, class Current = Target>
class All : public Behavior<Model, Target, Current> {
    using Child = Behavior<Model, Target, Current>;
    using Result = typename Child::Result;
    std::vector<Child*> children;  // non-owning pointers
    int watermark{-1};

public:
    void Add(Child& new_behavior) { children.push_back(&new_behavior); }

    Result Update(Target tgt, Current& crnt, Model& model) final
    {
        for (int idx = 0; idx < children.size(); ++idx) {
            watermark = std::max(watermark, idx);
            switch (children[idx]->Update(tgt, crnt, model)) {
                case Result::FAIL:
                    return Result::FAIL;  // parent must invoke undo
                case Result::PENDING:
                    PartUndo(idx, crnt, model);
                    return Result::PENDING;
                case Result::SUCCESS:
                    /* continue */
                    break;
            }
        }
        return Result::SUCCESS;
    }
    void Undo(Current& crnt, Model& model) final
    {
        for (int idx = 0; idx <= watermark; ++idx)
            children[idx]->Undo(crnt, model);
        watermark = -1;
    }

private:
    void PartUndo(int from, Current& crnt, Model& model)
    {
        for (int idx = from + 1; idx <= watermark; ++idx)
            children[idx]->Undo(crnt, model);
        watermark = from;
    }
};

template <class Model, class Target, class Current = Target>
class Any : public Behavior<Model, Target, Current> {
    using Child = Behavior<Model, Target, Current>;
    using Result = typename Child::Result;
    std::vector<Child*> children;    // non-owning pointers
    std::vector<bool> child_active;  // indicates has been updated and needs clean up

public:
    void Add(Child& new_behavior)
    {
        Expects(children.size() == child_active.size());
        children.push_back(&new_behavior);
        child_active.push_back(false);
        Ensures(children.size() == child_active.size());
    }

    Result Update(Target tgt, Current& crnt, Model& model) final
    {
        Expects(children.size() == child_active.size());
        for (int idx = 0; idx < children.size(); ++idx) {
            child_active[idx] = true;
            switch (children[idx]->Update(tgt, crnt, model)) {
                case Result::FAIL:
                    return Result::FAIL;  // parent must invoke undo
                case Result::PENDING:
                    break;
                case Result::SUCCESS:
                    UndoAllBut(idx, crnt, model);
                    return Result::SUCCESS;
            }
        }
        return Result::PENDING;
    }
    void Undo(Current& crnt, Model& model) final
    {
        for (int idx = 0; idx <= children.size(); ++idx)
            if (child_active[idx]) {
                children[idx]->Undo(crnt, model);
                child_active[idx] = false;
            }
    }

private:
    void UndoAllBut(int exception, Current& crnt, Model& model)
    {
        for (int idx = 0; idx <= children.size(); ++idx)
            if (idx != exception && child_active[idx]) {
                children[idx]->Undo(crnt, model);
                child_active[idx] = false;
            }
    }
};

template <class Model, class Target, class Current = Target>
All<Model, Target, Current> operator&&(All<Model, Target, Current>&& l, Behavior<Model, Target, Current>& r)
{
    All<Model, Target, Current> retval(l);
    retval.Add(r);
    return retval;
}

template <class Model, class Target, class Current = Target>
All<Model, Target, Current> operator&&(Behavior<Model, Target, Current>& l, Behavior<Model, Target, Current>& r)
{
    All<Model, Target, Current> retval;
    retval.Add(l);
    retval.Add(r);
    return retval;
}

template <class Model, class Target, class Current = Target>
Any<Model, Target, Current> operator||(Any<Model, Target, Current>&& l, Behavior<Model, Target, Current>& r)
{
    Any<Model, Target, Current> retval(l);
    retval.Add(r);
    return retval;
}

template <class Model, class Target, class Current = Target>
Any<Model, Target, Current> operator||(Behavior<Model, Target, Current>& l, Behavior<Model, Target, Current>& r)
{
    Any<Model, Target, Current> retval;
    retval.Add(l);
    retval.Add(r);
    return retval;
}
