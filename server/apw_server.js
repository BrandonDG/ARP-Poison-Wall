const net = require('net');
const fs  = require('fs');
const lineReader = require('readline');
const prependFile = require('prepend-file');
const mysql = require('mysql');
const uniqid = require('uniqid');
const nodemailer = require('nodemailer');

var transporter = nodemailer.createTransport({
  service: 'gmail',
  auth: {
    user: '',
    pass: ''
  }
});

var mailOptions = {
  from: 'arp.poison.wall@gmail.com',
  to: 'brandon.gillespie08@gmail.com',
  subject: 'Sending Email using Node.js',
  text: 'That was easy!'
};

var connection = mysql.createConnection({
  host     : 'localhost',
  user     : 'root',
  password : '',
  database : 'arp_poison_db'
});

var raw_config = fs.readFileSync("server_conf.json");
var config = JSON.parse(raw_config);
var key = "hW)V,>I>)Bh(T9\0"
//console.log(config.key);

/*
function enter_log(payload) {
  prependFile('client_logs', payload, function (err) {
    if (err) {
      console.log("Error Occurred")
      console.log(err)
    }
  });
} */

function enter_log(mysql_connection, log_json) {
  console.log("Enter Log: " + log_json.TimeStamp);
  var date = new Date(log_json.TimeStamp);
  var time_s = date.toISOString().replace(/T/, ' ').replace(/\..+/, '');
  query = 'INSERT INTO logs (from_a, to_a, time_s) VALUES (?, ?, ?);'
  mysql_connection.query(query, [log_json.From, log_json.To, time_s], function(error, results, field) {
    if (error) throw error;
  });
}

function enter_alert(mysql_connection, alert_json) {
  var date = new Date(alert_json.TimeStamp);
  var end_t = date.toISOString().replace(/T/, ' ').replace(/\..+/, '');
  date.setSeconds(date.getSeconds() - 10);
  var start_t = date.toISOString().replace(/T/, ' ').replace(/\..+/, '');
  query1 = 'SELECT * FROM alerts WHERE from_a = ? AND to_a = ? AND end_t BETWEEN ? AND ?'
  mysql_connection.query(query1, [alert_json.From, alert_json.To, start_t, end_t], function (error, results, fields) {
    if (error) throw error;
    if (results[0] == undefined) {
      console.log("No previous alert found");
      query2 = 'INSERT INTO alerts (alertid, from_a, to_a, start_t, end_t, status) VALUES (?, ?, ?, ?, ?, ?);'
      mysql_connection.query(query2, [uniqid(), alert_json.From, alert_json.To, start_t, end_t, "Active"], function(error, results, field) {
        if (error) throw error;
        // TODO: When doing 100% testing, uncomment this. Do not
        // want emails sent right now.
        /*
        mailOptions.subject = "ARP Poison Attack";
        mailOptions.text = "ARP Poison Attack in Progress from attacker "
            + alert_json.From + " to host " + alert_json.To + " at time " + end_t;
        transporter.sendMail(mailOptions, function(error, info){
          if (error) {
            console.log(error);
          } else {
            console.log('Email sent: ' + info.response);
          }
        }); */
      });
    } else {
      console.log('Alert already exists: ', results[0].alertid);
      mysql_connection.query('UPDATE alerts SET end_t = ? WHERE alertid = ?', [end_t, results[0].alertid], function (error, results, fields) {
        if (error) throw error;
      });
    }
  });
}

function xor_message(input) {
	var output = [];
	for (var i = 0; i < input.length; i++) {
		var charCode = input.charCodeAt(i) ^ key[i % key.length].charCodeAt(0);
		output.push(String.fromCharCode(charCode));
	}
	return output.join("");
}

connection.connect();
net.createServer(socket => {
  socket.on('data', function(data) {
    buf1 = Buffer.alloc(1024);

    buf1.write(data.toString());
    //console.log('Received: %s', data.toString());
    //socket.write(buf1);

    console.log(xor_message(data.toString()));

    //console.log(socket.remoteAddress)
    //var decoded_message = xor_message(data.toString());
    var message;
    try {
      message = JSON.parse(xor_message(data.toString()));

      // Hopefully isn't too slow doing it this way.
      var reader_interface = lineReader.createInterface({
        input: fs.createReadStream('identification.conf')
      });
      reader_interface.on('line', function (line) {
        tokens = line.split(" ");
        if (tokens[0] == message.payload.To) {
          if (tokens[1] == message.password) {
            if (message.type == "alert") {
              console.log("Run alert code");
              //console.log(message.payload);
              enter_alert(connection, message.payload);
            } else if (message.type == "log") {
              console.log("Run log code");
              //console.log(JSON.stringify(message.payload));
              // (JSON.stringify(message.payload)).concat('\n')
              enter_log(connection, message.payload);
            }
          }
        }
      });
    } catch(e) {
      console.log("json parse error");
    }

  })
  socket.on('error', (err) => {
    console.log("Error Occurred")
    console.log(err)
  })
}).listen(8001)
