const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = 3303;
const ROOT = __dirname;

const mime = {
  '.html': 'text/html',
  '.css':  'text/css',
  '.js':   'application/javascript',
  '.png':  'image/png',
  '.jpg':  'image/jpeg',
  '.jpeg': 'image/jpeg',
  '.svg':  'image/svg+xml',
  '.ico':  'image/x-icon',
  '.json': 'application/json',
  '.woff2':'font/woff2',
};

http.createServer(function(req, res) {
  let filePath = path.join(ROOT, req.url === '/' ? 'index.html' : decodeURIComponent(req.url));
  const ext = path.extname(filePath) || '.html';
  if (!path.extname(filePath)) filePath += '.html';

  fs.readFile(filePath, function(err, data) {
    if (err) {
      res.writeHead(404);
      res.end('Not found');
      return;
    }
    res.writeHead(200, { 'Content-Type': mime[ext] || 'text/plain' });
    res.end(data);
  });
}).listen(PORT, function() {
  console.log('SOLAR 303 server running on port ' + PORT);
});
