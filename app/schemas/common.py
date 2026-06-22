from pydantic import BaseModel, Field


class CommandIdentityModel(BaseModel):
    request_id: str = Field(..., description="Unique command identifier")
    box_id: str = Field(..., description="Logical Box identifier")
    controller_id: str = Field(..., description="ESP32 controller identifier")
    cell_id: int = Field(..., ge=1, description="Cell number inside a Box")


class CellTargetModel(BaseModel):
    box_id: str = Field(..., description="Logical Box identifier")
    controller_id: str = Field(..., description="ESP32 controller identifier")
    cell_id: int = Field(..., ge=1, description="Cell number inside a Box")
