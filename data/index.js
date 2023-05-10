var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

const obj = {
    state: 0,
    mode: 0,
    brightnessValue: 0,
    speedValue: 0,
    redValue: 0,
    greenValue: 0,
    blueValue: 0,
}

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
    document.getElementById("sendBtn").addEventListener("click", btnPressed);
}


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

    setDebug();
}

function sendWebSocket(sendObj) {
    console.log("SENDING THIS:");
    console.log(sendObj);
    websocket.send(JSON.stringify(sendObj));
    setDebug();
}

function btnPressed() {
    console.log("Send button was pressed. Sending websockets...");
    sendWebSocket(obj);
}


function setRGB() {
    obj.redValue = document.getElementById('red').value;
    obj.greenValue = document.getElementById('green').value;
    obj.blueValue = document.getElementById('blue').value;
    sendWebSocket(obj);
}

function setBrightness() {
    obj.brightnessValue = document.getElementById('brightness').value;
    sendWebSocket(obj);
}

function setSpeed() {
    obj.speedValue = document.getElementById('speed').value;
    sendWebSocket(obj);
}

function setMode(_mode){
    obj.mode = _mode;
    sendWebSocket(obj);
}

function stateBtn() {
    const btn = document.getElementById('btnState');

    obj.state = !obj.state;

    if (obj.state == true) {
        btn.className = "button-off"
        btn.textContent = "OFF"
    }
    else {
        btn.className = "button-on"
        btn.textContent = "ON"
    }
    sendWebSocket(obj);
}

function setDebug()
{
    document.getElementById('debug1').textContent = obj.state;
    document.getElementById('debug2').textContent = obj.mode;
    document.getElementById('debug3').textContent = obj.brightnessValue;
    document.getElementById('debug4').textContent = obj.speedValue;
    document.getElementById('debug5').textContent = obj.redValue;
    document.getElementById('debug6').textContent = obj.greenValue;
    document.getElementById('debug7').textContent = obj.blueValue;
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