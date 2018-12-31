Traffic Lights Example Code
Demonstation of the problems of using state machines in concurrent code, and possible solutions. 

Problem Definition
Traffic lights are probably the simplest practicle example of a state machine you could think of.
As long as state transition is instantaneous, state machines work really well. The key point is that the
"state" of the system is defined as the state of the "state machine". The problems arise, becuase state
machines are not really any good at "reflecting" state. That is, while the "state" is whatever we tell it
to be there is no problem. However, when "behaviours" (what a state machine does), takes time, the state
machine starts to serve two purpose. It drvies the "state" and it reflects the state. It is both input
and output. This always causes issues. In a asyncronous system, the true state of the system can never
be truely known, and so this problem always arises.

Demo 1
Here we introduce the problem in it's simplest form. We are modelling a form of lighting that "warms" up
and "warms down", thus state transitions are not instantaneous. Here we can easily see that by pressing
the space bar to cycle between states, it is possible to skip a state entirely. This "works" in our
demo, but the problem is already baked-in. The whole point of a state-machine is to allow the designer
of the system to reduce the complexity of the system. They can look at each state in isolation, only 
considering the limited changes that my occur in that state. By breaking this assumption, we have 
rendered the state machine virtually pointless.

Demo 2
By providing an "agent" we have solved the issue, but we have done so by introducing a syncronization
point between the two systems. We are not truely asyncronous at this point.

Demo 3
By removing the mutex and using an atomic, our solution is now "lock-free". However, we now have the
reverse problem of that we started with. We can now "loose" button presses. The race condition is
between the controller and the internal update. If we update the controller, just as the light object
is updating, our "step to next", calculates the new value against the old one.
The next demo will expose the problem.

Demo 4
By the addition of a sleep, right next to the race condition, we expose the bug. This version is totally
artificial, but the problem is now visible.
A common solution to this problem is to duplicate the state of the asyncronous object in the controller.
This solution has issues though, it embeds a model of the "model" in the controller, which is obviously
a weakening of encapsulation. (Functional dependence)
Instead, a better solution is to go "goal oriented". In goal oriented, we store only a "target state"
and the controller becomes an (stateless) encapsulation of the problem, "given I am at A, how do I get B"
where A is the state of the controlled object and B is some target passed in by an external system.

Demo 5
An extremely simple goal oriented system. The target state is simply a count of the changes requested.
The controller logic consists of two pieces of logic
1. If a change was requested, wait for it to happen.
2. Else if a change is required request one.

Demo 6
(includes bug fixes to exit handling)
Now things are geting interesting. We encapsulated the two behaviors outlined above as seperate
objects. Each object represents a behavior. A behavior represents a chunk of meaningful action
that takes us towards our target. Notice the error handling is built into the design. Each
behavior may FAIL, not used in this design. PENDING menas the action is not complete yet.
Notice, there is a natural structure forming. It's a little forced right now, but the idea that
behavior has an "Undo" means that it can be reasoned with - as a transaction. This is one of the
major benifits of behaviors.
Using the "Wait" behavior to trigger the undo of the transition is also one of the major features
of BTs. Preceeding states can be used as preconditions for later ones, simplifying thier design
requirements. Also, each behavior is meaningful in it's own right, making them both testable and
reusable.

Demo 7
A completely contrived example, consider the example that we want to model the traffic lights
have problem that they brake after 10 cycles. We can add the behavior "Enough". Enough counts
to 10, and then blocks the other behaviors`.
This is an example of an "Any" behavior (Select). It models conditions that are independent.
Like "All" (Sequence) behaviors, the order of declaration is important, as the first condition
that succeeds (or fails) will block the others.  

