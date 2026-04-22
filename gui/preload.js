const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('viking', {
  pickEngine: () => ipcRenderer.invoke('engine:pick'),
  loadEngine: (p) => ipcRenderer.invoke('engine:load', p),
  send: (cmd) => ipcRenderer.invoke('engine:send', cmd),
  stopEngine: () => ipcRenderer.invoke('engine:stop'),
  onLine: (cb) => ipcRenderer.on('engine:line', (_e, line) => cb(line)),
  onExit: (cb) => ipcRenderer.on('engine:exit', (_e, code) => cb(code))
});
