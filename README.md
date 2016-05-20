# MOBOO : Multi-Objective Build Order Optimizer

First thought for use in StarCraft: Brood War AI, the MOBOO searches
for series of tasks from a given system in order to find the best
options for a given strategy. To do so, the MOBOO needs three inputs:
* System
* Initial State
* Strategy

## System
Defines (a) the number of **resources** and their relations between each
other; (b) the number of **tasks** and their costs, requirements and so
on; (c) the number of **cyclic events** and their productions.

## Initial State
Defines the number of resources collected and used at the initial
point of optimization.

## Strategy
Defines the additional objective functions, and additional restrictions
that need to be applied to the original model.
