import os
from pathlib import Path
from typing import Optional

from pydantic import BaseModel


def read_secret(value_name: str, file_name: str) -> Optional[str]:
    value = os.getenv(value_name)
    if value:
        return value

    secret_file = os.getenv(file_name)
    if not secret_file:
        return None

    path = Path(secret_file)
    if not path.is_file():
        return None

    return path.read_text(encoding="utf-8").strip()


def read_bool(name: str, default: bool) -> bool:
    value = os.getenv(name)
    if value is None:
        return default

    return value.lower() in {"1", "true", "yes", "on"}


class Settings(BaseModel):
    app_name: str = os.getenv("APP_NAME", "GetBox API")
    api_prefix: str = os.getenv("API_PREFIX", "/api/v1")

    box_id: str = os.getenv("GETBOX_BOX_ID", "box_001")
    controller_id: str = os.getenv(
        "GETBOX_CONTROLLER_ID",
        "ctrl_01",
    )
    cell_count: int = int(os.getenv("GETBOX_CELL_COUNT", "6"))

    command_timeout_seconds: int = int(
        os.getenv("COMMAND_TIMEOUT_SECONDS", "30")
    )
    accepted_timeout_seconds: int = int(
        os.getenv("ACCEPTED_TIMEOUT_SECONDS", "3")
    )
    relay_timeout_seconds: int = int(
        os.getenv("RELAY_TIMEOUT_SECONDS", "2")
    )
    door_open_timeout_seconds: int = int(
        os.getenv("DOOR_OPEN_TIMEOUT_SECONDS", "30")
    )

    mqtt_host: str = os.getenv("MQTT_HOST", "localhost")
    mqtt_port: int = int(os.getenv("MQTT_PORT", "1883"))
    mqtt_client_id: str = os.getenv(
        "MQTT_CLIENT_ID",
        "getbox_api_server",
    )
    mqtt_username: Optional[str] = os.getenv("MQTT_USERNAME")
    mqtt_password: Optional[str] = read_secret(
        "MQTT_PASSWORD",
        "MQTT_PASSWORD_FILE",
    )
    mqtt_tls_enabled: bool = read_bool(
        "MQTT_TLS_ENABLED",
        False,
    )


settings = Settings()