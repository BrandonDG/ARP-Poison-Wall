const express = require('express');
const cors = require('cors');
const jwt = require('jsonwebtoken');
const cookieParser = require('cookie-parser');
const fs  = require('fs');
const lineReader = require('readline');

const withAuth = require('./auth_layer');
const app = express();

const secret = 'mysecretsshhh';

app.use(cors());
app.use(express.json());
app.use(cookieParser());

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
    tokens = lines[0].split(" ");
    ips.push({ ip: tokens[0] });
  }

  console.log(ips);
  return res.send(ips);
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
