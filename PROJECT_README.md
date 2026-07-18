# Accessible OBS Studio 1.0

[![Tiflo.Info logo: blue waves above the words Tiflo.info and the Russian phrase “Close your eyes and see”](assets/tiflo-info-logo.jpg)](https://tiflo.info)

Accessible OBS Studio is a Windows accessibility plugin for OBS Studio. It is designed for blind keyboard and screen-reader users and has been developed with both JAWS and NVDA in mind.

## Requirements

- 64-bit Windows 10 or Windows 11.
- A supported 64-bit OBS Studio 32 release.
- An OpenAI API key only for canvas analysis and compatibility analysis.
- Internet access during installation only if WebView2 or the Visual C++
  Runtime is missing, and later when an OpenAI feature is used.

The installer checks for Microsoft WebView2 and the Visual C++ Runtime. It downloads and installs only missing components from Microsoft. OBS and Qt files are never replaced.

## Installation

1. Install the 64-bit edition of OBS Studio 32.0 or later, then run
   `AccessibleOBSStudio-1.0-Setup.exe`.
2. If OBS Studio is absent, damaged, or older than 32.0, Setup offers to open
   the [official OBS Studio download page](https://obsproject.com/download) and
   exits without installing files or prerequisites. You can also update an
   older OBS release from Help > Check for Updates in OBS Studio.
3. OBS Studio 32.x is supported. With OBS Studio 33 or later, Setup warns that
   this plugin release may be incompatible and offers the
   [latest-plugin page](https://tiflo.info/aobs) before allowing an explicit
   install-anyway choice.
4. Close OBS Studio before continuing. If it is running, Setup asks you to
   close it and choose Retry; it never terminates OBS automatically.
5. Complete the installation and start OBS Studio.
6. On the final page, the **Open the ReadMe in your web browser** checkbox opens
   the HTML documentation matching the language selected for Setup.

The plugin is installed under `C:\ProgramData\obs-studio\plugins\accessible-obs-studio`. No desktop or Start-menu shortcut is created.

To uninstall, open Windows Installed Apps and remove Accessible OBS Studio. OBS must be closed. Settings are preserved unless you explicitly select the unchecked option to remove them.

## Default keyboard shortcuts

- F3: Basic Description of the current canvas, limited to 80 characters.
- Shift+F3: Detailed Description of the current canvas.
- Alt+F3: read visible canvas text without translation or commentary.
- Ctrl+F3: describe visible people and their backgrounds.
- F4: open Visual Checker to check the stream or recording for visual issues involving layout, camera, lighting, framing, focus, grain, appearance, clothing, background, and unwanted objects.
- Ctrl+M: focus the visible native Media Controls.
- F5: start or stop streaming.
- F6: move to the next main OBS interface area.
- Shift+F6: move to the previous interface area.
- F7: start or stop recording.
- F8: start or stop the Virtual Camera.
- Ctrl+0 through Ctrl+5: focus Video Preview, Scenes, Sources, Audio Mixer, Scene Transitions, or Controls.
- Ctrl+Grave: open the Accessible Volume Console. Grave is the physical key immediately below Escape; its printed character depends on the keyboard layout.

The command **Accessible OBS Studio: Open Keyboard Shortcut Editor** opens the same editor directly. It is available for assignment but has no default keyboard shortcut. Its internal identifier is unchanged, so an existing assignment is preserved.

By default, Accessible OBS Studio forces all OBS keyboard shortcuts to work only while OBS is the active application. It keeps **Settings > Advanced > Hotkey Focus Behavior** set to **Disable hotkeys when main window is not in focus** and restores that value if it changes. To return control to OBS, select **Allow OBS Studio to manage whether keyboard shortcuts work outside OBS** in the Keyboard Shortcut Editor and save. When selected, the plugin stops changing the setting and stops overriding OBS hotkey state.

On first run and after a profile change, the plugin checks planned defaults against existing assignments. A modal, screen-reader-accessible dialog appears only when real conflicts exist. You can remove only the conflicting combinations and use the Accessibility defaults, or keep the existing assignments and leave the conflicting Accessibility defaults unassigned. “Do not ask again” remembers the chosen policy for this plugin build across profiles; a later build asks again when conflicts exist.

## Keyboard Shortcut Editor

Choose **Tools > Accessible OBS Studio** to open the Keyboard Shortcut Editor directly. Search the Commands list and select a command. Press Enter or activate **Add or Edit** to open the Keyboard Shortcut dialog; press Delete to remove the selected command's assignments. The list uses arrow-key navigation, while Tab moves between the search field, the currently selected command, the OBS hotkey-control checkbox, and the editor buttons. The checkbox is cleared by default. Activate **OpenAI API Settings** in this editor to configure OpenAI.

The Keyboard Shortcut dialog supports every keyboard shortcut assigned to the command. Type one combination into the Keyboard Shortcut field, use **Add Another Keyboard Shortcut** for an additional assignment, or remove a selected assignment. Enter or OK checks immediately for duplicates. If another command uses the combination, the dialog identifies it and asks whether to reassign it. No returns to the assignment dialog; Yes removes that conflicting assignment. Escape cancels the dialog. Tab, Shift+Tab, Enter, Escape, Alt+F4, Windows-key combinations, and other reserved system commands are not captured.

Changes are staged until you activate OK in the main editor. Delete or Remove clears every assignment for the selected command. Closing with unsaved changes offers Save, Discard, and Cancel.

## Audio Mixer and Media Controls

Ctrl+3 focuses the native Audio Mixer. The plugin no longer renames, numbers, or monitors its controls; use Tab and Shift+Tab with native OBS keyboard behavior. Number-based source selection remains available in the Accessible Volume Console.

Ctrl+Grave opens the modal **Accessible Volume Console** without changing the control remembered in the main OBS window. It contains every audio source currently active in the OBS Mixer, including applicable sources in groups or nested scenes and global devices such as Desktop Audio and Mic/Aux. The first vertical slider receives focus. Left and Right move between sources; Up and Down change the selected source by 1 dB; Home restores 0 dB; and Space toggles mute. Number keys 1 through 9 select the first nine sources and 0 selects the tenth. Changes take effect immediately. Escape closes the console and restores the previous OBS control. JAWS and NVDA receive the source name, volume in dB, and mute state as values change.

The console takes an exclusive snapshot of the available audio sources, volumes, and mute states when it opens. Its changes take effect in OBS immediately, but it does not monitor changes made through the native mixer, an external controller, scene switching, or other control methods while it remains open. Do not manipulate the mixer elsewhere during an Accessible Volume Console session; close and reopen the console to load external changes. All initially captured sources remain reachable with Left and Right even when there are more than ten; only direct number-key selection is limited to ten. The console command and its default keyboard shortcut can be changed in the Keyboard Shortcut Editor.

Ctrl+M focuses the Media Controls only when they are visible. Accessible OBS Studio does not replace or intercept OBS media commands; use the keys supplied by OBS.

## Canvas Describer and approved fixes

An API key is optional and is never requested for ordinary plugin functions. Open **Tools > Accessible OBS Studio**, then activate **OpenAI API Settings** in the Keyboard Shortcut Editor when you want to save, replace, or remove a key. The Qt dialog supports standard keyboard navigation. A stored key is never displayed. The key format and OpenAI authentication are checked before Windows Credential Manager encrypts it for the current Windows account; an existing key is retained if its replacement cannot be validated or saved. **I Have No Key Yet** appears only when no key is stored, while **Remove key** appears only when a key is stored. Removal requires confirmation and reports whether it succeeded. OpenAI features remain blocked without a key, while all other plugin functions continue to work.

All five Canvas Describer keyboard shortcuts capture the rendered OBS canvas without requiring preview focus. A brief click confirms that the request has started. The current control remains focused while capture and upload begin. The WebView2 result window then receives focus; closing it restores the previous OBS control when appropriate and does not steal focus from another application. Only a newly received answer uses the aggressive ARIA alert role. Questions are never echoed into the conversation.

All five modes permit image-related follow-up questions. Image text and follow-up messages are treated as untrusted content. Unrelated questions are refused, and the image conversation ends when the window closes or another direct capture begins.

Basic Description is also a starting point. Detailed Description is always offered without resubmitting the image. Read Text appears only when text is detected; People and Backgrounds appears only when people are detected. Suggested Fixes appears only when the plugin has a specific approved OBS transform that can address a detected issue. A successful preset disappears while other relevant presets remain.

People and Backgrounds makes visible people the primary subject, then describes their immediate backgrounds. Unrelated interface, text, and scene details are omitted unless they directly affect a person's presentation.

**Visual Checker** reports high-severity automatically correctable OBS layout problems, blank black or white captures, unusable camera or lighting conditions, and visibly non-full-screen Zoom meetings; medium-severity framing concerns; and low-severity lighting, blur, possible lens contamination, grain, appearance, clothing, background, unwanted objects, and other visual concerns. It checks only how the stream or recording looks. It ignores verbal content: the language, spelling, grammar, translation, wording, facts, calculations, subject matter, names, numbers, opinions, tone, appropriateness, captions, and subtitles are never treated as issues. Text can be reported only as a visual object when it is too small, clipped, blurred, low contrast, obstructed, or obstructs important visuals. A dialog or error window can be reported only when it visibly covers content or reveals a capture or layout problem, not because of its message. A blank capture is reported without guessing its cause. For a visibly non-full-screen Zoom meeting, the guidance is to switch to Zoom, press Alt+F, return to OBS, and avoid minimizing Zoom or all windows during the stream. Browser, presentation, and other video-calling interfaces are treated as possible captured content. Automatic-fix controls appear only for approved source transforms. When visual issues remain, **Check Again** captures and submits a fresh frame, compares it with the earlier frame, and reports improvements, regressions, changes, and unresolved visual issues.

The **Suggested Fixes** and **Fix Automatically** buttons expose only applicable items from a bounded list of reversible OBS source transforms. A source must be selected; if none is selected, an accessible source list is shown. Each fix displays its risk and requires an individual checkbox. Nothing runs until **Apply Selected** is activated. OBS’s own actions and Undo history are used. Streaming, recording, audio, credentials, arbitrary commands, scene deletion, and output settings cannot be controlled through this feature.

## Compatibility analysis

When OBS is newer than the tested major release, the warning offers **Cancel**, **Run Anyway**, and **Analyze Compatibility**. Analysis combines local read-only probes with official OBS sources searched through OpenAI. It reports Low, Medium, High, or Unknown risk; it never guarantees compatibility.

A successful report is saved for the exact OBS version, Accessible OBS Studio version, and architecture. Reopening it does not contact OpenAI again. The report is displayed in an accessible WebView2 window and copied to the Clipboard.

## Privacy and cost

Canvas analysis sends the captured canvas, OBS locale, the fixed safety prompt, and valid follow-up questions to OpenAI. Preset descriptions reuse the existing response chain without resubmitting the frame. Check Again intentionally sends a fresh frame for comparison. Compatibility analysis sends version and dependency information plus a fixed plugin capability manifest; it does not send scenes, media, credentials, or the API key itself as content. OpenAI API charges belong to the key owner. No telemetry, advertising, or unrelated network request is included.

## Troubleshooting

- If the plugin does not appear, confirm that OBS is 64-bit and was closed during installation.
- If F3 reports that no key is stored, save the key through OpenAI Settings.
- If a keyboard shortcut does not fire, inspect the Keyboard Shortcut Editor, OBS Settings > Hotkeys, and Advanced > Hotkey Focus Behavior.
- If Media Controls are unavailable, select a currently playable media source so OBS displays them.
- If WebView2 cannot initialize, repair its Microsoft runtime or run the Accessible OBS Studio installer again while connected to the Internet.

## License and contact

Copyright (C) 2026 [Tiflo.Info](https://tiflo.info). Licensed under GNU GPL version 2 or later. See [LICENSE.txt](LICENSE.txt) and [LICENSE-GPL-2.0.txt](LICENSE-GPL-2.0.txt). Project names and logos are separately protected branding and are not an additional restriction on GPL-covered code.

Localized documentation: [Deutsch](docs/README.de-DE.md), [Español](docs/README.es-ES.md), [Français](docs/README.fr-FR.md), [Русский](docs/README.ru-RU.md), [Українська](docs/README.uk-UA.md).
