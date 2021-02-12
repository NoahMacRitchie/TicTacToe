const {ipcRenderer} = require('electron')

const myNotification = new Notification('Title', {
    body: 'Notification from the Renderer process'
})
let team = '';
  
myNotification.onclick = () => {
    console.log('Notification clicked')
}

ipcRenderer.on('ping', (event, data) => {
    console.log("got new-message");
    console.log(data);
});

ipcRenderer.on('notify-team', (event, data) => {
    console.log("got team!");
    document.getElementById("team-name").innerHTML = `Team: ${data[1]}`;
    team = data[1];
    console.log(data);
});

function submit(){ 
    ipcRenderer.send('testing-line', document.getElementById("test").value)
    return false;

}
