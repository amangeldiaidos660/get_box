from pydantic import BaseModel


class Settings(BaseModel):
    app_name: str = "GetBox API"
    api_prefix: str = "/api/v1"
    command_timeout_seconds: int = 30
    accepted_timeout_seconds: int = 3
    relay_timeout_seconds: int = 2
    door_open_timeout_seconds: int = 30


settings = Settings()
