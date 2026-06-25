from pathlib import Path

from fastapi import APIRouter, Request
from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates

from app.core.config import settings
from app.services.command_store import command_store
from app.services.event_store import event_store

router = APIRouter(tags=["panel"])

templates = Jinja2Templates(
    directory=str(
        Path(__file__).resolve().parents[2]
        / "templates"
    )
)


@router.get(
    "/panel",
    response_class=HTMLResponse,
)
def panel(request: Request) -> HTMLResponse:
    return templates.TemplateResponse(
        request=request,
        name="panel.html",
        context={
            "box_id": settings.box_id,
            "controller_id": settings.controller_id,
            "cell_ids": range(
                1,
                settings.cell_count + 1,
            ),
        },
    )


@router.get("/api/v1/panel/state")
def panel_state() -> dict[str, object]:
    state = event_store.snapshot()
    state["commands"] = command_store.list_recent(
        limit=30
    )
    state["box_id"] = settings.box_id
    state["controller_id"] = settings.controller_id
    state["cell_count"] = settings.cell_count

    return state