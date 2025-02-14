# ![mute](https://github.com/user-attachments/assets/197bb48a-923e-44ac-bd45-b807e3ef28a9) mute by Stian

mute is a super lightweight and robust utility that gives you a system-wide hotkey to mute and unmute your microphone(s).

## Why?

Juggling different apps' hotkeys to mute and unmute yourself and making sure that the various apps are in the same mute state is tedious and error-prone. Think video or voice calls, streaming, and gaming. mute gives you a single global hotkey that mutes your microphone(s) at the system level (the Windows Core Audio API) and applies to any and all applications capturing audio from your audio recording devices.

With an on-screen display and strict error policy, mute gives you definitive knowledge of whether anything on your computer can hear you through any of your audio capture devices. If mute encounters an error, it will not tell you that you are muted. Only upon Windows reporting that all connected audio capture devices were successfully muted does mute show you that you are muted.

Certain nice-to-have features have been intentionally left out to make mute safe to use with anti-cheat software. mute does not use any hooks.

## How to install

Download [the latest mute.exe](https://github.com/stianhoiland/mute/releases/latest) in the Releases section. Double click to run. Delete to uninstall. The executable leaves no trace behind.

## How to use

After launching mute.exe, press *Alt+M* to mute and unmute your microphone(s). An on-screen display briefly shows whether your microphone(s) are muted or unmuted. Press Alt+Shift+M or double click mute.exe again to quit.

## How does it work?

mute is extremely fast and small. It is written in plain C and directly interfaces with Windows. It uses the Core Audio API to enumerate through all connected audio recording devices at system level and mutes or unmutes them all at the same time.

This is equivalent to going to the Recording tab in the `mmsys.cpl` control panel, right clicking each microphone, Properties, Levels, and clicking the speaker icon to mute the recording device. In fact, you can go to that control panel and check that each audio recording device is muted and unmuted when you use mute.

__NOTE:__ If you have a setup with multiple audio capture devices and need them to be in a mixed state of muted and unmuted, then this utility is not for you. mute values simplicity and the 80% use case of just wanting to be muted or unmuted and achieves this by muting and unmuting all connected audio capture devices all at once.

## Limitations / Known issues

- Features like hold-to-unmute are not implemented as they require a keyboard hook which may trip anti-cheat software
- mute's OSD can not show over fullscreen applications (windowed fullscreen is fine!), as doing so would require a hook which may trip anti-cheat software
- At the moment mute sticks to non-AA text drawing, which can sometimes look a little uneven, but is extremely fast and has a clean implementation
- There is a bug related to special caching of layered windows where the OSD may briefly flash the prior muted/unmuted state. It is inconsequential, but will be fixed in time.

## Donations

For some reason this convenient functionality is neither a native part of Windows itself nor a use case Microsoft are willing to support (see PowerToys Video Conference Mute). In addition, it seems that there is almost as-if a sport in offering this functionality only with paid software and not for free. I'm unsure why.

If you really appreciate me spending my time creating this and making it available free of charge, then consider donating to me on [Patreon](https://www.patreon.com/stianhoiland).

## Buildling from source

These instructions require `git`, `cmake`, and *Visual Studio* or *MSBuild*.
1. `git clone https://github.com/stianhoiland/mute.git`
2. `cd mute`
3. `mkdir build`
4. `cmake -G "Visual Studio 17 2022" -B build`
5. *(in developer console)* `devenv build\mute.sln /build "MinSizeRel|x64"`
6. or: *(in developer console)* `msbuild build\mute.sln /property:Configuration=MinSizeRel`
7. *(run mute)* `build\MinSizeRel\mute.exe`


## Credits

My girlfriend who made the beautiful icon.

Dennis Ritchie who co-created the C programming language. Rest in peace.

## Endorsements

> Another utility to rule them all,
    another utility to find them,
Another utility to bring them all
    and in streaming bind them.
~ Sauron feat. Stian
