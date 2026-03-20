# Unreal Multithreading Demo

## Overview

This project is a practical Unreal Engine multithreading sandbox built to explore how far gameplay-side movement work can be pushed off the game thread, where the real bottlenecks appear, and how different choices affect total frame cost.

The project was inspired by the "Road to 60 fps in *The Witcher 4* UE5 tech demo" talk shown at Unreal Fest:

- https://www.youtube.com/watch?v=ji0Hfiswcjo

After watching that presentation, the goal here was not to recreate that work directly, but to build a personal testbed for learning Unreal Engine systems in a hands-on way while ramping up on the engine. The focus was to take a concrete use case with large numbers of movers updating every frame, and use it in conjunction with Unreal Insights to understand the practical tradeoffs between:

- game-thread actor movement
- worker-thread calculation
- batching strategies
- subsystem-driven management
- instanced rendering with `UHierarchicalInstancedStaticMeshComponent`

## Project Goal

The original objective was straightfoward:

- Create a large number of moving objects, offload movement calculation from the game thread, and compare implementation strategies in a way that is easy to inspect in Unreal Insights.

That goal evolved into a broader performance and architecture exercise:

1. Build a baseline actor/component-driven mover system.
2. Move calculation work to an `FRunnable` worker.
3. Introduce batching and optional `ParallelFor` processing.
4. Profile the system to identify the actual bottlenecks.
5. Migrate to a manager-owned instanced mesh approach when profiling showed that actor/component transform updates were far more expensive than the math itself.

## High-Level Architecture

The current project architecture is centered around a few core pieces.

### `UMoverWorldSubsystem`

The world subsystem acts as the runtime manager for mover data. It owns the active mover array, builds work snapshots, submits jobs to the worker, drains results, and applies updated transforms back to the instance manager.

Responsibilities:

- owns mover state as plain data structs
- ticks every frame
- builds movement jobs from active movers
- submits work to the background worker
- applies completed movement results on the game thread
- coordinates batching mode selection

This subsystem is also where the project instrumentation lives for Unreal Insights, including frame markers and counters for mover count, batch count, chunk size, in-flight jobs, and applied results.

### `FMoverWorker`

The worker is implemented with `FRunnable` and a dedicated `FRunnableThread`.

Responsibilities:

- waits on an event when idle
- consumes pending movement batches from a thread-safe queue
- computes new positions from plain data only
- produces results into a completed-results queue
- optionally uses `ParallelFor` for large enough batches

Important rule:

The worker never touches `UObject`, `AActor`, `UActorComponent`, or scene state directly. It only operates on copied data snapshots. All engine-facing transform application remains on the game thread.

### `AMoverInstanceManager`

The instance manager owns a single `UHierarchicalInstancedStaticMeshComponent` and is responsible for the visual representation of movers.

Responsibilities:

- stores the shared mesh representation for all movers
- adds instances in bulk
- applies batched transform updates
- avoids per-actor movement overhead

This replaced the original actor-per-mover setup after profiling showed that the worker math was inexpensive, while per-actor transform updates and related engine calculations were dominating the frame.

### `AMoverBenchmarkSpawner`

The benchmark spawner exists to make runtime testing easy.

Responsibilities:

- request a flexible number of movers at runtime
- generate random origins, destinations, and movement speeds
- submit spawn batches into the subsystem
- clear movers between runs
- randomize destinations for repeated tests

This makes it easy to run any number of movers without manually placing content in a level.

### `UMoverSpawnerWidget`

The widget provides a simple runtime control surface.

Responsibilities:

- request new mover batches from the UI
- clear movers
- randomize destinations
- make the test interactive without editor-only controls

### `ADemoPlayerController`

The player controller wires the runtime demo together.

Responsibilities:

- creates the user widget
- finds the benchmark spawner in the world
- binds Enhanced Input actions for quick testing

## Evolution of the Demo

### Phase 1: Actor-Based Movers

The first implementation used many actors, each with a mover component containing:

- current location
- destination
- move speed

A manager component gathered movers, generated jobs, and the worker computed updated positions for each frame.

This proved the threading model worked, but profiling showed the real issue clearly: applying new positions back to thousands of actors on the game thread was expensive.

### Phase 2: Batching and Parallel Processing

The next step introduced queue-based batching and optional `ParallelFor` execution inside the worker.

Multiple processing modes were supported:

- Per Mover
- Fixed Chunk Linear
- Fixed Chunk + ParallelFor
- "Smart" Auto

These modes were made configurable so they could be compared directly in Unreal Insights. Batch mode and chunk size settings can be changed from inside the `Project Settings` -> `Mover Threading Settings` section.

This phase demonstrated that background movement calculation stayed inexpensive even at large counts. The worker spent far more time waiting for jobs than doing actual work, but of course did not solve the issue with the cost of applying the results on the game thread.

### Phase 3: Profiling-Driven Rewrite to HISM

Unreal Insights made the next bottleneck obvious: `DrainResults` and transform application were consuming far more frame time than the worker.

That led to the migration from many actors/components to a manager-owned `UHierarchicalInstancedStaticMeshComponent`.

This changed the system from:

- moving thousands of scene components individually

into:

- updating plain mover data
- applying batched instance transforms to one shared component

This change significantly reduced game thread's frame cost, although it's really only applicable to scenarios where visual variety is limited. All collisions and most lighting options were disabled to further reduce general overhead for the demo.

Example of an Insights frame with 1850 movers using "smart auto" mode with the default settings:
<img width="1224" height="287" alt="image" src="https://github.com/user-attachments/assets/edd8326c-6ef5-4e7a-9541-ed3631e3e626" />

## Profiling and Unreal Insights

A core part of the project is its profiling-first approach.

The system includes trace markers and counters to make comparisons visible in Unreal Insights. That includes markers around:

- subsystem tick
- snapshot/job submission
- worker processing
- result draining
- transform application
- linear versus `ParallelFor` batch processing

This made it possible to compare batching modes directly and, more importantly, to identify that the biggest cost had moved from worker-side math to game-thread transform application.

That discovery shaped the rewrite to the instanced mesh approach.

## Key Technical Themes

### Safe Threading Boundaries

All worker-thread processing is data-only. Engine object mutation stays on the game thread.

### Queue-Based Work Submission

Pending movement batches and completed results are passed through queues to decouple simulation from application.

### Configurable Batching

Batching strategies are configurable so the project can serve as both a technical experiment and a profiling demo.

### Data-Oriented Runtime Representation

The final version stores movers as data records with instance indices instead of full actors.

### Runtime-Driven Benchmarking

The spawner and widget make it easy to scale tests up and down instantly during runtime.

## Current State

The project currently includes:

- a dedicated `FRunnable` worker for movement jobs
- queue-driven job/result flow
- configurable batching modes
- subsystem-based runtime management
- runtime spawning and destination randomization
- a manager-owned `UHierarchicalInstancedStaticMeshComponent`
- Unreal Insights trace markers and counters for analysis
- a simple UI and controller-driven runtime test harness

## Future Considerations

Potential future investigations:

- comparing `FRunnable` against Unreal task-based alternatives
- extending the mover data to support more complex simulation work
- exploring instance removal/remapping strategies
- comparing `UInstancedStaticMeshComponent` and `UHierarchicalInstancedStaticMeshComponent`
