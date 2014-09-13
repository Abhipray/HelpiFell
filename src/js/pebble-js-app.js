// Function to send a message to the Pebble using AppMessage API
var dictionary = {
  "KEY_FALL_STATUS" = fall_value;
};

function sendMessage() {
	Pebble.sendAppMessage(dictionary, 
	  // ack message to console
	  function(e) {
		  console.log("Call for help successful!");
		},
		// nack message to console
		function(e) {
		  console.log("Call for help unsuccessful");
    }			
	
);

}

function forwardMessage(){
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
  var url = "http://felldown.herokuapp.com/events/majorfall?" +
      pos.coords.latitude + "&lng=" + pos.coords.longitude;

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
							getLocation();
							});
												
// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage",
							function(e) {
								console.log("Received Status: " + e.payload.status);
								forwardMessage();
							});
							
