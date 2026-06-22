from typing import Literal, Optional

from pydantic import Field

from app.schemas.common import CellTargetModel, CommandIdentityModel


class OpenCellRequest(CellTargetModel):
    action: Literal["open_cell"] = "open_cell"
    duration_ms: int = Field(default=1000, ge=100, description="Relay activation time in milliseconds")


class CommandStatusResponse(CommandIdentityModel):
    status: Literal["accepted", "queued", "relay_activated", "door_opened", "door_closed", "error"]
    action: str = "open_cell"
    duration_ms: Optional[int] = None
    error_code: Optional[str] = None
    message: Optional[str] = None


class CommandAcceptedResponse(CommandIdentityModel):
    status: Literal["accepted"] = "accepted"
    action: Literal["open_cell"] = "open_cell"
    duration_ms: int = Field(default=1000, ge=100)
