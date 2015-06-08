/*jslint browser: true, sloppy: true, white: true */
/*global Pebble */
//-----------------------------------------------------------------------------
// Lines above are for jslint, the JavaScript verifier.  http://www.jslint.com/
//-----------------------------------------------------------------------------

(function() {
	function showConfiguration() {
		console.log("showConfiguration");
		var url = "http://webonastick.com/watchfaces/dress-watch/config/";
		url += "?showDate="    + encodeURIComponent(localStorage.getItem("showDate"));
		url += "&showBattery=" + encodeURIComponent(localStorage.getItem("showBattery"));
		url += "&useBoldFont=" + encodeURIComponent(localStorage.getItem("useBoldFont"));
		console.log(url);
		Pebble.openURL(url);
	}
	function setConfigFrom(o) {
		var message = {};
		message[0] = o.showDate;
		message[1] = o.showBattery;
		message[2] = o.useBoldFont;
		localStorage.setItem("showDate",    o.showDate);
		localStorage.setItem("showBattery", o.showBattery);
		localStorage.setItem("useBoldFont", o.useBoldFont);
		console.log("showDate: " + o.showDate);
		console.log("showBattery: " + o.showBattery);
		console.log("useBoldFont: " + o.useBoldFont);
		Pebble.sendAppMessage(message);
	}
	function configurationClosed(e) {
		console.log("configurationClosed " + JSON.stringify(e));
		if (e.response && e.response !== "CANCELLED") {
			try {
				var settings = JSON.parse(decodeURIComponent(e.response));
				if (Object.keys(settings).length <= 0) {
					return;
				}
				setConfigFrom(settings);
			} catch (ignore) {
			}
		}
	}
	function appmessage(data) {
		console.log("appmessage " + JSON.stringify(data));
		try {
			setConfigFrom(data);
		} catch (ignore) {
		}
	}
	function webviewclosed(e) {
		console.log("webviewclosed " + JSON.stringify(e));
		if (e.response && e.response !== 'CANCELLED') {
			try {
				var settings = JSON.parse(decodeURIComponent(e.response));
				if (Object.keys(settings).length <= 0) {
					return; 
				}
				setConfigFrom(settings);
			} catch (ignore) {
			}
		}
	}
	Pebble.addEventListener("ready", function(e) {
		console.log("ready and running!");
	});
	Pebble.addEventListener("appmessage", appmessage);
	Pebble.addEventListener("showConfiguration", showConfiguration);
	Pebble.addEventListener("configurationClosed", configurationClosed);
	Pebble.addEventListener("webviewclosed", webviewclosed);
}());
