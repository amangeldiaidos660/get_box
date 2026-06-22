from datetime import datetime
from typing import Literal, Optional

from pydantic import BaseModel, Field

from app.schemas.common import CellTargetModel, CommandIdentityModel


class CommandHistoryItem(BaseModel):
    status: Literal["queued", "accepted", "relay_activated", "door_opened", "door_closed", "error"]
    recorded_at: datetime
    message: Optional[str] = None
    error_code: Optional[str] = None
    door_state: Optional[Literal["open", "closed"]] = None
    duration_ms: Optional[int] = None


class OpenCellRequest(CellTargetModel):
    action: Literal["open_cell"] = "open_cell"
    duration_ms: int = Field(default=1000, ge=100, description="Relay activation time in milliseconds")


class CommandStatusUpdateRequest(BaseModel):
    status: Literal["accepted", "relay_activated", "door_opened", "door_closed", "error"]
    message: Optional[str] = None
    error_code: Optional[str] = None
    door_state: Optional[Literal["open", "closed"]] = None
    duration_ms: Optional[int] = Field(default=None, ge=100)


class CommandStatusResponse(CommandIdentityModel):
    status: Literal["accepted", "queued", "relay_activated", "door_opened", "door_closed", "error"]
    action: str = "open_cell"
    duration_ms: Optional[int] = None
    error_code: Optional[str] = None
    message: Optional[str] = None
    door_state: Optional[Literal["open", "closed"]] = None
    created_at: Optional[datetime] = None
    updated_at: Optional[datetime] = None
    accepted_at: Optional[datetime] = None
    relay_activated_at: Optional[datetime] = None
    door_opened_at: Optional[datetime] = None
    door_closed_at: Optional[datetime] = None
    accepted_deadline_at: Optional[datetime] = None
    relay_deadline_at: Optional[datetime] = None
    door_close_deadline_at: Optional[datetime] = None
    history: list[CommandHistoryItem] = Field(default_factory=list)


class CommandAcceptedResponse(CommandIdentityModel):
    status: Literal["accepted"] = "accepted"
    action: Literal["open_cell"] = "open_cell"
    duration_ms: int = Field(default=1000, ge=100)
