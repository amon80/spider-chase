var haveEvents = 'ongamepadconnected' in window;
var controllers = {};

var num = 0;

function connecthandler(e) {
  console.log("connected")
  addgamepad(e.gamepad);
}

function addgamepad(gamepad) {
  controllers[gamepad.index] = gamepad;

  var d = document.createElement("div");
  d.setAttribute("id", "controller" + gamepad.index);
  d.setAttribute("style","display:none");

  var t = document.createElement("h1");
  t.appendChild(document.createTextNode("gamepad: " + gamepad.id));
  d.appendChild(t);

  var b = document.createElement("div");
  b.className = "buttons";
  for (var i = 0; i < gamepad.buttons.length; i++) {
    var e = document.createElement("span");
    e.className = "button";
    //e.id = "b" + i;
    e.innerHTML = i;
    b.appendChild(e);
  }

  d.appendChild(b);

  var a = document.createElement("div");
  a.className = "axes";

  for (var i = 0; i < gamepad.axes.length; i++) {
    var p = document.createElement("progress");
    p.className = "axis";
    //p.id = "a" + i;
    p.setAttribute("max", "2");
    p.setAttribute("value", "1");
    p.innerHTML = i;
    a.appendChild(p);
  }

  d.appendChild(a);

  //console.dir(d)
  //console.dir(a);
  //console.dir(haveEvents);
  //haveEvents = true;

  // See https://github.com/luser/gamepadtest/blob/master/index.html
  var start = document.getElementById("start");
  if (start) {
    start.style.display = "none";
  }

  document.body.appendChild(d);
  requestAnimationFrame(updateStatus);
}

function disconnecthandler(e) {
  removegamepad(e.gamepad);
}

function removegamepad(gamepad) {
  var d = document.getElementById("controller" + gamepad.index);
  document.body.removeChild(d);
  delete controllers[gamepad.index];
}

function updateStatus() {
  //console.log("UpdateStatus")
  if (!haveEvents) {
    scangamepads();
  }
  //console.log("UpdateStatusAfterScan")

  var i = 0;
  var j;
  
  require('electron').remote.getGlobal('sharedObj').button = null
  require('electron').remote.getGlobal('sharedObj').axis = "0: 0.0000,1: 0.0000,2: 0.0000,3: 0.0000,"

  for (j in controllers) {
    var controller = controllers[j];
    var d = document.getElementById("controller" + j);
    var buttons = d.getElementsByClassName("button");

    for (i = 0; i < controller.buttons.length; i++) {
      var b = buttons[i];
      var val = controller.buttons[i];
      var pressed = val == 1.0;
      if (typeof(val) == "object") {
        pressed = val.pressed;
        val = val.value;
      }

      var pct = Math.round(val * 100) + "%";
      b.style.backgroundSize = pct + " " + pct;

      if (pressed) {
        b.className = "button pressed";
        //console.log("buttonPressed"+pressed+" :" +b.innerHTML)
        require('electron').remote.getGlobal('sharedObj').button = i;
      } else {
        b.className = "button";
      }
    }

    var axes = d.getElementsByClassName("axis");
    var axistore = ""
    for (i = 0; i < controller.axes.length; i++) {
      var a = axes[i];
      a.innerHTML = i + ": " + controller.axes[i].toFixed(4);
      a.setAttribute("value", controller.axes[i] + 1);
      axistore += a.innerHTML+","
    }
    require('electron').remote.getGlobal('sharedObj').axis = axistore;
  }

  requestAnimationFrame(updateStatus);
}

function scangamepads() {
  var gamepads = navigator.getGamepads ? navigator.getGamepads() : (navigator.webkitGetGamepads ? navigator.webkitGetGamepads() : []);
  for (var i = 0; i < gamepads.length; i++) {
    if (gamepads[i]) {
      if (gamepads[i].index in controllers) {
        controllers[gamepads[i].index] = gamepads[i];
      } else {
        addgamepad(gamepads[i]);
      }
    }
  }
}


window.addEventListener("gamepadconnected", connecthandler);
window.addEventListener("gamepaddisconnected", disconnecthandler);

if (!haveEvents) {
  setInterval(scangamepads, 100);
}