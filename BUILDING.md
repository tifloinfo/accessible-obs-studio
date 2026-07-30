# Building Accessible OBS Studio

The Windows x64 build requires Visual Studio 2022 Build Tools with the C++
desktop workload, Qt 6 development files compatible with OBS Studio, and the
Microsoft Edge WebView2 SDK.

Place Qt under `dependencies/qt6` and the unpacked WebView2 NuGet package under
`dependencies/webview2`, or supply their locations as MSBuild properties:

```powershell
msbuild AccessibleOBSStudio.sln /p:Configuration=Release /p:Platform=x64 `
  /p:QtRoot=C:\path\to\qt6 /p:WebView2Root=C:\path\to\webview2
```

The output is `build/Release/accessible-obs-studio.dll`. The installer source
is `installer/AccessibleOBSStudio.iss` and requires Inno Setup 6.7 or later.
When required on the destination computer, the installer downloads WebView2
and the Visual C++ Runtime directly from Microsoft's permanent download links,
then verifies a valid Microsoft Authenticode signature before executing either
download.

Run `tools/test-hardening.ps1` before packaging. It checks the shutdown lifetime,
Audible Meter, real-time volume, canvas-memory, prerequisite-verification, and
test-build identity invariants that are easy to regress during development.

After changing any localized Markdown ReadMe, install Node.js dependencies once
with `npm install`, then run `npm run docs:html`. This regenerates the six
accessible HTML ReadMe files that the installer can open in the user's selected
installation language.
