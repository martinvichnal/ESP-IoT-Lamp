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

// Get current sensor readings when the page loads
window.addEventListener('load', getReadings);

// Function to get current readings on the webpage when it loads for the first time
function getReadings() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            var myObj = JSON.parse(this.responseText);
            // CONSOLING THE INFOS
            document.getElementById("info1").innerHTML = myObj.info1;
            document.getElementById("info2").innerHTML = myObj.info2;
            document.getElementById("info3").innerHTML = myObj.info3;
            // console.log(myObj.info1);
            // console.log(myObj.info2);
            // console.log(myObj.info3);
            console.log(document.getElementById("info2").text);
            console.log("getReadings()");
            console.log(myObj);
        }
    };
    xhr.open("GET", "/readings", true);
    xhr.send();
}

if (!!window.EventSource) {
    var source = new EventSource('/events');

    source.addEventListener('open', function (e) {
        console.log("Events Connected");
    }, false);

    source.addEventListener('error', function (e) {
        if (e.target.readyState != EventSource.OPEN) {
            console.log("Events Disconnected");
        }
    }, false);

    source.addEventListener('message', function (e) {
        console.log("message", e.data);
    }, false);

    source.addEventListener('new_readings', function (e) {
        console.log("new_readings", e.data);
        var myObj = JSON.parse(e.data);
        console.log("Event Listener");
        console.log(myObj);
        document.getElementById("info21").innerHTML = myObj.info1;
        document.getElementById("info22").innerHTML = myObj.info2;
        document.getElementById("info23").innerHTML = myObj.info3;
    }, false);
}