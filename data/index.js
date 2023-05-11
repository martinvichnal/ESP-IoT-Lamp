const btn = document.getElementById('btnState');
const brightnessSlider = document.getElementById('brightness');
const speedSlider = document.getElementById('speed');
const rgbSlider = document.getElementById('rgb');
// const redSlider = document.getElementById('red');
// const greenSlider = document.getElementById('green');
// const blueSlider = document.getElementById('blue');

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

const obj = {
    state: 0,
    mode: 0,
    brightnessValue: 0,
    speedValue: 0,
    rgbValue: 0
    // redValue: 0,
    // greenValue: 0,
    // blueValue: 0,
}

window.addEventListener('load', onLoad);

function onLoad(event) {
    document.getElementById('connectedOrNot').textContent = "NOT CONNETED";
    document.getElementById('connectedOrNot').style.color = "red";
    initWebSocket();
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
    document.getElementById('connectedOrNot').textContent = "CONNETED";
    document.getElementById('connectedOrNot').style.color = "green";
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    let data = JSON.parse(event.data);
    // console.log(data);
    obj.state = data.state;
    obj.mode = data.mode;
    obj.brightnessValue = data.brightnessValue;
    obj.speedValue = data.speedValue;
    obj.rgbValue = data.rgbValue;
    // obj.redValue = data.redValue;
    // obj.greenValue = data.greenValue;
    // obj.blueValue = data.blueValue;

    document.getElementById('sentOrReceived').textContent = "RECEIVED";
    document.getElementById('sentOrReceived').style.color = "green";

    setEverything();
    setDebug();
}

function sendWebSocket(sendObj) {
    // console.log("SENDING THIS:");
    // console.log(sendObj);
    websocket.send(JSON.stringify(sendObj));
    setDebug();
    document.getElementById('sentOrReceived').textContent = "SENT";
    document.getElementById('sentOrReceived').style.color = "red";
}

function btnPressed() {
    console.log("Send button was pressed. Sending websockets...");
    sendWebSocket(obj);
}


function setRGB() {
    // obj.redValue = redSlider.value;
    // obj.greenValue = greenSlider.value;
    // obj.blueValue = blueSlider.value;
    obj.rgbValue = rgbSlider.value;
    sendWebSocket(obj);
}

function setBrightness() {
    obj.brightnessValue = brightnessSlider.value;
    sendWebSocket(obj);
}

function setSpeed() {
    obj.speedValue = speedSlider.value;
    sendWebSocket(obj);
}

function setMode(_mode){
    obj.mode = _mode;
    switch (_mode) {
        case 0:
            document.getElementById('btnMode0').className = "button-off"
            document.getElementById('btnMode1').className = "button-on"
            document.getElementById('btnMode2').className = "button-on"
            break;
        case 1:
            document.getElementById('btnMode0').className = "button-on"
            document.getElementById('btnMode1').className = "button-off"
            document.getElementById('btnMode2').className = "button-on"
            break;
        case 2:
            document.getElementById('btnMode0').className = "button-on"
            document.getElementById('btnMode1').className = "button-on"
            document.getElementById('btnMode2').className = "button-off"
            break;
    }
    sendWebSocket(obj);
}

function stateBtn() {
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
    document.getElementById('debug5').textContent = obj.rgbValue;
    // document.getElementById('debug5').textContent = obj.redValue;
    // document.getElementById('debug6').textContent = obj.greenValue;
    // document.getElementById('debug7').textContent = obj.blueValue;
}

function setEverything()
{
    // Setting the on off button
    if (obj.state == true) {
        btn.className = "button-off"
        btn.textContent = "OFF"
    }
    else {
        btn.className = "button-on"
        btn.textContent = "ON"
    }

    // Setting the mode buttons
    switch (obj.mode) {
        case 0:
            document.getElementById('btnMode0').className = "button-off"
            document.getElementById('btnMode1').className = "button-on"
            document.getElementById('btnMode2').className = "button-on"
            break;
        case 1:
            document.getElementById('btnMode0').className = "button-on"
            document.getElementById('btnMode1').className = "button-off"
            document.getElementById('btnMode2').className = "button-on"
            break;
        case 2:
            document.getElementById('btnMode0').className = "button-on"
            document.getElementById('btnMode1').className = "button-on"
            document.getElementById('btnMode2').className = "button-off"
            break;
    }

    // Setting the brightness slider
    brightnessSlider.value = obj.brightnessValue;

    // Setting the speed slider
    speedSlider.value = obj.speedValue;

    // Setting the RGB sliders
    rgbSlider.value = obj.rgbValue;
    // redSlider.value = obj.redValue;
    // greenSlider.value = obj.greenValue;
    // blueSlider.value = obj.blueValue;
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