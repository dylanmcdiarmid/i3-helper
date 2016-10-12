const net = require('net');
const fs = require('fs');
const dgram = require('dgram');
const i3 = require('i3').createClient();
const conf = require('./lib/config.js');
const commands = require('./lib/commands');

var commandQueue = [];
var isBooted = false;

var i3State = {
  workspaces: null
}

var server = net.createServer(handleConnection);

function startServer() {
  console.log("Starting server listening to ", conf.localSocket);
  server.listen(conf.localSocket);
  // Boot 
}

function handleConnection(sock) {
  sock.on('data', function (buf) {
    let input = buf.toString();
    if (!isBooted) {
      commandQueue.push(input);
      return null;
    }
    processInput(input);
  });
}

function processInput(input) {
  try {
    let input = buf.toString().split(' ');
    if (!input || !input.length) {
      throw new Error('No input found.');
    }
    let cmd = commands[input[0]];
    let args = input.slice(1);
    if (!cmd) {
      console.error('Unknown command', cmd);
      return null;
    }
    cmd(i3State, args, onCommandComplete.bind(cmd));
  } catch (e) {
    console.error('Error handling socket input.', e);
  }
}

function onCommandComplete(cmd, err, result) {
  if (err) {
    console.error('Error running cmd', err);
    return null;
  }
  if (result) {
    console.log('Result of ' + cmd, result);
  }
}

var reconnectAttempted = false;
server.on('error', function (err) {
  if (err.code && err.code === 'EADDRINUSE' && reconnectAttempted === false) {
    reconnectAttempted = true;
    var clientSocket = new net.Socket();
    clientSocket.on('error', function (e) {
      if (e.code === 'ECONNREFUSED') {
        console.log("Socket exists, deleting and retrying");
        fs.unlinkSync(conf.localSocket);
        startServer();
      } else {
        console.error('Received EADDRINUSE, then unexpected on attempted connection: ', err);
      }
    });
    clientSocket.connect({ path: conf.localSocket }, function () {
      console.error('Socket server error, giving up.', err);
      process.exit();
    });
  }
});

startServer();
