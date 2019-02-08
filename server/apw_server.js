const net = require('net')

//console.log("Hello I am Nodejs")
net.createServer(socket => {
  socket.on('data', function(data){
    buf1 = Buffer.alloc(1024)
    buf1.write(data.toString())
    console.log('Echoing: %s', data.toString())
    socket.write(buf1)

    var message = JSON.parse(data.toString())
    console.log(message.type)
  })
  socket.on('error', (err) => {
    //console.log("Error Found")
    //console.log(err)
  })
}).listen(8001)
