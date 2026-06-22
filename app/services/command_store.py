from dataclasses import dataclass
from datetime import datetime, timezone
from threading import Lock
from typing import Dict, Optional
from uuid import uuid4


@dataclass
class CommandRecord:
    request_id: str
    box_id: str
    controller_id: str
    cell_id: int
    action: str
    status: str
    duration_ms: int
    created_at: datetime


class CommandStore:
    def __init__(self) -> None:
        self._commands: Dict[str, CommandRecord] = {}
        self._lock = Lock()

    def create_open_cell(self, box_id: str, controller_id: str, cell_id: int, duration_ms: int) -> CommandRecord:
        record = CommandRecord(
            request_id=f"req_{uuid4().hex}",
            box_id=box_id,
            controller_id=controller_id,
            cell_id=cell_id,
            action="open_cell",
            status="accepted",
            duration_ms=duration_ms,
            created_at=datetime.now(timezone.utc),
        )
        with self._lock:
            self._commands[record.request_id] = record
        return record

    def get(self, request_id: str) -> Optional[CommandRecord]:
        with self._lock:
            return self._commands.get(request_id)


command_store = CommandStore()
