const express = require('express');
const cors = require('cors');
const jwt = require('jsonwebtoken');
const cookieParser = require('cookie-parser');

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
  return res.send('Welcome!');
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

/*
app.post('/api/authenticate', function(req, res) {
  const { email, password } = req.body;
  User.findOne({ email }, function(err, user) {
    if (err) {
      console.error(err);
      res.status(500)
        .json({
        error: 'Internal error please try again'
      });
    } else if (!user) {
      res.status(401)
        .json({
          error: 'Incorrect email or password'
        });
    } else {
      user.isCorrectPassword(password, function(err, same) {
        if (err) {
          res.status(500)
            .json({
              error: 'Internal error please try again'
          });
        } else if (!same) {
          res.status(401)
            .json({
              error: 'Incorrect email or password'
          });
        } else {
          // Issue token
          const payload = { email };
          const token = jwt.sign(payload, secret, {
            expiresIn: '1h'
          });
          res.cookie('token', token, { httpOnly: true })
            .sendStatus(200);
        }
      });
    }
  });
}); */

app.listen(process.env.PORT || 8080);
