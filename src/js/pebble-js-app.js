// Function to send a message to the Pebble using AppMessage API

var min_major;

function sendMessage() {
  console.log("JS- sendMessage");
	Pebble.sendAppMessage({"status": 0});
}

function forwardMessage(e){
  console.log("JS-Forward message");
  min_major = e.payload.message;
  getLocation();
  sendMessage(); //Send success or failure to Pebble 
}

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
// if location is successful will send message to web
  // Construct URL
  console.log("JS-locationSuccess");
  var url;
  if(min_major == 1)
  {
      console.log("major fall"); 
      url = "http://felldown.herokuapp.com/events/majorfall?lat=" +
      pos.coords.latitude + "&lng=" + pos.coords.longitude;
  }
  else
  {
      url = "http://felldown.herokuapp.com/events/minorfall?lat=" +
      pos.coords.latitude + "&lng=" + pos.coords.longitude;
  }
  console.log("Latitude " + pos.coords.latitude);
  console.log("Longitude " + pos.coords.longitude);
  // Send request to helpifelldown server
  xhrRequest(url, 'GET', 
    function(responseText) {   
        console.log("Returned from sending to server");
    }      
  );
}

function locationError(err) {
// will set default to home address

}

function getLocation() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
		locationError,
    {timeout: 15000, maximumAge:60000}
	);
}

// Called when JS is ready and watchface open
Pebble.addEventListener("ready",
							function(e) {
							console.log("JS is ready");
								
							// Send notification to Web to notify contacts
							//getLocation();
							});
												
// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage",
							function(e) {
								console.log("Received Status: " + e.payload.status);
								forwardMessage(e);
							});
							
