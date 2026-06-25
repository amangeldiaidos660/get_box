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