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
