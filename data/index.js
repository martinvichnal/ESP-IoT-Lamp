var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

// var state = false;
// var mode = 0;
// var brightnessValue = 0;
// var speedValue = 0;
// var redValue = 0;
// var greenValue = 0;
// var blueValue = 0;


const obj = {
    state: 0,
    mode: 0,
    brightnessValue: 0,
    speedValue: 0,
    redValue: 0,
    greenValue: 0,
    blueValue: 0,
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
    document.getElementById("sendBtn").addEventListener("click", btnPressed);
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
    // document.getElementById('state').innerHTML = data.state;
    obj.state = data.state;
    obj.mode = data.mode;
    obj.brightnessValue = data.brightnessValue;
    obj.speedValue = data.speedValue;
    obj.redValue = data.redValue;
    obj.greenValue = data.greenValue;
    obj.blueValue = data.blueValue;

    document.getElementById('state').innerHTML = obj.state;
    document.getElementById('WSinfo1').innerHTML = obj.mode;
    document.getElementById('WSinfo2').innerHTML = obj.brightnessValue;
    document.getElementById('WSinfo3').innerHTML = obj.speedValue;
}

function sendWebSocket(sendObj) {
    // websocket.send('toggle');
    // websocket.send(JSON.stringify({'action':'toggle'}));
    console.log("SENDING THIS:");
    console.log(sendObj);
    websocket.send(JSON.stringify(sendObj));
}

function btnPressed()
{
    console.log("Send button was pressed. Sending websockets...");
    sendWebSocket(obj);
}


function setRGB() {
    obj.redValue = document.getElementById('red').value;
    obj.greenValue = document.getElementById('green').value;
    obj.blueValue = document.getElementById('blue').value;
}

function buttonClick(state) {
    obj.state = state;
}




// function setRGB() {
//     var redValue = document.getElementById('red').value;
//     var greenValue = document.getElementById('green').value;
//     var blueValue = document.getElementById('blue').value;
//     var xhttp = new XMLHttpRequest();
//     xhttp.open('GET', '/setRGB?red=' + redValue + '&green=' + greenValue + '&blue=' + blueValue, true);
//     xhttp.send();
// }

// function buttonClick(state) {
//     // var btn1 = document.getElementById("btnOn");
//     // var btn2 = document.getElementById("btnOff");
//     var xhttp = new XMLHttpRequest();
//     xhttp.open('GET', '/setBtn?state=' + state, true);
//     xhttp.send();
// }