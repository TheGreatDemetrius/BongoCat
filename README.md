<img src="https://raw.githubusercontent.com/TheGreatDemetrius/BongoCat/refs/heads/main/img/icon.ico" width="64" alt="Logo">

# BongoCat

A tiny, always-on-top, click‑powered desktop buddy for Windows. Bongo Cat reacts to your keyboard and mouse input with left/right paw bops. Keep the vibes up while you work.

## Features
- **Input‑reactive animation**: Global low‑level keyboard and mouse hooks drive paws.
- **Always on top**: Stays visible over your workspace; can be hidden from the tray.
- **Drag anywhere**: Move the cat by dragging anywhere on the image.
- **System tray controls**: Show/Hide, Reset position, Skins, Startup app, Close.
- **Remembers position**: Window position is saved and restored across sessions.
- **Unlockable skins**: Progressively unlock more skins by accumulating input “clicks”.

## System Requirements
- **OS**: Windows 10 or 11
- **CPU/Memory**: Any modern x86 or x64 computer
- **Permissions**: No administrator rights required

## Build from Source
### Visual Studio (recommended)
1. Open `build/BongoCat.sln` in Visual Studio 2022.
2. Select configuration `Release` or `Debug`.
3. Choose platform `x86` or `x64`.
4. Build and run the `BongoCat` project.

### Command line (MSBuild)
From a Developer PowerShell for VS prompt:

```
msbuild .\build\BongoCat.sln -property:Configuration=Release -property:Platform=x64
```

The output executable will be placed in the corresponding build output directory for your platform.

## Usage
### Window controls
- **Show/Hide**: Left‑click the tray icon or use the tray menu item.
- **Move**: Drag anywhere on the cat image.

### Tray menu (right‑click the tray icon)
- Show/Hide (hidden mode still counts clicks)
- Reset position (places the cat on the taskbar)
- Skins (locked ones show required clicks)
- Startup app (runs with Windows)
- Close

## Privacy
**No telemetry and no network access.** The app only installs local low‑level input hooks to count clicks and drive animation.

## License
Licensed under the [MIT](./LICENSE) license.
