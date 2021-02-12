const { app, BrowserWindow, ipcMain } = require('electron');
const isDev = require('electron-is-dev');   
const path = require('path');


// C SERVER BACKEND IMPORTS
const net = require('net');
const client = new net.Socket();
 
let mainWindow;
 
function createWindow() {
    mainWindow = new BrowserWindow({
        width:650,
        height:650,
        show: false,
        // resizable: false
    });
    const startURL = isDev ? 'http://localhost:3000' : `file://${path.join(__dirname, '../build/index.html')}`;
 
    mainWindow.loadURL(startURL);
 
    mainWindow.once('ready-to-show', () => mainWindow.show());
    mainWindow.once('did-finish-load', () => connectToServer());
    mainWindow.on('closed', () => {
        mainWindow = null;
    });
}

const connectToServer = () => {
    client.connect(49157, '127.0.0.1', function() {
        console.log('Connected to server at 127.0.0.1:49157');
    });
}

app.on('ready', createWindow);


const uint8ToString = (uint8Array) => {
    return String.fromCharCode.apply(null, uint8Array);
}

ipcMain.on('testing-line', (event, data) => {
    console.log(`Sending '${data}' to server...`);
    client.write(data);
})

const notifyTeam = (data) => {
    console.log("notifying team...")
    win.webContents.send('notify-team', data);
}

const handleMove = (data) => {
    console.log("handling move...")
}

client.on('data', function (data) {

    const dataStr = uint8ToString(data);

    console.log('Received: ' + data);

    switch (dataStr[0]) {
        case 'N':
            notifyTeam(dataStr);
            break;
        case 'S':
            handleMove(dataStr);
            break;

    }

});

client.on('close', function () {
    console.log('Connection closed');
});