const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const { spawn } = require('child_process');
const path = require('path');

let win = null;
let engine = null;
let engineBuf = '';

function createWindow() {
  win = new BrowserWindow({
    width: 1200,
    height: 820,
    title: 'Viking GUI',
    icon: path.join(__dirname, 'renderer', 'logo.png'),
    backgroundColor: '#1e1e1e',
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false
    }
  });
  win.setMenuBarVisibility(false);
  win.loadFile(path.join(__dirname, 'renderer', 'index.html'));
}

function killEngine() {
  if (!engine) return;
  try { engine.stdin.write('quit\n'); } catch (_) {}
  try { engine.kill(); } catch (_) {}
  engine = null;
  engineBuf = '';
}

ipcMain.handle('engine:pick', async () => {
  const r = await dialog.showOpenDialog(win, {
    title: 'Select UCI engine executable',
    properties: ['openFile'],
    filters: [
      { name: 'Executable', extensions: ['exe'] },
      { name: 'All files', extensions: ['*'] }
    ]
  });
  if (r.canceled || !r.filePaths.length) return null;
  return r.filePaths[0];
});

ipcMain.handle('engine:load', (_evt, enginePath) => {
  killEngine();
  try {
    engine = spawn(enginePath, [], {
      cwd: path.dirname(enginePath),
      stdio: ['pipe', 'pipe', 'pipe'],
      windowsHide: true
    });
  } catch (e) {
    return { ok: false, error: e.message };
  }

  engine.stdout.on('data', (chunk) => {
    engineBuf += chunk.toString();
    let idx;
    while ((idx = engineBuf.indexOf('\n')) !== -1) {
      const line = engineBuf.slice(0, idx).replace(/\r$/, '');
      engineBuf = engineBuf.slice(idx + 1);
      if (win && !win.isDestroyed()) win.webContents.send('engine:line', line);
    }
  });
  engine.stderr.on('data', (chunk) => {
    if (win && !win.isDestroyed()) {
      win.webContents.send('engine:line', '[stderr] ' + chunk.toString().trimEnd());
    }
  });
  engine.on('exit', (code) => {
    if (win && !win.isDestroyed()) {
      win.webContents.send('engine:exit', code);
    }
    engine = null;
  });
  engine.on('error', (err) => {
    if (win && !win.isDestroyed()) {
      win.webContents.send('engine:line', '[error] ' + err.message);
    }
  });

  return { ok: true, path: enginePath };
});

ipcMain.handle('engine:send', (_evt, cmd) => {
  if (!engine) return false;
  try {
    engine.stdin.write(cmd + '\n');
    return true;
  } catch (_) {
    return false;
  }
});

ipcMain.handle('engine:stop', () => { killEngine(); return true; });

app.whenReady().then(createWindow);
app.on('window-all-closed', () => { killEngine(); app.quit(); });
