function sendCredentials() {
    var SSID = document.getElementById('ssid').value;
    var PASS = document.getElementById('pass').value;

    var xhttp = new XMLHttpRequest();
    xhttp.open('GET', '/get?ssid=' + SSID + '&pass=' + PASS, true);
    xhttp.send();
}