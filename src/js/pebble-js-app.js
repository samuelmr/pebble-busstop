var MAX_DEPS = 7;
var departureURI = "http://pubtrans.it/hsl/reittiopas/departure-api?max=25";
var initialized = false;
var messageQueue = [];
var options = [];

if (localStorage) {
 console.log("Stored configuration: " + localStorage.getItem('options'));
 options = JSON.parse(localStorage.getItem('options')) || options;
}


function refreshStops() {
  var stops = options['stops'] || [];
  var lines = options['lines'] || "";
  // pebble bug
  lines = lines.replace(/\+/g, ' ');
  if ((stops.length <= 0) || (lines.length <= 0)) {
    console.log("stops.length = " + stops.length + ", lines.length = " + lines.length);
    // alert - not cofigured
    return false;
  }
  var href = departureURI;
  for (var i=0; i<stops.length; i++) {
    href += "&stops%5B%5D=" + stops[i];
  }
  var req = new XMLHttpRequest();
  console.log("fetching " + href);
  req.open("GET", href, true);
  req.onload = function(e) {
    if (this.readyState == 4) {
      if(this.status == 200) {
        var deps = JSON.parse(req.responseText);
        console.log("OK, got " + deps.length + " departures");
        if (deps.length) {
          for (i=0; i<deps.length; i++) {
            var dep = deps[i];
            if (lines.indexOf("|" + dep["route"] + "|") < 0) {
              console.log("route '" + dep["route"] + "' not in " + lines);
              continue;
            }
            console.log("route '" + dep["route"] + "' found!");
            var msg = {};
            msg["0"] = dep["stopname"];
            msg["1"] = dep["line"];
            msg["2"] = descandify(dep["dest"])
            var d = new Date(dep["time"]*1000);
            var m = d.getMinutes();
            m = (m < 10) ? "" + "0" + m : m;
            msg["3"] = d.getHours() + ":" + m;
            messageQueue.push(msg);
            console.log("Queued departure for " + msg["0"] + ": " + msg["1"] + ":" + msg["2"] + ":" + msg["3"]);
            if (messageQueue.length >= MAX_DEPS) {
              break;
            }
          }
          sendNextMessage();
        }
      }
      else {
        console.log("Error: " + this.status);
      }
    }
  }
  req.send(null);
}

function sendNextMessage() {
  if (messageQueue.length > 0) {
    Pebble.sendAppMessage(messageQueue[0], appMessageAck, appMessageNack);
    console.log("Sent message to Pebble! " + JSON.stringify(messageQueue[0]));
  }
}

function appMessageAck(e) {
  console.log("Message accepted by Pebble!");
  messageQueue.shift();
  sendNextMessage();
}

function appMessageNack(e) {
  console.log("Message rejected by Pebble! " + e.error);
}

Pebble.addEventListener("ready",
  function(e) {
    console.log("JavaScript app ready and running!");
    messageQueue = [];
    refreshStops();
    initialized = true;
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    options = JSON.parse(decodeURIComponent(e.response));
    console.log("Webview window returned: " + JSON.stringify(options));
    localStorage['options'] = JSON.stringify(options);
    refreshStops();
  }
);

Pebble.addEventListener("showConfiguration",
  function() {
    var uri = "https://rawgithub.com/samuelmr/pebble-busstop/master/configure.html";
    console.log("Configuration url: " + uri);
    Pebble.openURL(uri);
  }
);

function descandify(str) {
  str = escape(str);
  str = str.replace(/%E8|%E9/, 'e').replace(/%C8|%C9/, 'E');
  str = str.replace(/%E5|%E4/, 'a').replace(/%C5|%C4/, 'A');
  str = str.replace(/%F6/, 'o').replace(/%D6/, 'O');
  str = str.replace(/%20/, ' ');
  return str;
}
