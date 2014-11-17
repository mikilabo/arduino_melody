var express = require('express'),
	http = require('http'),
	mysql = require('mysql'),
	logger = require("morgan"),
	path = require('path'),
    app = express();

//sql connection
var connection = mysql.createConnection({
  host: 'localhost',
  user: 'melody',
  password: 'melody123',
  database: 'melody_data'
});
 
app.set('views', __dirname + '/views');
app.set('view engine', 'ejs');
app.set('port', '3000');

app.use(logger('dev'));
app.use(express.static(path.join(__dirname, 'public')));

server = http.createServer(app); // add
server.listen(app.get('port'), function(){ //add
  console.log("Express server listening on port " + app.get('port'));
});

// societ io
var socketIO = require('socket.io');
var io = socketIO.listen(server);

// melody id
var melody_id = 0; //default id = 0
// arduino ringing?
var arduino_ring = 0; //0->off 1->ring


//Arduino死活監視
var timerObject = null ;
function setTimer(){
	console.log("ring = " + arduino_ring);

	//clear timer
	if( timerObject != null ){
		io.emit('arduino_check',  "1" ); //arduino online
		clearInterval(timerObject);
		timerObject = null ;
	}	

	//set timer
	timerObject = setInterval(function(){ 
		console.log("no access from arduino"); 

		//arduino dead
		io.emit('arduino_check', "0" ); //arduino offline

	}, 10000); //every 10 sec
} 
//start timer
setTimer();

//access to index page 
app.get('/', function(req, res) {
	console.log(" index accessed ");

	var melodies = [];

    //test query
    connection.query( 
        //'select * from Melody where id = 2',
        'select * from Melody ',
        function (err, rows, field) { 
            if (err) {
                throw err;
            }

            //for DEBUG
            for (var i in rows) {
				ide = rows[i].id;
                tone = rows[i].tone;
                tempo = rows[i].tempo;
                name = rows[i].name;
                console.log('tone: ', rows[i].tone);
                console.log('tempo: ', rows[i].tempo);
                console.log('name: ', rows[i].name);
                console.log();

				var melody = {
					id: rows[i].id,
					name: rows[i].name,
					tone: rows[i].tone,
					tempo: rows[i].melody
				};
				melodies.push(melody);
	
            }
            //console.log(result);
            //console.log(field);
        	//End of DB Connection

	    	res.render('index', {melodies: melodies});
		});
});


//access to melody page 
app.get('/melody', function(req, res) {
	console.log("req.counter  " + req.query.counter);
	console.log("req.stop  " + req.query.stop);
	console.log("req.myname  " + req.query.myname);
	console.log("melody accessed id =  " + melody_id);

	//got melody stop requst,
	//then set melody_id = 0 -> return no data
	if( req.query.stop == 1  ){
		melody_id = 0;
		console.log("stop accessed id = " + melody_id);

		arduino_ring = 0; //arduno ringing off

		//Arduino melody off, then start Arduino alive check
		io.emit('stop_event', "stop event received!");
		io.emit('arduino_check',  "1" ); //arduino online
		setTimer();
	}

	//set arduino name 
	if( req.query.myname != "" ){
		io.emit('arduino_name', req.query.myname);
	}

	var tone = "", tempo = "", name = "", name_s1 = "", name_s2 = "";

	/***
	// for DEUBG TEST
	if( melody_id == 1 ) melody_id = 2;
	else melody_id = 1;
	**/

    //DB query
    connection.query( 
        'select * from Melody where id = ' + melody_id, function (err, rows, field) { 
            if (err) {
                throw err;
            }
            console.log(rows);

			if(rows.length > 0 ){
				//exist data
				console.log("melody data exist");
				tone = rows[0].tone;
				tempo = rows[0].tempo;
				name = rows[0].name;
				name_s1 = rows[0].name_s1;
				name_s2 = rows[0].name_s2;
			
				arduino_ring = 1; //arduno ringing on
				io.emit('arduino_check',  "2" ); //arduino online

				//melody mode -> timer off
				clearInterval(timerObject);
				timerObject = null ;
			}else{
				console.log("no melody data");
				name = "NODATA";
				tone = "";
				tempo = "";
				name_s1 = "";
				name_s2 = "";

				arduino_ring = 0; //arduno ringing off
			}
			
		//console.log(field);
		//End of DB Connection

		console.log("name = [" + name + "]");
	    res.render('melody', 
					{name: name,
					tone: tone,
					tempo: tempo,
					name_s1: name_s1,
					name_s2: name_s2}
					);
	});

	//set timer for ardino active check
	setTimer();
});

///
/// socket.io
///
// クライアントが接続してきたときの処理
io.sockets.on('connection', function(socket) {
	var melody_name = "No Melody Selected.";

    socket.on('emit_from_client_select', function(data) {
		//setting melody_id from client
		melody_id = data;
        console.log("selected id = " + melody_id);

    	connection.query( 
        	'select name from Melody where id = ' + melody_id, function (err, rows, field) { 
            if (err) {
                throw err;
            }
            console.log(rows);

			if(rows.length != 1 ){
				meldoy_nane = "data error, pls check DB."
			}else{
				melody_name = rows[0].name;
			}
			
			//End of DB Connection

			console.log("XXX: melody_name = [" + melody_name + "]");
			io.emit('server_melody',  melody_name); 

			//set date to start field
			io.emit('select_date',  0); 
		});

    });

    socket.on('emit_from_client_clear', function(data) {
		melody_id = 0;
		io.emit('arduino_check',  "1" ); //arduino online
    });

	io.emit('server_melody',  melody_name); 
});

