const net = require('net');
const fs  = require('fs');
const lineReader = require('readline');
const prependFile = require('prepend-file');

function enter_log(payload) {
  prependFile('client_logs', payload, function (err) {
    if (err) {
      console.log("Error Occurred")
      console.log(err)
    }
  });
}

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
