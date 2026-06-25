from fastapi import APIRouter, HTTPException, status


from app.schemas.cells import (
    CommandQueuedResponse,
    CommandStatusResponse,
    OpenCellRequest,
)
from app.services.command_store import command_store
from app.services.mqtt_service import mqtt_service
from app.core.config import settings

router = APIRouter(prefix="/cells", tags=["cells"])


@router.post(
    "/open",
    response_model=CommandQueuedResponse,
    status_code=status.HTTP_202_ACCEPTED,
)
def open_cell(
    payload: OpenCellRequest,
) -> CommandQueuedResponse:
    
    if payload.box_id != settings.box_id:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="Unknown Box",
        )

    if payload.controller_id != settings.controller_id:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="Unknown controller",
        )

    if not 1 <= payload.cell_id <= settings.cell_count:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="Unknown cell",
        )
    
    if not mqtt_service.is_connected():
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="MQTT broker is not connected",
        )

    record = command_store.create_open_cell(
        box_id=payload.box_id,
        controller_id=payload.controller_id,
        cell_id=payload.cell_id,
        duration_ms=payload.duration_ms,
    )

    if not mqtt_service.publish_open_cell(record):
        command_store.update_status(
            record.request_id,
            "error",
            error_code="mqtt_publish_failed",
            message="Failed to publish MQTT command",
        )

        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="Failed to publish MQTT command",
        )

    return CommandQueuedResponse(
        request_id=record.request_id,
        box_id=record.box_id,
        controller_id=record.controller_id,
        cell_id=record.cell_id,
        duration_ms=record.duration_ms,
    )


@router.get(
    "/commands/{request_id}",
    response_model=CommandStatusResponse,
)
def get_command_status(
    request_id: str,
) -> CommandStatusResponse:
    record = command_store.get(request_id)

    if record is None:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Command not found",
        )

    return CommandStatusResponse(**record.snapshot())