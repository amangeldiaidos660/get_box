from fastapi import APIRouter, HTTPException, status

from app.schemas.cells import CommandAcceptedResponse, CommandStatusResponse, OpenCellRequest
from app.services.command_store import command_store

router = APIRouter(prefix="/cells", tags=["cells"])


@router.post("/open", response_model=CommandAcceptedResponse, status_code=status.HTTP_202_ACCEPTED)
def open_cell(payload: OpenCellRequest) -> CommandAcceptedResponse:
    record = command_store.create_open_cell(
        box_id=payload.box_id,
        controller_id=payload.controller_id,
        cell_id=payload.cell_id,
        duration_ms=payload.duration_ms,
    )
    return CommandAcceptedResponse(
        request_id=record.request_id,
        box_id=record.box_id,
        controller_id=record.controller_id,
        cell_id=record.cell_id,
        duration_ms=record.duration_ms,
    )


@router.get("/commands/{request_id}", response_model=CommandStatusResponse)
def get_command_status(request_id: str) -> CommandStatusResponse:
    record = command_store.get(request_id)
    if record is None:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Command not found")

    return CommandStatusResponse(
        request_id=record.request_id,
        box_id=record.box_id,
        controller_id=record.controller_id,
        cell_id=record.cell_id,
        status=record.status,
        action=record.action,
        duration_ms=record.duration_ms,
    )
