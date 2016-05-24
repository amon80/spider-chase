

var IP
var checkstring = "2 packets transmitted, 2 packets received"
var remote = require('electron').remote;

function TestConnection(){
	IP = document.getElementById('ip').value
	console.log("Clicked IP: " + IP)
	var cp = require('child_process');
	var ping = cp.spawnSync('ping', ["-c 2" , IP], { encoding : 'utf8' });
	// uncomment the following if you want to see everything returned by the spawnSync command
	//console.log('ls: ' , ping);
	if(ping.stdout.indexOf(checkstring)>=0){
		console.log("TUTT OK");
		remote.getGlobal('sharedObj').IP = IP 
		cw = remote.getCurrentWindow();
		cw.loadURL('file://' + __dirname + '/index.html')
	}else{
		div = document.getElementById('error');
		div.innerHTML = "CAN'T CONNECT";
	}
}
button = document.getElementById('enter').onclick = TestConnection;