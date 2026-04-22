# Viking GUI

Minimal Electron desktop GUI for the **Viking** UCI chess engine (or any UCI engine).

See the top-level [README](../README.md#viking-gui) for the full feature list and setup instructions. Quick start:

```
cd gui
npm install
npm start
```

Then click **Load engine…** and pick your `engine.exe`. Produce a Windows installer with `npm run dist` (outputs into `dist/`).

## Project layout

```
gui/
├── main.js              Electron main process — spawns the engine, UCI plumbing
├── preload.js           contextBridge API exposed to the renderer
├── package.json         Electron + electron-builder config
└── renderer/
    ├── index.html       UI layout
    ├── style.css        Dark theme styling
    ├── renderer.js      Board, chess.js bookkeeping, UCI parsing, analysis
    └── logo.png         Viking logo (window icon + in-app branding)
```

## Notes

- Chessboard, chess.js, and jQuery are loaded from CDN. For an offline build, install them via npm and switch the tags in `renderer/index.html` to local paths.
- Legality is enforced client-side by chess.js; illegal drops snap back.
- Evaluations are normalized to White's point of view regardless of whose turn it is.
- The `nps` and `time` fields are computed in the GUI from the engine's `nodes` output and wall-clock, since Viking's `info` line currently emits neither.
