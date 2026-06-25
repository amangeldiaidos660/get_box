# GetBox FastAPI Stage 1

Stage 1 covers the HTTP interface for GetBox:

- FastAPI project scaffold
- cell control endpoints
- request/response schemas
- command tracking by `request_id`

## API surface

- `GET /` service summary
- `GET /api/v1/health` health check
- `POST /api/v1/cells/open` create an open-cell command
- `GET /api/v1/cells/commands/{request_id}` read command status

## Python dependencies

```txt
fastapi>=0.110
uvicorn[standard]>=0.29
pydantic>=2.7
```

## Suggested run commands

Install dependencies:

```powershell
python -m pip install -r requirements.txt
```

Run the app:

```powershell
uvicorn app.main:app --reload
```


## Board configs

Board: ESP32 Dev Module (ESP32 by Espressif Systems.)

Port: COM7 Serial Port (USB)


## Docker infrastructure

Docker Compose содержит два сервиса:

- `api` — FastAPI;
- `mqtt` — Eclipse Mosquitto.

Mosquitto использует:

- внутренний порт `1883` для FastAPI;
- публичный порт `8883` с TLS для ESP32.

Анонимный доступ запрещён. Для FastAPI и ESP32 используются разные пользователи и пароли.

Перед запуском необходимы:

```text
.env
secrets/mqtt_api_password.txt
secrets/mqtt_device_password.txt
docker/mosquitto/certs/fullchain.pem
docker/mosquitto/certs/privkey.pem
```

MQTT доступен для ESP32 по адресу:

```text
SERVER_IP:8883
```

FastAPI подключается к Mosquitto внутри Docker-сети:

```text
mqtt:1883
```

Порт `1883` не публикуется на хосте.

Перед запуском необходимо создать локальные файлы:

```text
.env
secrets/mqtt_api_password.txt
secrets/mqtt_device_password.txt
docker/mosquitto/certs/fullchain.pem
docker/mosquitto/certs/privkey.pem
```

Эти файлы не отправляются в Git.



## Server deployment

Тестовое серверное окружение GetBox развёрнуто 25 июня 2026 года.

### Server information

```text
Public IP: 91.243.71.86
Project directory: /opt/getbox
Git branch: main
Compose command: docker-compose
```

На сервере используется отдельный бинарный `docker-compose`. Команда `docker compose` на текущем сервере недоступна.

### Published services

| Service | Address | Purpose |
|---|---|---|
| FastAPI | `http://91.243.71.86:8010` | HTTP API GetBox |
| Mosquitto | `91.243.71.86:8883` | Публичный MQTT через TLS |
| Internal MQTT | `mqtt:1883` | Связь FastAPI с Mosquitto внутри Docker |

Порт `1883` не опубликован на сервере. ESP32 подключается к публичному IP на порт `8883`.

### Current controller configuration

```text
box_id: box_001
controller_id: ctrl_01
MQTT device username: ctrl_01
MQTT API username: getbox_api
```

Пользователь `ctrl_01` имеет право:

- читать команды только из `getbox/box_001/cell/ctrl_01/cmd`;
- публиковать статусы в `getbox/box_001/cell/ctrl_01/status`;
- публиковать события в `getbox/box_001/cell/ctrl_01/event`;
- публиковать heartbeat в `getbox/box_001/device/ctrl_01/heartbeat`;
- публиковать ошибки в `getbox/box_001/device/ctrl_01/error`.

Контроллер не может публиковать сообщения в собственный командный topic. Это ограничение проверено через MQTT ACL.

### Server files

```text
/opt/getbox/.env
/opt/getbox/secrets/mqtt_api_password.txt
/opt/getbox/secrets/mqtt_device_password.txt
/opt/getbox/docker/mosquitto/certs/ca.crt
/opt/getbox/docker/mosquitto/certs/fullchain.pem
/opt/getbox/docker/mosquitto/certs/privkey.pem
```

Пароли, закрытые ключи, сертификаты сервера и серверный `.env` не хранятся в Git.

### PKI files

Исходные файлы собственного центра сертификации находятся только на сервере:

```text
/root/getbox-pki/ca.key
/root/getbox-pki/ca.crt
/root/getbox-pki/server.key
/root/getbox-pki/server