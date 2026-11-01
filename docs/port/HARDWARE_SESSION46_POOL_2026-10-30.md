# Session46: one pose helper

Archived build/deko3d-test/pool-helper-session46/. Effective envPOOL_CORES1. Pool plan1helpercore2,creator/gamecore1,renderworker core0. No affinity failure in reported initialization. Prior45pool0. User gave no visual/subjective report.

Experiment is unfavorable in sampled windows: prepare10.2-13.15ms vs prior45~3.9-7.8ms; ticks30.6-33.7sec vs prior45~35.7-41.4 heavy samples. Game12-14.3ms,capture2.4-2.9ms,workerwaitnearzero. GPUcommand13-19ms toward end,scene workload not identical. Do not claim controlled causal percentage; nevertheless leave helper opt-in and recommend pooloff for current candidate.

Repeat skip decoder works(unread0),all checked heavy verdictsdifferent104-109,repeat0. Full ordered upload equality rejects useful skips. Need first-difference aggregates per transfer before optimization, not ignoring differences blindly.

Audio trace has~1000second wall elapsed but only~35-54seconds mixer and21-31game seconds in late windows, indicating long pause/suspension/clock discontinuity or trace lifetime effect; do not infer1000second sound drift. Exact cause unestablished. Reported mixer-vs-game drift14-23seconds is cumulative and not directly comparable to earlier routes without resets/pause context.

Next: use session45-tests/01-repeat-skip-live/env.txt on sameNRO to disablepool. No required repeat of same route merely to confirm env; if investigating controlled difference later use matchedcamera/clock route. Source next: profile parallel chunk scheduling/cache contention/atomic next+done+notify overhead,actual task counts and per-thread work; current pool assigns individualitems via two sharedatomicincrements and mutex notifications so synchronization/false-sharing may dominate tiny jobs. This is hypothesis,not proven. Consider coarsechunks/minimumtaskthreshold and serialfallback only after measurement,keep correctness/exceptions/join/reentrancy. Preserve vendor/cap fixes. No source or build changes in this analysis.
