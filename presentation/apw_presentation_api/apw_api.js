const express = require('express');
const cors = require('cors');
const jwt = require('jsonwebtoken');
const cookieParser = require('cookie-parser');
const fs  = require('fs');
const lineReader = require('readline');
const mysql = require('mysql');

const withAuth = require('./auth_layer');
const app = express();

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
    console.log(lines);
    ips.push({ ip: tokens[0] });
  }

  console.log(ips);
  return res.send(ips);
});

app.get('/api/host', withAuth, function(req, res) {
  console.log("api/host request // body = " + req.header('selectedip'));
  var response = { logs: [], alerts: [], status: ''};
  var query = "SELECT * FROM logs WHERE to_a = ?";
  connection.query(query, [req.header('selectedip')], function(error, results, field) {
    if (error) throw error;
    if (results[0] != undefined) {
      response.logs.push(results);
    }
    //console.log(response.logs);
    query = "SELECT * FROM alerts WHERE to_a = ?";
    connection.query(query, [req.header('selectedip')], function(error, results, field) {
      if (error) throw error;
      if (results[0] != undefined) {
        response.alerts.push(results);
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

app.get('/api/secret', withAuth, function(req, res) {
  res.send('You are successfully validated');
});

app.post('/api/authenticate', function(req, res) {
  console.log(req.body);
  const { email, password } = req.body;
  const payload = { email };
  const token = jwt.sign(payload, secret, {
    expiresIn: '1h'
  });
  res.cookie('token', token, { httpOnly: true })
    .sendStatus(200);
});

app.get('/checkToken', withAuth, function(req, res) {
  res.sendStatus(200);
});

app.listen(process.env.PORT || 8080);
