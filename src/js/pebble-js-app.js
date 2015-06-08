/*jslint browser: true, sloppy: true, white: true */
/*global Pebble */
//-----------------------------------------------------------------------------
// Lines above are for jslint, the JavaScript verifier.  http://www.jslint.com/
//-----------------------------------------------------------------------------

(function() {
	function showConfiguration() {
		var url = "http://webonastick.com/watchfaces/dress-watch/config/";
		url += "?showDate="      + encodeURIComponent(localStorage.getItem("showDate"));
		url += "&showBattery="   + encodeURIComponent(localStorage.getItem("showBattery"));
		url += "&useBoldFont="   + encodeURIComponent(localStorage.getItem("useBoldFont"));
		url += "&useLargerFont=" + encodeURIComponent(localStorage.getItem("useLargerFont"));
		Pebble.openURL(url);
	}
	function setConfigFrom(o) {
		var message = {};
		message[0] = o.showDate;
		message[1] = o.showBattery;
		message[2] = o.useBoldFont;
		message[3] = o.useLargerFont;
		localStorage.setItem("showDate",      o.showDate);
		localStorage.setItem("showBattery",   o.showBattery);
		localStorage.setItem("useBoldFont",   o.useBoldFont);
		localStorage.setItem("useLargerFont", o.useLargerFont);
		Pebble.sendAppMessage(message);
	}
	function configurationClosed(e) {
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
		try {
			setConfigFrom(data);
		} catch (ignore) {
		}
	}
	function webviewclosed(e) {
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
	Pebble.addEventListener("appmessage", appmessage);
	Pebble.addEventListener("showConfiguration", showConfiguration);
	Pebble.addEventListener("configurationClosed", configurationClosed);
	Pebble.addEventListener("webviewclosed", webviewclosed);
}());
