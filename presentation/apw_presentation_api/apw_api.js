const express = require('express');
const cors = require('cors');
const jwt = require('jsonwebtoken');
const cookieParser = require('cookie-parser');
const fs  = require('fs');
const lineReader = require('readline');
const mysql = require('mysql');
const net = require('net');
const exec = require('child_process').exec;

const withAuth = require('./auth_layer');
const app = express();

var key = "hW)V,>I>)Bh(T9\0"
const secret = 'mysecretsshhh';
var connection = mysql.createConnection({
  host     : 'localhost',
  user     : 'root',
  password : '',
  database : 'arp_poison_db'
});

app.use(cors());
app.use(express.json());
app.use(cookieParser());

connection.connect();

function xor_message(input) {
	var output = [];
	for (var i = 0; i < input.length; i++) {
		var charCode = input.charCodeAt(i) ^ key[i % key.length].charCodeAt(0);
		output.push(String.fromCharCode(charCode));
	}
	return output.join("");
}

app.get('/', function (req, res) {
 return res.send('Hello world');
});

app.get('/api/auth', function(req, res) {
  return res.send('True');
});

app.get('/api/home', function(req, res) {
  var ips = [];
  var lines = fs.readFileSync('/home/brandondg/Documents/BTECH_T4/COMP8045/ARPPoisonWall/server/identification.conf', 'utf-8')
    .split('\n')
    .filter(Boolean);

  for (var i = 0; i < lines.length; i++) {
    tokens = lines[i].split(" ");
    ips.push({ ip: tokens[0] });
  }

  console.log(ips);
  return res.send(ips);
});

app.get('/api/host', withAuth, function(req, res) {
  console.log("api/host request // body = " + req.header('selectedip'));
  var response = { logs: [], alerts: [], status: ''};
  //var query = "SELECT * FROM logs WHERE to_a = ?";
  var query = "SELECT * FROM logs WHERE to_a = ? ORDER BY time_s DESC";
  connection.query(query, [req.header('selectedip')], function(error, results, field) {
    if (error) throw error;
    if (results[0] != undefined) {
      //response.logs.push(results);
      for (var i = 0; i < results.length; i++) {
        response.logs.push({
          logid: results[i].logid,
          time_s: results[i].time_s,
          from_a: results[i].from_a,
          to_a: results[i].to_a
        });
      }
    }
    //console.log(response.logs);
    query = "SELECT * FROM alerts WHERE to_a = ? ORDER BY end_t DESC";
    connection.query(query, [req.header('selectedip')], function(error, results, field) {
      if (error) throw error;
      if (results[0] != undefined) {
        for (var i = 0; i < results.length; i++) {
          response.alerts.push({
            alertid: results[i].alertid,
            end_t: results[i].end_t,
            start_t: results[i].start_t,
            from_a: results[i].from_a,
            to_a: results[i].to_a,
            status: results[i].status
          });
        }
      }
      //console.log(response.alerts);
      query = "SELECT * FROM alerts WHERE to_a = ? AND status = ?";
      connection.query(query, [req.header('selectedip'), 'Active'], function(error, results, field) {
        if (error) throw error;
        if (results[0] != undefined) {
          response.status = 'Active';
        } else {
          response.status = 'Inactive';
        }
        console.log(response.logs);
        console.log(response.alerts);
        console.log(response.status);

        return res.send(response);
      });
    });
  });
});

app.put('/api/changestatus', withAuth, function(req, res) {
  console.log("api/changestatus request // body = " + req.header('selectedalert'));
  console.log("api/changestatus request // body = " + req.header('status'));
  var changeto = "";
  if (req.header('status') == "Active") {
    changeto = "Inactive";
  } else {
    changeto = "Active"
  }
  query = "UPDATE alerts SET status = ? WHERE alertid = ?";
  connection.query(query, [changeto, req.header('selectedalert')], function(error, results, field) {
    if (error) throw error;
    return res.send("Changed");
  });
});

app.get('/api/secret', withAuth, function(req, res) {
  res.send('You are successfully validated');
});

app.post('/api/configuration', withAuth, function(req, res) {
  var client = new net.Socket();
  buf1 = Buffer.alloc(1024);
  buf2 = Buffer.alloc(1024);
  message = {};
  config_changes = {};
  ips = [];

  if (req.body.router_mac === "") {
    config_changes.router_mac = "Empty";
  } else {
    config_changes.router_mac = req.body.router_mac;
  }
  if (req.body.router_ip === "") {
    config_changes.router_ip = "Empty";
  } else {
    config_changes.router_ip = req.body.router_ip;
  }
  config_changes.threshold = req.body.threshold;
  if (req.body.password === "") {
    config_changes.password = "Empty";
  } else {
    config_changes.password = req.body.password;
  }

  var lines = fs.readFileSync('/home/brandondg/Documents/BTECH_T4/COMP8045/ARPPoisonWall/server/identification.conf', 'utf-8')
    .split('\n')
    .filter(Boolean);

  message.password = "password1";
  message.payload = config_changes;

  for (var i = 0; i < lines.length; i++) {
    tokens = lines[i].split(" ");
    ips.push(tokens[0]);
  }

  // TODO::
  // We will only establish a connection to ips[2] (127.0.0.1)
  // to test. Later these values will be placed into the loop above
  // tokens[0] will be the IP to configure, and tokens[1] are the
  // password, will need to update password between sending.
  client.connect(8045, '127.0.0.1', function() {
  	console.log('Connected');
    buf1.write(JSON.stringify(message));
    buf2.write(xor_message(buf1.toString('ascii')));
  	client.write(buf2);
  });

  /*
  var replace = "sed -i '/" + "127.0.0.1" + "/c\\" + "127.0.0.1 " + config_changes.password + "' "
      + "/home/brandondg/Documents/BTECH_T4/COMP8045/ARPPoisonWall/server/identification.conf";
  console.log(replace);
  exec(replace); */

  client.on('close', function() {
  	console.log('Connection closed');
  });

  res.sendStatus(200);
});

app.post('/api/authenticate', function(req, res) {
  console.log(req.body);
  const { email, password } = req.body;
  const payload = { email };

  // TODO: PUT IN SOME VALIDATIONS AND CHECKS

  query = "SELECT * FROM users WHERE username = ? AND password = ?";
  connection.query(query, [email, password], function(error, results, field) {
    if (error) throw error;
    if (results[0] != undefined) {
      const token = jwt.sign(payload, secret, {
        expiresIn: '1h'
      });
      res.cookie('token', token, { httpOnly: true })
        .sendStatus(200);
    } else {
      res.sendStatus(403);
    }
  });
});

app.get('/checkToken', withAuth, function(req, res) {
  res.sendStatus(200);
});

app.listen(process.env.PORT || 8080);
