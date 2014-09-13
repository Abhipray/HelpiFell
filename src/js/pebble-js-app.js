// Function to send a message to the Pebble using AppMessage API
function sendMessage() {
	Pebble.sendAppMessage();
	
	// PRO TIP: If you are sending more than one message, or a complex set of messages, 
	// it is important that you setup an ackHandler and a nackHandler and call 
	// Pebble.sendAppMessage({ /* Message here */ }, ackHandler, nackHandler), which 
	// will designate the ackHandler and nackHandler that will be called upon the Pebble 
	// ack-ing or nack-ing the message you just sent. The specified nackHandler will 
	// also be called if your message send attempt times out.
}


function locationSuccess(pos) {
// if location is successful will send message to web

}

function locationError(err) {
// will set default to home address

}

function getLocation() {
  navigator.geolocation.getCurrentPosition(
	  locationSuccess,
		locationError,
		{timeout: 15000, maximumAge:60000)
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
								sendMessage();
							});
							
