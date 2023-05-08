var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

// ----------------------------------------------------------------------------
// WebSocket handling
// ----------------------------------------------------------------------------

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    let data = JSON.parse(event.data);
    console.log(data);
    console.log(event);
    document.getElementById('state').innerHTML = data.state;
}

function toggle() {
    console.log("Sending 'toggle' with websocket");
    // websocket.send('toggle');
    websocket.send(JSON.stringify({'action':'toggle'}));
}



function setRGB() {
    var redValue = document.getElementById('red').value;
    var greenValue = document.getElementById('green').value;
    var blueValue = document.getElementById('blue').value;
    var xhttp = new XMLHttpRequest();
    xhttp.open('GET', '/setRGB?red=' + redValue + '&green=' + greenValue + '&blue=' + blueValue, true);
    xhttp.send();
}

function buttonClick(state) {
    // var btn1 = document.getElementById("btnOn");
    // var btn2 = document.getElementById("btnOff");
    var xhttp = new XMLHttpRequest();
    xhttp.open('GET', '/setBtn?state=' + state, true);
    xhttp.send();
}