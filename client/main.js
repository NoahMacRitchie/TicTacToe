const { app, BrowserWindow, ipcMain, Notification, ipcRenderer } = require('electron')
const net = require('net');

const client = new net.Socket();

let win = null;

const createWindow = () => {
  win = new BrowserWindow({
    width: 800,
    height: 600,
    webPreferences: {
      nodeIntegration: true
    }
  })

  win.loadFile('http://localhost:3000');

  win.once('ready-to-show', () => win.show());

  win.webContents.on('did-finish-load', () => {
    // connectToServer();
  });
}

const connectToServer = () => {
  client.connect(49157, '127.0.0.1', function() {
    console.log('Connected to server at 127.0.0.1:49157');
  });
}

const uint8ToString = (uint8Array) => {
  return String.fromCharCode.apply(null, uint8Array);
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow()
  }
});


ipcMain.on('testing-line', (event, data) => {
  console.log(`Sending '${data}' to server...`);
  client.write(data);
})

const notifyTeam = (data) => {
  console.log("notifying team...")
  win.webContents.send('notify-team', data);
  client.write("OK");
}

const handleMove = (data) => {
  console.log("handling move...")
}

client.on('data', function(data) {

  const dataStr = uint8ToString(data);

  console.log('Received: ' + data);

  switch(dataStr[0]){
    case 'N':
      notifyTeam(dataStr);
      break;
    case 'S':
      handleMove(dataStr);
      break;
  }
  
});

client.on('close', function() {
  console.log('Connection closed');
});