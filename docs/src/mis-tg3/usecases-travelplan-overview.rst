Supply Chain Optimization Overview
====================

The supply chain optimization problem can be formulated as a *travel plan* scheduling problem in the following way.

Consider a list of travel plans or trips that need to be completed. Additionally, consider two types of *resources*
(1) Trucks and (2) Work Orders, that are used to execute a travel plan. The use case is constrained by the following
specifications:

#. There are a fixed number of trucks
#. Each travel plan is assigned a truck (not necessarily unique)
#. Number of trucks << Number of travel plans
#. There are a fixed number of work orders to be completed
#. Each travel plan is associated with one or more work orders
#. A work order might need more than 1 trip to be completed
#. Number of work orders << Number of travel plans

Finally, the resource conflict is modelled as:
**If two travel plans share a truck or a work order, they can not be executed at the same time.**

Problem Formulation
-------------------

In this example, the goal is to execute all travel plans in the least amount of time. To accomplish this, it can be
stated that for a list of travel plans that are yet to be scheduled, we need to find the maximum number of travel plans
that can be scheduled at the same time, given the set of available trucks and work orders to complete. When a set of
scheduled travel plans are finished, the process is repeated until all travel plans are executed.

Consider a scenario with 3 travel plans, 2 trucks and 2 work orders. The travel plans have dependencies on trucks
and work orders as shown in the following figure:
