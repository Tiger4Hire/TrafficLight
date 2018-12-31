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




