# Sound Features FAQ

This document covers Sound Doctor, Audible Meter, and the Accessible Volume Console in Accessible Studio 1.1.3.

## Overview

### What is the difference between these features?

**Audible Meter** continuously watches active sources and communicates level problems through tones and screen-reader announcements. **Accessible Volume Console** is a keyboard-accessible way to inspect and change OBS source volume, output, and monitoring. **Sound Doctor** runs a separate diagnostic session, analyzes level statistics, and offers optional compressor and limiter filters. Audible Meter and Sound Doctor cannot run at the same time.

### Do these features record, transmit, or retain my audio?

No. They read numerical levels supplied by OBS; they do not capture an audio waveform, create an audio recording, send audio over the network, or write measurement history or reports to disk. Session statistics use fixed-size memory and are discarded when the feature stops. Only ordinary preferences are retained: Advanced Sound Settings and any source identities placed on the pre-fader stop list.

## Sound Doctor

### What exactly does Sound Doctor do?

Start it with Ctrl+Shift+D or from **Tools > Accessible Studio > Audio Tools > Sound Doctor**. After confirmation it watches active audio sources for 120 seconds while you use them normally. A source must provide at least 45 seconds of active signal to qualify for a processing recommendation. If streaming or recording is still active after 120 seconds, measurement continues and the report waits until both have stopped.

For each source, Sound Doctor keeps fixed-size distributions of RMS level, peak observations, active time, source identity, initial fader level, and a fingerprint of the filter chain. It also inspects existing compressor and limiter settings. It never listens for words, recognizes content, or sends measurements to an AI service.

### How does it decide that compression may help?

Compression is considered only for source types that are likely to be live inputs, such as microphone and direct audio-input captures. Sound Doctor compares the median active RMS level—the typical passage—with the 95th-percentile RMS level—the louder passage. Their difference is the measured dynamic spread.

It recommends a compressor when all of these are true:

- the source supplied at least 45 seconds of active audio;
- its fader or filter chain did not change during the test;
- it is a likely live-input source;
- no compressor, enabled or disabled, already exists; and
- the dynamic spread meets the configured minimum, 8 dB by default.

The proposed threshold is the median plus 5 dB, rounded and limited to the range −24 through −10 dB. Attack is 6 ms, release is 100 ms, and output gain is 0 dB. The ratio adapts to the spread: 2:1 below 10 dB, 2.5:1 from 10 dB to below 14 dB, and the configured maximum at 14 dB or more. The default maximum is 3:1.

### Can Sound Doctor reliably tell speech from music?

No. The available level measurements describe loudness and dynamics, not semantic content. Source type is used only as a likely-live-input heuristic. A microphone can carry music and an application capture can carry speech. For that reason, the report explicitly advises leaving a compressor recommendation unselected when the source is primarily music.

### When does it recommend a limiter?

By default, it recommends a limiter for every likely live-input source that supplied at least 45 seconds of active audio, did not change during the test, and has no existing limiter. It does not require observed clipping or peaks near the ceiling: this is preventive brick-wall protection against an unexpected future peak. The proposed limiter is last in the filter chain, with a default ceiling of −3 dBFS and a 60 ms release. Audio below the ceiling is unaffected by the limiter.

The advanced scope setting can extend limiter recommendations to every sufficiently active audio source. This may be useful for aggressive safety policy, but it can be unnecessary for mastered music or media that already has controlled peaks.

### Are compressor and limiter recommendations linked?

No. They are evaluated independently. A source may receive neither recommendation, one, or both. A compressor controls dynamic variation; a final limiter provides a hard peak ceiling.

### What else appears in the report?

Sound Doctor reports existing compressor and limiter settings, warns when a limiter is not last, identifies repeated pre-fader peaks near clipping, and explains when evidence was insufficient or the source changed. A filter cannot repair audio that was clipped before reaching OBS, so pre-fader clipping calls for lower device or application input gain.

### Does Sound Doctor change anything automatically?

Not merely by running. Every recommendation has an initially unchecked **Apply this change automatically** checkbox. **Finish** applies only checked items. Before applying, Sound Doctor rechecks the source, its volume, and its filter chain; stale recommendations are skipped. New filters have visible Sound Doctor names, are inserted in the intended order, and participate in OBS Undo and Redo. Escape closes the report without applying anything.

### Can I change its recommendation policy?

Yes. Open **Tools > Accessible Studio > Audio Tools > Advanced Sound Settings**, select the **Sound Doctor** page, and use:

- **Minimum dynamic variation for compression:** 6–15 dB; default 8 dB. Lower values recommend compression more often.
- **Maximum recommended compressor ratio:** 2.5:1, 3:1, or 3.5:1; default 3:1. The adaptive calculation can still select 2:1 or 2.5:1.
- **Sources eligible for limiter recommendations:** likely live inputs only by default, or all active audio sources.
- **Recommended limiter ceiling:** −12 through −1 dBFS; default −3 dBFS.

The 120-second session and 45-second minimum active-signal requirement are intentionally fixed. Apply saves without closing, OK saves and closes, and Cancel discards changes since the last Apply.

## Audible Meter and automatic level correction

### What does Audible Meter monitor?

Ctrl+I starts or stops one windowless Audible Meter mode. It follows all currently active audio sources. It uses post-fader peak for output warnings, pre-fader peak for eligible live captures, and RMS summaries for typical-level commands. It keeps operating whether or not OBS is streaming or recording.

### What does the automatic warning beep mean?

The high automatic warning tone means at least one source has spent the configured amount of actual time at or above the output warning level. By default that time is 1.5 seconds. The default level follows OBS's meter mode: −9 dBFS with sample peak or −2 dBFS with true peak. Brief dips shorter than one second do not fully reset accumulated exposure; one second below the boundary rearms the warning. The tone is a warning, not a precise measurement, and the screen reader announces the most problematic source.

The lower warning tone has a different meaning: after you agreed to adjust an eligible input, its pre-fader signal is still in OBS's red zone. Lower the gain at the microphone, audio interface, Windows input, or source application. Moving the OBS fader cannot repair pre-fader distortion.

### What do the two tones in Accessible Volume Console mean?

While Audible Meter is active and the Console is open, the focused source produces a middle tone in OBS's yellow output zone and a higher tone in the red output zone. Green, silence, an unavailable source, or focus outside a source control produces no tone. These are immediate measurement tones, not delayed warnings. They remain active even if automatic warnings are turned off with I.

Zone boundaries follow OBS's peak mode. With sample peak, yellow begins at −20 dBFS and red at −9 dBFS. With true peak, yellow begins at −13 dBFS and red at −2 dBFS.

### Will Audible Meter lower my sound on its own?

No. An output warning only informs you. While its high warning tone is sounding, Shift+I explicitly requests a correction. Audible Meter then considers every currently responsible source. For each, it calculates a target equal to the warning level minus the safety margin, compares that target with the highest observed peak in the incident, rounds the required reduction upward to the next 0.5 dB, and lowers the OBS fader by no more than the configured maximum. It never raises a source and never changes a source merely for a pre-fader warning.

Defaults are a 3 dB safety margin and a maximum 12 dB reduction per correction. Automatic correction is conservative peak protection; it does not balance speech, music, gameplay, and effects artistically.

### Can I undo an automatic correction?

Yes. Ctrl+Shift+I restores the most recent Shift+I correction. A source is restored only if its fader still matches the automatically set value; sources changed afterward are deliberately skipped. Starting or stopping Audible Meter, changing scene collection, or saving new meter settings clears this one-level restoration record. OBS's own controls remain available as well.

### How can I change Audible Meter behavior?

On the **Audible Meter** page of Advanced Sound Settings you can change:

- **Output warning level:** −30 through 0 dBFS; default −9 dBFS for sample peak or −2 dBFS for true peak.
- **Time above warning level:** 0.1–30 seconds; default 1.5 seconds.
- **Peak safety margin:** 0–12 dB; default 3 dB.
- **Maximum automatic reduction:** 0.5–30 dB; default 12 dB.
- **Pre-fader stop list:** remove saved source opt-outs so those unchanged inputs are checked again.

These settings do not alter the fixed OBS green/yellow/red boundaries, Console measurement tones, H/J/K/L readings, or the pre-fader red boundary.

### Can I temporarily silence warnings?

While Audible Meter is active, press I to toggle automatic input and output warnings. Re-enabling them starts fresh exposure timing. This does not turn off Console measurement tones. Letter commands are never intercepted while focus is in an editable control.

### What are H, J, K, and L for?

- H reports the current post-fader level and zone of the source most recently focused in the Console.
- J reports the currently loudest active source.
- K reports the selected source's typical active RMS level for this session.
- L reports the source with the loudest typical active RMS level for this session.

### Why does a pre-fader warning take over everything else?

It identifies possible damage before the OBS fader, so it has priority over output warnings. The decision dialog is silent. Choosing Yes starts guided monitoring; the low tone continues while the input is red. A sustained healthy active level resolves it. Silence is not treated as success: announcements occur after 2 and 12 seconds, followed by a question after 22 seconds. Choosing No or Escape stores that unchanged source on the pre-fader stop list. Changing its identity or settings causes it to be evaluated afresh.

### What can I do in Accessible Volume Console?

Ctrl+Grave opens it. Left and Right select sources; Up and Down adjust the focused fader by 1 dB; Home sets 0 dB; 1–9 select the first nine sources and 0 the tenth. Space toggles monitoring and program output together, Ctrl+Space toggles monitoring only, and Shift+Space toggles output only. The console normally lists active program sources; **Show All Sources** includes inactive sources. Changes take effect immediately, and the Console never raises a normally configured source above 0 dB by itself.

### What is the safest workflow?

Run Sound Doctor during representative speech and sound, review rather than blindly accept its suggestions, and leave music compression unchecked unless you want that effect. Keep Audible Meter active during normal work, use the Console's tones for immediate inspection, respond to pre-fader warnings at the original device or application, and use Shift+I only when you want a conservative output-fader correction.
