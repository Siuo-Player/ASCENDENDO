# ASCENDENDO difficulty model

## Status

The authoritative mechanical validator answers **whether a level is valid and mechanically reachable**. It does not define player difficulty.

The control-space analysis from PR #343 is retained as a diagnostic feature extractor. It is not the final difficulty scale.

## Player/session model

Difficulty experiments model complete gameplay sessions with player profiles rather than selecting one optimal jump. A profile includes hypotheses for reaction time, execution error, braking error, aiming error, route noise, consistency and learning rate.

The experiment records attempts, progress, elapsed time, success/failure and failure reason. Learning is represented as a state transition between failed attempts.

The current profile parameters and stochastic session proxy are **research scaffolding only**. They must not be presented as human-validated difficulty measurements.

## Calibration path

1. Keep mechanical validity separate from difficulty.
2. Keep geometric/control-space features as diagnostics.
3. Replace the temporary session proxy with a full-episode controller using the actual movement simulator.
4. Run multiple seeds and player profiles over complete levels and retries.
5. Record clear rate, deaths, progress, time, retry count, route choices and learning curves.
6. Collect human telemetry using the same schema.
7. Calibrate player profiles and level difficulty against the human data.
8. Only then expose a calibrated scalar difficulty estimate.

Until step 6 is available, reports should distinguish **simulated difficulty** from **human-calibrated difficulty** and include confidence/coverage rather than pretending the scale is ground truth.

## Campaign use

Campaign progression should be evaluated from the resulting skill/completion curves and learning pattern, not by forcing individual levels into arbitrary score thresholds. A good campaign may contain deliberate recovery levels and challenge peaks.
