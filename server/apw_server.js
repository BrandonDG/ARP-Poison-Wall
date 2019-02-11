const net = require('net');
const fs  = require('fs');
const lineReader = require('readline');
const prependFile = require('prepend-file');
const mysql = require('mysql');
const uniqid = require('uniqid');

function enter_log(payload) {
  prependFile('client_logs', payload, function (err) {
    if (err) {
      console.log("Error Occurred")
      console.log(err)
    }
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
      mysql_connection.query(query2, [uniqid(), alert_json.From, alert_json.To, start_t, end_t, "Active"])
    } else {
      console.log('The solution is: ', results[0].alertid);
    }
  });
}


var connection = mysql.createConnection({
  host     : 'localhost',
  user     : 'root',
  password : '',
  database : 'arp_poison_db'
});

connection.connect();

//console.log("Hello I am Nodejs")
net.createServer(socket => {
  socket.on('data', function(data){
    buf1 = Buffer.alloc(1024);
    buf1.write(data.toString());
    console.log('Echoing: %s', data.toString());
    socket.write(buf1);

    var message = JSON.parse(data.toString());
    //console.log(socket.remoteAddress)

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
            console.log(message.payload);
            enter_alert(connection, message.payload);
          } else if (message.type == "log") {
            console.log("Run log code");
            console.log(JSON.stringify(message.payload));
            enter_log((JSON.stringify(message.payload)).concat('\n'));
          }
        }
      }
    });

  })
  socket.on('error', (err) => {
    console.log("Error Occurred")
    console.log(err)
  })
}).listen(8001);


/*
const fs = require('fs');
const readline = require('readline');

const rl = readline.createInterface({
  input: fs.createReadStream('identification.conf'),
  crlfDelay: Infinity
});

rl.on('line', (line) => {
  tokens = line.split(" ");
  if (tokens[0] == message.payload.To) {
    console.log("validated part 1 passed")
    if (tokens[1] == message.password) {
      console.log("validated part 2 passed")
      console.log(message.type);
    }
  }
});

var lineReader = require('readline').createInterface({
  input: require('fs').createReadStream('identification.conf')
});

lineReader.on('line', function (line) {
  console.log('Line from file:', line);
}); */
