# Session42: combined rendering experiment

Logs archived build/deko3d-test/rendering-session42. User reports slight subjective improvement, unsure FPS. Compare with session41 baseline cautiously: no identical camera/route evidence or clock trace.

Effective environment and worker initialization checked in log. Repeat-image skip stays inactive: all checked late-image paths not cleanly decoded; zero frames skipped by that experiment.

Later tick-phase samples40.8-52.3 ticks/sec, versus representative baseline heavy29-30 and lighter37-41. Do not call these displayed FPS: presents/tick0.58-0.85 in samples, and direct present intervals21.6-41.5ms (roughly24-46 presentations/sec). Some displayed frame counts/pacing tick counts are not aligned with tick-phase windows; investigate counters/undrawn ticks rather than asserting all ticks drawn.

Audio sector1555962 at26.43seconds after first sound: game20seconds, audio26.42seconds, drift6.42seconds. Baseline same wall interval: game14seconds/drift12.42seconds. Strong evidence faster game advancement for these recorded spans, but still desynchronized and not proof of matched-scene FPS gain.

GPU scene later samples10.5-13.2ms vs baseline heavy~19ms; workload differs, cannot attribute shader/GPU speedup. Queue gap variable~9.6-28.8ms remains. Worker wait nearlyzero; preparation/capture remains on game thread. Next unchanged candidate +03-rendering-audio env, same route to isolate incremental early-audio behavior. No build/source changes for analysis.
