# Hardware session47 serial pool/upload diagnostics

Archived build/deko3d-test/serial-stats-session47/. Effective envPOOL_STATS120,helperoff,pool0. Correct serial config. No subjective/visual report or independent clock verification.

Representative pose stats:65-95pieces/call~28-38us/piece,total1.95-3.42ms; late heavier165.7pieces43.2us,total7.15ms;163.5pieces40.5us,total6.62ms. Actual workcount varies substantially, so compare helper cost per piece and similar classes/workload rather than whole-windowms alone. Stageprepare includes work beyond pool.

Sampled ticks~37-45 in lighter windows;~30 in heavierlatewindows. Presentation/tick1.00. Not a controlled performance comparison. GPU also needs per-pass context in heavy windows; don't assume purely CPU bound throughout.

Upload first difference overwhelmingly bytes-only(~105-113/window),fewplace/count;order-only0in selectedwindows. Block0x3317 CT32 64x64 changes114/114 or117/117,distinct8cap;laterblock0x333f CT32 64x64 changes112/112. Most listed HUD/palette destinations stable in thosewindows. This isolates changing64x64content as a major whole-list-equality blocker. Actual semantic purpose unestablished; do not call it HUD or discard it. Need identify ownership/material/sampling before transfer-only skip design. Aggregatekeys omitcorners/dbw and persist acrosslevels,so not standaloneidentityproof.

Next sameNRO+session46-tests/02-one-helper-stats/env.txt,matchedroute/camera,stockclocks640x480. Compare actualpieces/call,caller/helperus,piecesplit,startlatency,tailwait. Hypothesismalloclockstillunproven. No sourcechanges/build for this analysis.
