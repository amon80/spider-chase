// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.
const Gauge  = require('./gauge.min.js');
const keycode = require('keycode');
// const request = require('request');
const controller =  require('./Controller.js')

var remote = require('electron').remote;

//dict maps numbers corrisponding to controller into directions and/or labels
//a accelerates forward, b does nothing, y is hand brake and x accelerates backward
var dict = {"12":"up","13":"down","15":"right","14":"left", "2":"x", "3":"y","0":"a", "1":"b", "7":"a", "6":"x"}
 
var retro = false;
//state = 1 if moving forward, -1 if moving backward, 0 if halted
var state = 0;
var velocityLeft = 0;
var velocityRight = 0;
var maxValue = 127;
var minValue = 1;
var accelerationIncrement = 50;
var brakeIncrement = accelerationIncrement;
var naturalSpeedIncrement = accelerationIncrement/5;
var cruiseControl = false;
var gaugeLeft;
var gaugeRight;
var turn_costant = 5;
var oldVelocityLeft = velocityLeft;
var oldVelocityRight = velocityRight;
var epsilon_axis = 0.2;

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
	// console.log("In renderGauge");
	// console.log(velocityLeft);
	// console.log(velocityRight);
	gaugeLeft.set(velocityLeft);	
	gaugeRight.set(velocityRight);	
}

//renderGauge is done outside this function
function modifyVelocity(pressed){

        // console.log(pressed);
	//a accelerates forward, b sets a fixed speed (cruise control), y is hand brake and x accelerates backward
	
	if (pressed == 'y') {
		state = 0;
		velocityLeft = minValue;
		velocityRight = minValue;
	}
	else if (pressed == 'b'){
		cruiseControl = true;
	}
	else{//no hand brake from here
		if (state == 1) { //moving forward
			if(pressed == 'a'){
				cruiseControl = false;
				if(velocityLeft+accelerationIncrement<= maxValue){
					velocityLeft+=accelerationIncrement;
				}
				else{
					velocityLeft = maxValue;
				}
				if(velocityRight+accelerationIncrement<= maxValue){
					velocityRight+=accelerationIncrement;
				}
				else{
					velocityRight = maxValue;
				}
			}
			else if(pressed == 'x'){
				cruiseControl = false;
				if(velocityLeft - brakeIncrement>=minValue){
					velocityLeft-=brakeIncrement;
				}
				else{
					velocityLeft=minValue;
				}
				if(velocityRight - brakeIncrement>=minValue){
					velocityRight-=brakeIncrement;
				}
				else{
					velocityRight=minValue;
				}
				if(velocityLeft == minValue && velocityRight == minValue){
					state = 0;
					// alert("Fermo by brake");
				}
			}
		}else {
			if(state == -1){ //moving backward
				if(pressed == 'x'){
					cruiseControl = false;
					if(velocityLeft+accelerationIncrement<= maxValue){
						velocityLeft+=accelerationIncrement;
					}
					else{
						velocityLeft = maxValue;
					}
					if(velocityRight+accelerationIncrement<= maxValue){
						velocityRight+=accelerationIncrement;
					}
					else{
						velocityRight = maxValue;
					}
				}
				else if(pressed == 'a'){
					cruiseControl = false;
					if(velocityLeft-brakeIncrement>=minValue){
						velocityLeft-=brakeIncrement;
					}
					else{
						velocityLeft=minValue;
					}
					if(velocityRight-brakeIncrement>=minValue){
						velocityRight-=brakeIncrement;
					}
					else{
						velocityRight=minValue;
					}
					if(velocityLeft == minValue && velocityRight ==minValue){
						state = 0;
					}
				}
			}
			else{ //quiet, state == 0
				if (pressed == 'x') {
					state = -1;
					transitionBackward();
				}
				if (pressed == 'a') {
					state = 1;
					transitionForward();
				}
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
setInterval(ReadController, 200);
function ReadController(){
	var button = remote.getGlobal('sharedObj').button;
	var axis = remote.getGlobal('sharedObj').axis;
	//check buttons
	if(button!=null){
		// console.log(remote.getGlobal('sharedObj').button);
		document.getElementById("type").src="./img/xboxPad.jpg";
		modifyVelocity(dict[button]);
	}/*
	else{//slowly decrease velocity based on actual status
		if(!cruiseControl){
			if(velocityLeft-naturalSpeedIncrement>= minValue){
				velocityLeft-=naturalSpeedIncrement;
			}
			else{
				velocityLeft= minValue;
			}
			if(velocityRight-naturalSpeedIncrement>= minValue){
				velocityRight-=naturalSpeedIncrement;
			}
			else{
				velocityRight= minValue;
			}
			if(velocityLeft ==  minValue && velocityRight == minValue){
				state = 0;
			}
		}
	}*/
	//check axis
	if(axis){
		// console.log(axis);
		turn_string = axis.split(",")[0];
		turn_string = turn_string.split(":")[1];
		turn_float = parseFloat(turn_string);
		// console.log(turn_float);
		if(Math.abs(turn_float) > epsilon_axis){
			if (turn_float > 0) {//right
				velocityLeft -= Math.round(turn_float * turn_costant);	
				if(velocityLeft < minValue)
					velocityLeft = minValue;
			}else{//left
				turn_float *= -1;
				velocityRight -= Math.round(turn_float * turn_costant);
				if(velocityRight < minValue)
					velocityRight= minValue;
			}
			// console.log(turn_string);
		}else if(velocityLeft != velocityRight){
			velocityLeft > velocityRight ? velocityRight = velocityLeft : velocityLeft = velocityRight;
		}
	}
	//refresh gauges
	renderGauge();
	//if speeds are different(caused by acceleration, deceleration or turning), send a package to robot
	if (velocityLeft != oldVelocityLeft || velocityRight != oldVelocityRight) {
		SendPackage();
		oldVelocityLeft = velocityLeft;
		oldVelocityRight = velocityRight;
	}
}

function SendPackage(){
	if(state == -1)//if retro
		stri = "m0"+pad(velocityLeft,3)+""+pad(velocityRight,3);
	else if(state == 1)
		stri = "m0"+pad(maxValue+velocityLeft+1,3)+""+pad(maxValue+velocityRight+1,3);
	else
		stri = "m0128128"

	// console.log(stri);
	/*request
		.get('http://192.168.4.1/?c='+stri)
		.on('response', function(response) {
			//console.log(response.statusCode) // 200 
			//console.log(response.headers['content-type']) // 'image/png' 
			console.log(response)
	})*/
	// or more concisely

	var cp = require('child_process');

	var ls = cp.spawnSync('curl', ["-m 2" ,'http://192.168.4.1/?c='+stri], { encoding : 'utf8' });
	// uncomment the following if you want to see everything returned by the spawnSync command
	console.log('ls: ' , ls);
	console.log('stdout here: \n' + ls.stdout);
	/*
	var res = request('GET', 'http://192.168.4.1/?c='+stri);
	console.log(res.getBody());*/
}

function pad(n, width, z) {
  z = z || '0';
  n = n + '';
  return n.length >= width ? n : new Array(width - n.length + 1).join(z) + n;
}

CreateGauge();
ReadKeyboard();
