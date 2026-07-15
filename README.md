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

1. Close OBS Studio. The installer never terminates OBS automatically.
2. Run `AccessibleOBSStudio-1.0-Setup.exe`.
3. Confirm the detected OBS installation or choose its folder.
4. Complete the installation and start OBS.

The plugin is installed under `C:\ProgramData\obs-studio\plugins\accessible-obs-studio`. No desktop or Start-menu shortcut is created.

To uninstall, open Windows Installed Apps and remove Accessible OBS Studio. OBS must be closed. Settings are preserved unless you explicitly select the unchecked option to remove them.

## Default keyboard commands

- F3: capture the current OBS canvas and ask OpenAI to analyze it.
- F4: focus the visible native Media Controls.
- F5: start or stop streaming.
- F6: move to the next main OBS interface area.
- Shift+F6: move to the previous interface area.
- F7: start or stop recording.
- F8: start or stop the Virtual Camera.
- Ctrl+0 through Ctrl+5: focus Video Preview, Scenes, Sources, Audio Mixer, Scene Transitions, or Controls.

The command **Accessible OBS Studio: Open Accessible OBS Studio** opens the Shortcut Editor. It is available for assignment but has no default shortcut.

The first time the plugin runs, it changes OBS’s Hotkey Focus Behavior to disable hotkeys while the main OBS window is not focused—but only if you have never explicitly chosen that OBS setting. Your later choice is always respected.

## Shortcut Editor

Choose **Tools > Accessible OBS Studio**. Search the Commands list, select a command, and activate **Add or Edit** or press Enter. OpenAI configuration is available through the **API Settings** button in this window.

The Hot Key dialog supports every shortcut assigned to the command. Type one shortcut into the Shortcut field, use **Add Another Shortcut** for an additional assignment, or remove a selected assignment. Enter accepts the dialog. Escape cancels it. Tab, Shift+Tab, Enter, Escape, Alt+F4, Windows-key combinations, and other reserved system commands are not captured.

Changes are staged until you activate OK in the main editor. Delete or Remove clears every assignment for the selected command. Closing with unsaved changes offers Save, Discard, and Cancel.

## Audio Mixer and Media Controls

Ctrl+3 focuses the Audio Mixer. Its visible volume controls are numbered in display order. Press 1 through 9 for sources one through nine, or 0 for source ten. Native OBS arrow keys adjust the focused slider. Muting and other controls remain native OBS operations available through Tab navigation.

F4 focuses the Media Controls only when they are visible. Accessible OBS Studio does not replace or intercept OBS media commands; use the keys supplied by OBS.

## Canvas analysis and approved actions

An API key is optional and is never requested for ordinary plugin functions. Open **Tools > Accessible OBS Studio**, then activate **API Settings** when you want to save a key. The dialog supports Tab and Shift+Tab; Enter in the key field activates **Save Key**. The key format and OpenAI authentication are checked before Windows Credential Manager encrypts it for the current Windows account. If an OpenAI feature requests a key and you do not have one, activate **I Have No Key Yet**; OpenAI features remain blocked, while all other plugin functions continue to work.

F3 captures the rendered canvas without requiring preview focus. A brief click confirms that the request has started. The current control remains focused while capture and upload begin. The analysis window then receives focus; closing it restores the previous OBS control when it still exists and does not steal focus from another application.

Follow-up questions must concern the captured image. Image text and follow-up messages are treated as untrusted content. Unrelated questions are refused, and the image conversation ends when the window closes or another capture begins.

The **Suggested Actions** button exposes a bounded list of reversible OBS transform actions. A source must be selected; if none is selected, an accessible source list is shown. Each action displays its risk and requires an individual checkbox. Nothing runs until **Apply Selected** is activated. OBS’s own actions and Undo history are used. Streaming, recording, audio, credentials, arbitrary commands, scene deletion, and output settings cannot be controlled through this feature.

## Compatibility analysis

When OBS is newer than the tested major release, the warning offers **Cancel**, **Run Anyway**, and **Analyze Compatibility**. Analysis combines local read-only probes with official OBS sources searched through OpenAI. It reports Low, Medium, High, or Unknown risk; it never guarantees compatibility.

A successful report is saved for the exact OBS version, Accessible OBS Studio version, and architecture. Reopening it does not contact OpenAI again. The report is displayed in an accessible WebView2 window and copied to the Clipboard.

## Privacy and cost

Canvas analysis sends the captured canvas, OBS locale, the fixed safety prompt, and valid follow-up questions to OpenAI. Compatibility analysis sends version and dependency information plus a fixed plugin capability manifest; it does not send scenes, media, credentials, or the API key itself as content. OpenAI API charges belong to the key owner. No telemetry, advertising, or unrelated network request is included.

## Troubleshooting

- If the plugin does not appear, confirm that OBS is 64-bit and was closed during installation.
- If F3 reports that no key is stored, save the key through OpenAI Settings.
- If a shortcut does not fire, inspect OBS Settings > Hotkeys and Advanced > Hotkey Focus Behavior.
- If Media Controls are unavailable, select a currently playable media source so OBS displays them.
- If WebView2 cannot initialize, repair its Microsoft runtime or run the Accessible OBS Studio installer again while connected to the Internet.

## License and contact

Copyright (C) 2026 [Tiflo.Info](https://tiflo.info). Licensed under GNU GPL version 2 or later. See [LICENSE.txt](LICENSE.txt) and [LICENSE-GPL-2.0.txt](LICENSE-GPL-2.0.txt). Project names and logos are separately protected branding and are not an additional restriction on GPL-covered code.

Localized documentation: [Deutsch](docs/README.de-DE.md), [Español](docs/README.es-ES.md), [Français](docs/README.fr-FR.md), [Русский](docs/README.ru-RU.md), [Українська](docs/README.uk-UA.md).
