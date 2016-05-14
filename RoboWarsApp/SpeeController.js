const keycode = require('keycode');
var velocity = 0;
gaugeLeft = require('electron').remote.getGlobal('gauges').left;
gaugeRight = require('electron').remote.getGlobal('gauges').right;

console.dir(gaugeRight);

var ReadKeyboard = function ReadKeyboard(){
	window.addEventListener('keydown', function(e) {

	  	console.log("You pressed", keycode(e))
	    document.getElementById('press').innerHTML = "<h1>You pressed: " + keycode(e)+"</h1>";

		if(keycode(e)=='up'){
			velocity+=100;
			velocity%=3000;
			gaugeLeft.set(velocity);
			gaugeRight.set(velocity); // set actual value
		}
		if(keycode(e)=='down'){
			velocity-=100;
			if(velocity<0)
				velocity=0
			velocity%=3000;
			gaugeLeft.set(velocity);
			gaugeRight.set(velocity); // set actual value
		}
		if(keycode(e)=='left'){
			velocity+=100;
			velocity%=3000;
			gaugeLeft.set(velocity);
		}
		if(keycode(e)=='right'){
			velocity+=100;
			velocity%=3000;
			gaugeRight.set(velocity); // set actual value
		}

	  
	 });
}

export default ReadKeyboard