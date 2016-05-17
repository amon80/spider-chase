// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.
const Gauge  = require('./gauge.min.js');
const keycode = require('keycode');
const request = require('request');
const controller =  require('./Controller.js')

var remote = require('electron').remote;
var ipcRenderer = require('electron').ipcRenderer;

//dict maps numbers corrisponding to controller into directions and/or labels
//a accelerates forward, b does nothing, y is hand brake and x accelerates backward
var dict = {"12":"up","13":"down","15":"right","14":"left", "2":"x", "3":"y","0":"a", "1":"b"}
 
var oldbutton
var oldaxis;

var retro = false;
//state = 1 if moving forward, -1 if moving backward, 0 if halted
var state = 0;
var velocityLeft = 0;
var velocityRight = 0;
var maxValue = 127;
var increment = 10;
var cruiseControl = false;
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
      limitMax: 'true',   // If true, the pointer will not go past the end of the gauge
      colorStart: '#6FADCF',   // Colors
      colorStop: '#8FC0DA',    // just experiment with them
      strokeColor: '#E0E0E0',   // to see which ones work best for you
      generateGradient: true
    };
    var targetLeft = document.getElementById('left'); // your canvas element
    targetLeft.width  = 320;
    targetLeft.height = 200;
    gaugeLeft = new Gauge.Gauge(targetLeft).setOptions(opts);
    gaugeLeft.maxValue = maxValue; // set max gauge value
    gaugeLeft.animationSpeed = 1; // set animation speed (32 is default value)
    gaugeLeft.set(velocityLeft); // set actual value
    //console.log(gaugeLeft);

    remote.getGlobal('sharedObj').Left = gaugeLeft;  
    //require('electron').remote.getGlobal('sharedObject').LeftGauge = gaugeLeft;

    var targetRight = document.getElementById('right'); // your canvas element
    targetRight.width  = 320;
    targetRight.height = 200;
    gaugeRight = new Gauge.Gauge(targetRight).setOptions(opts);
    gaugeRight.maxValue = maxValue; // set max gauge value
    gaugeRight.animationSpeed = 1; // set animation speed (32 is default value)
    gaugeRight.set(velocityRight); // set actual value


    remote.getGlobal('sharedObj').Right = gaugeRight;  


}

function transitionBackward() {
	document.getElementById("arrow").src="./img/Down.png";
	var opts = {
		lines: 12, // The number of lines to draw
		angle: 0.15, // The length of each line
		lineWidth: 0.44, // The line thickness
		pointer: {
		length: 0.9, // The radius of the inner circle
		strokeWidth: 0.035, // The rotation offset
		color: '#000000' // Fill color
		},
		limitMax: 'true',   // If true, the pointer will not go past the end of the gauge
		colorStart: '#CFABA1',   // Colors
		colorStop: '#DA0000',    // just experiment with them
		strokeColor: '#E0E0E0',   // to see which ones work best for you
		generateGradient: true
	};
	gaugeLeft.setOptions(opts);
	gaugeLeft.maxValue = maxValue; // set max gauge value
	gaugeLeft.animationSpeed = 1; // set animation speed (32 is default value)

	gaugeRight.setOptions(opts);
	gaugeRight.maxValue = maxValue; // set max gauge value
	gaugeRight.animationSpeed = 1; // set animation speed (32 is default value)
}

function transitionForward() {
	document.getElementById("arrow").src="./img/Up.png";
	var opts = {
		lines: 12, // The number of lines to draw
		angle: 0.15, // The length of each line
		lineWidth: 0.44, // The line thickness
		pointer: {
		length: 0.9, // The radius of the inner circle
		strokeWidth: 0.035, // The rotation offset
		color: '#000000' // Fill color
		},
		limitMax: 'true',   // If true, the pointer will not go past the end of the gauge
		colorStart: '#6FADCF',   // Colors
		colorStop: '#8FC0DA',    // just experiment with them
		strokeColor: '#E0E0E0',   // to see which ones work best for you
		generateGradient: true
	};
	gaugeLeft.setOptions(opts);
	gaugeLeft.maxValue = maxValue; // set max gauge value
	gaugeLeft.animationSpeed = 1; // set animation speed (32 is default value)

	gaugeRight.setOptions(opts);
	gaugeRight.maxValue = maxValue; // set max gauge value
	gaugeRight.animationSpeed = 1; // set animation speed (32 is default value)
}

function renderGauge() {
	gaugeLeft.set(velocityLeft);	
	gaugeRight.set(velocityRight);	
}

//renderGauge is done outside this function
function modifyVelocity(pressed){

        console.log(pressed);
	//a accelerates forward, b does nothing(cruise control??), y is hand brake and x accelerates backward
	
	if (pressed == 'y') {
		// console.log("hand brake");
		state = 0;
		velocityLeft = 0;
		velocityRight = 0;
	}
	else if (pressed == 'b'){
		cruiseControl = true;
	}
	else{//no hand brake from here
		if (state == 1) { //moving forward
			if(pressed == 'a'){
				cruiseControl = false;
				if(velocityLeft+increment<= maxValue){
					velocityLeft+=increment;
				}
				else{
					velocityLeft = maxValue;
				}
				if(velocityRight+increment <= maxValue){
					velocityRight+=increment;
				}
				else{
					velocityRight = maxValue;
				}
			}
			else if(pressed == 'x'){
				cruiseControl = false;
				if(velocityLeft-increment*3>= 0){
					velocityLeft-=increment*10;
				}
				else{
					velocityLeft=0;
				}
				if(velocityRight-increment*3>= 0){
					velocityRight-=increment*10;
				}
				else{
					velocityRight=0;
				}
				if(velocityLeft == 0 && velocityRight == 0){
					state = 0;
				}

			}
		}		
		else if(state == -1){ //moving backward
			if(pressed == 'x'){
				cruiseControl = false;
				if(velocityLeft+increment<= maxValue){
					velocityLeft+=increment;
				}
				else{
					velocityLeft = maxValue;
				}
				if(velocityRight+increment <= maxValue){
					velocityRight+=increment;
				}
				else{
					velocityRight = maxValue;
				}
			}
			else if(pressed == 'a'){
				cruiseControl = false;
				if(velocityLeft-increment*3>= 0){
					velocityLeft-=increment*10;
				}
				else{
					velocityLeft=0;
				}
				if(velocityRight-increment*3>= 0){
					velocityRight-=increment*10;
				}
				else{
					velocityRight=0;
				}
				if(velocityLeft == 0 && velocityRight == 0){
					state = 0;
				}

			}
		} else{ //quiet
			if (pressed == 'x') {
				// console.log("Retro");
				state = -1;
				transitionBackward();
			}
			if (pressed == 'a') {
				// console.log("Forward");
				state = 1;
				transitionForward();
			}
		}
		if(pressed=='left'){
		    if(velocityLeft+increment<=maxValue){
			velocityLeft+=increment;
		    }
		}
		if(pressed=='right'){
		    if(velocityRight+increment<=maxValue){
			velocityRight+=increment;
		    }
		}
	}
}

//Keyboard can be handled with EventListener...
function ReadKeyboard(){
    window.addEventListener('keydown', function(e) {
        document.getElementById("type").src="./img/KeyBoard.png";
        modifyVelocity(keycode(e));
    });
}

//... while controller needs polling
setInterval(ReadController, 100);

function ReadController(){
    
	var button = remote.getGlobal('sharedObj').button;
	var axis = remote.getGlobal('sharedObj').axis;
	if(button!=null){
		console.log(remote.getGlobal('sharedObj').button);
		document.getElementById("type").src="./img/xboxPad.jpg";
		oldbutton = button
		modifyVelocity(dict[button]);
		// SendPackage(button);
	}
	else{//slowly decrease velocity based on actual status
		if(!cruiseControl){
			if(velocityLeft-increment>= 0){
				velocityLeft-=increment;
			}
			else{
				velocityLeft=0;
			}
			if(velocityRight-increment>= 0){
				velocityRight-=increment;
			}
			else{
				velocityRight=0;
			}
			if(velocityLeft == 0 && velocityRight == 0){
				state = 0;
			}
		}
	}
	renderGauge();
	if(axis && axis!="0: 0.0000,1: 0.0000,2: 0.0000,3: 0.0000,"){
		console.log(remote.getGlobal('sharedObj').axis);
		oldaxis = axis;
		// SendPackage(axis);
	}
	//console.log(axis);
}

function SendPackage(button){
    if(retro)
        stri = "p0"+pad(velocityLeft,3)+""+pad(velocityRight,3);
    else
        stri = "p0"+pad(velocityLeft*2,3)+""+pad(velocityRight*2,3);
    console.log(stri);
    request
        .get('http://192.168.4.1?c='+stri)
        .on('response', function(response) {
            console.log(response.statusCode) // 200 
            console.log(response.headers['content-type']) // 'image/png' 
        })
}

function pad(n, width, z) {
  z = z || '0';
  n = n + '';
  return n.length >= width ? n : new Array(width - n.length + 1).join(z) + n;
}

CreateGauge();
ReadKeyboard();
