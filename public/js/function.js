$(function() {
	var socket = io.connect();

	//arduino status 0->dead, 1->alive
	var arduino_status = 0;

	//melody status 0->melody off, 1->melody on
	var melody_status = 0;

	//Melody selected
	$("#send").click(function(){
		var myid = $('#melody_id').val();
		console.log("melody_id:" + myid );

		//Send melody id to server
		//$('#melody_msg').text('melody_id = ' + myid);
		socket.emit('emit_from_client_select', myid);


		//button diabled 
		$('#send').attr('disabled', true);  //select button diabled
	
		$('#end').val("");
		melody_status = 1;	
	});

	//select date
	socket.on('select_date', function (data) {
		//set date to start field
		var getTime = jQuery . now();
		var date = new Date( getTime ) . toLocaleString();
		$('#start').val(date);
	});

	//Melody Stopped
	socket.on('stop_event', function (data) {
		console.log(data);

		//set date to end field
		var getTime = jQuery . now();
		var date = new Date( getTime ) . toLocaleString();
		$('#end').val(date);

		//button abled 
		$('#send').attr('disabled', false);  

		$('#melody_status').val("N/A");

		melody_status = 0;	
	});

	//Clear button pushed
	$("#clear").click(function(){
		$('#start').val("");
		$('#end').val("");
		$('#send').attr('disabled', false); //select button enable
		melody_status = 0;	

		socket.emit('emit_from_client_clear', 0);
	});

	//arduino alive check
	socket.on('arduino_check', function (data) {
		console.log("Arduino status check:" + data);
		if(data == 1){
			//Arduino Online
			arduino_status = 1;
			$('#arduino_status').val("Arduino ONLINE (Waiting)");

			if(melody_status == 0){
				$('#send').attr('disabled', false); //select button enable
			}
		}else if(data == 2){
			//Arduino ringing
			$('#arduino_status').val("Arduino Ringing");
		}else{
			//Arduino Offline
			arduino_status = 0;
			$('#arduino_status').val("Arduino OFFLINE");

			$('#send').attr('disabled', true);  //select button diabled
		}
	});

	// Socket.IO server connection error
	socket.io.on('connect_error', function(err) {
		//console.log('Error connecting to server');
		$('#arduino_status').val("Server Offline");
		$('#melody_status').val("N/A");
	});

	//set arduino name
	socket.on('arduino_name', function (data) {
		console.log("arduino name:" + data);
		$('#arduino_name').val(data);
	});

	//server melody
	socket.on('server_melody', function (data) {
		console.log("ZZZ: server_melody id :" + data);
		$('#melody_status').val(data);
	});

});

