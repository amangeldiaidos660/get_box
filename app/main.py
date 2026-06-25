from contextlib import asynccontextmanager

from fastapi import FastAPI

from app.api.router import api_router
from app.api.routes.panel import router as panel_router
from app.core.config import settings
from app.services.mqtt_service import mqtt_service


@asynccontextmanager
async def lifespan(app: FastAPI):
    mqtt_service.start()
    yield
    mqtt_service.stop()


app = FastAPI(
    title=settings.app_name,
    lifespan=lifespan,
)

app.include_router(
    api_router,
    prefix=settings.api_prefix,
)
app.include_router(panel_router)


@app.get("/")
def root() -> dict[str, str]:
    return {
        "service": settings.app_name,
        "docs": "/docs",
        "panel": "/panel",
    }
