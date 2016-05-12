// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.
const Gauge  = require('./gauge.min.js');
const keycode = require('keycode');
const request = require('request');

var remote = require('electron').remote;
var ipcRenderer = require('electron').ipcRenderer;

var dict = {"12":"up","13":"down","15":"right","14":"left"}
 
var oldbutton
var oldaxis;

var velocityLeft = 0;
var velocityRight = 0;
var maxValue = 3000;
var increment = 100;
var gaugeLeft;
var gaugeRight;

function CreateGauge(){
    var opts = {
      lines: 12, // The number of lines to draw
      angle: 0.15, // The length of each line
      lineWidth: 0.44, // The line thickness
      pointer: {
        length: 0.9, // The radius of the inner circle
        strokeWidth: 0.035, // The rotation offset
        color: '#000000' // Fill color
      },
      limitMax: 'false',   // If true, the pointer will not go past the end of the gauge
      colorStart: '#6FADCF',   // Colors
      colorStop: '#8FC0DA',    // just experiment with them
      strokeColor: '#E0E0E0',   // to see which ones work best for you
      generateGradient: true
    };
    var targetLeft = document.getElementById('left'); // your canvas element
    targetLeft.width  = 350;
    targetLeft.height = 200;
    gaugeLeft = new Gauge.Gauge(targetLeft).setOptions(opts);
    gaugeLeft.maxValue = maxValue; // set max gauge value
    gaugeLeft.animationSpeed = 1; // set animation speed (32 is default value)
    gaugeLeft.set(velocityLeft); // set actual value
    //console.log(gaugeLeft);

    remote.getGlobal('sharedObj').Left = gaugeLeft;  
    //require('electron').remote.getGlobal('sharedObject').LeftGauge = gaugeLeft;

    var targetRight = document.getElementById('right'); // your canvas element
    targetRight.width  = 350;
    targetRight.height = 200;
    gaugeRight = new Gauge.Gauge(targetRight).setOptions(opts);
    gaugeRight.maxValue = maxValue; // set max gauge value
    gaugeRight.animationSpeed = 1; // set animation speed (32 is default value)
    gaugeRight.set(velocityRight); // set actual value


    remote.getGlobal('sharedObj').Right = gaugeRight;  


}

function renderGauge(pressed){

        console.log(pressed);

         if(pressed=='up'){
            if(velocityLeft!=velocityRight){
                if (velocityLeft > velocityRight){
                    velocityRight=velocityLeft;
                }
                else{
                    velocityLeft = velocityRight;
                }
            }else{
                if(velocityLeft+increment<= maxValue){
                    velocityLeft+=increment;
                    velocityRight+=increment
                }
            }
            gaugeLeft.set(velocityLeft);
            gaugeRight.set(velocityRight); // set actual value
        }


        if(pressed=='down'){
            if (velocityLeft-increment >= 0 ){
                velocityLeft-=increment;
                gaugeLeft.set(velocityLeft);
            }
            if (velocityRight-increment >= 0){
                velocityRight-=increment;
                gaugeRight.set(velocityRight); // set actual value
            }
        }
        if(pressed=='left'){
            if(velocityLeft+increment<=maxValue){
                velocityLeft+=increment;
                gaugeLeft.set(velocityLeft);
            }
        }
        if(pressed=='right'){
            if(velocityRight+increment<=maxValue){
                velocityRight+=increment;
                gaugeRight.set(velocityRight); // set actual value
            }
        }


}

function ReadKeyboard(){
    window.addEventListener('keydown', function(e) {

        renderGauge(keycode(e));
   
    });
}

setInterval(ReadController, 100);

function ReadController(){
    
    var button = remote.getGlobal('sharedObj').button;
    var axis = remote.getGlobal('sharedObj').axis;
    if(button!=null){
        console.log(remote.getGlobal('sharedObj').button);
        oldbutton = button
        renderGauge(dict[button])
        //SendPackage(button);
    }
    if(axis && axis!="0: 0.0000,1: 0.0000,2: 0.0000,3: 0.0000,"){
        console.log(remote.getGlobal('sharedObj').axis);
        oldaxis = axis;
        //SendPackage(axis);
    }
    //console.log(axis);
}

function SendPackage(button){
    request
        .get('http://127.0.0.1:2222?pressed='+button)
        .on('response', function(response) {
            console.log(response.statusCode) // 200 
            console.log(response.headers['content-type']) // 'image/png' 
        })
}

CreateGauge();
ReadKeyboard();

const controller =  require('./Controller.js')

console.log(ipcRenderer.sendSync('synchronous-message', 'ping')); // prints "pong"

ipcRenderer.on('asynchronous-reply', (event, arg) => {
  console.log(arg); // prints "pong"
});
ipcRenderer.send('asynchronous-message', 'ping');

