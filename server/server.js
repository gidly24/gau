// server.js
const WebSocket = require('ws');
const http = require('http');
const fs = require('fs');
const path = require('path');

// HTTP-сервер для выдачи HTML
const server = http.createServer((req, res) => {
    if (req.url === '/') {
        fs.readFile(path.join(__dirname, 'index.html'), (err, data) => {
            if (err) {
                res.writeHead(500);
                return res.end("Ошибка загрузки страницы");
            }
            res.writeHead(200, { 'Content-Type': 'text/html' });
            res.end(data);
        });
    }
});

const wss = new WebSocket.Server({ server });

// Список подключённых клиентов (браузеры + ESP32)
wss.on('connection', ws => {
    console.log('Клиент подключился');
    ws.on('message', message => {
        console.log('Получено:', message.toString());

        // Рассылаем всем клиентам
        wss.clients.forEach(client => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(message.toString());
            }
        });
    });
});

// Запуск HTTP+WebSocket сервера
const PORT = 8080;
server.listen(PORT, () => {
    console.log(`Сервер запущен: http://localhost:${PORT}`);
});
