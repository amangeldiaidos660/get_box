from dataclasses import dataclass
from datetime import datetime, timedelta, timezone
from threading import Lock
from typing import Dict, Optional
from uuid import uuid4

from app.core.config import settings


@dataclass
class LifecycleEventRecord:
    status: str
    recorded_at: datetime
    message: Optional[str] = None
    error_code: Optional[str] = None
    door_state: Optional[str] = None
    duration_ms: Optional[int] = None


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
    updated_at: datetime
    accepted_at: Optional[datetime] = None
    relay_activated_at: Optional[datetime] = None
    door_opened_at: Optional[datetime] = None
    door_closed_at: Optional[datetime] = None
    accepted_deadline_at: Optional[datetime] = None
    relay_deadline_at: Optional[datetime] = None
    door_close_deadline_at: Optional[datetime] = None
    door_state: Optional[str] = None
    error_code: Optional[str] = None
    message: Optional[str] = None
    history: list[LifecycleEventRecord] = None

    def __post_init__(self) -> None:
        if self.history is None:
            self.history = []

    def add_history(
        self,
        status: str,
        recorded_at: datetime,
        message: Optional[str] = None,
        error_code: Optional[str] = None,
        door_state: Optional[str] = None,
        duration_ms: Optional[int] = None,
    ) -> None:
        self.history.append(
            LifecycleEventRecord(
                status=status,
                recorded_at=recorded_at,
                message=message,
                error_code=error_code,
                door_state=door_state,
                duration_ms=duration_ms,
            )
        )

    def snapshot(self) -> dict[str, object]:
        return {
            "request_id": self.request_id,
            "box_id": self.box_id,
            "controller_id": self.controller_id,
            "cell_id": self.cell_id,
            "status": self.status,
            "action": self.action,
            "duration_ms": self.duration_ms,
            "error_code": self.error_code,
            "message": self.message,
            "door_state": self.door_state,
            "created_at": self.created_at,
            "updated_at": self.updated_at,
            "accepted_at": self.accepted_at,
            "relay_activated_at": self.relay_activated_at,
            "door_opened_at": self.door_opened_at,
            "door_closed_at": self.door_closed_at,
            "accepted_deadline_at": self.accepted_deadline_at,
            "relay_deadline_at": self.relay_deadline_at,
            "door_close_deadline_at": self.door_close_deadline_at,
            "history": [
                {
                    "status": item.status,
                    "recorded_at": item.recorded_at,
                    "message": item.message,
                    "error_code": item.error_code,
                    "door_state": item.door_state,
                    "duration_ms": item.duration_ms,
                }
                for item in self.history
            ],
        }


class CommandStore:
    def __init__(self) -> None:
        self._commands: Dict[str, CommandRecord] = {}
        self._lock = Lock()

    def create_open_cell(self, box_id: str, controller_id: str, cell_id: int, duration_ms: int) -> CommandRecord:
        created_at = datetime.now(timezone.utc)
        record = CommandRecord(
            request_id=f"req_{uuid4().hex}",
            box_id=box_id,
            controller_id=controller_id,
            cell_id=cell_id,
            action="open_cell",
            status="queued",
            duration_ms=duration_ms,
            created_at=created_at,
            updated_at=created_at,
            accepted_deadline_at=created_at + timedelta(seconds=settings.accepted_timeout_seconds),
        )
        record.add_history(status="queued", recorded_at=created_at, duration_ms=duration_ms)
        with self._lock:
            self._commands[record.request_id] = record
        return record

    def get(self, request_id: str) -> Optional[CommandRecord]:
        with self._lock:
            record = self._commands.get(request_id)
            if record is None:
                return None
            self._evaluate_timeouts(record)
            return record
        
    def list_recent(
        self,
        limit: int = 20,
    ) -> list[dict[str, object]]:
        with self._lock:
            records = sorted(
                self._commands.values(),
                key=lambda item: item.created_at,
                reverse=True,
            )

            for record in records:
                self._evaluate_timeouts(record)

            return [
                record.snapshot()
                for record in records[:limit]
            ]

    def update_status(
        self,
        request_id: str,
        status: str,
        *,
        message: Optional[str] = None,
        error_code: Optional[str] = None,
        door_state: Optional[str] = None,
        duration_ms: Optional[int] = None,
    ) -> Optional[CommandRecord]:
        now = datetime.now(timezone.utc)
        with self._lock:
            record = self._commands.get(request_id)
            if record is None:
                return None

            self._evaluate_timeouts(record, now=now)
            if record.status == "error" and status != "error":
                return record

            self._apply_status_transition(
                record,
                status=status,
                now=now,
                message=message,
                error_code=error_code,
                door_state=door_state,
                duration_ms=duration_ms,
            )
            return record

    def _evaluate_timeouts(self, record: CommandRecord, now: Optional[datetime] = None) -> None:
        current_time = now or datetime.now(timezone.utc)

        if record.status == "queued" and record.accepted_deadline_at and current_time > record.accepted_deadline_at:
            self._mark_error(record, current_time, error_code="accepted_timeout", message="ESP32 did not confirm accepted in time")
            return

        if record.status == "accepted" and record.relay_deadline_at and current_time > record.relay_deadline_at:
            self._mark_error(record, current_time, error_code="relay_activation_timeout", message="ESP32 did not activate relay in time")
            return

        if (
            record.status == "door_opened"
            and record.door_state == "open"
            and record.door_close_deadline_at
            and current_time > record.door_close_deadline_at
        ):
            self._mark_error(record, current_time, error_code="door_left_open", message="Door stayed open after timeout")

    def _apply_status_transition(
        self,
        record: CommandRecord,
        *,
        status: str,
        now: datetime,
        message: Optional[str],
        error_code: Optional[str],
        door_state: Optional[str],
        duration_ms: Optional[int],
    ) -> None:
        if status == "accepted":
            record.status = "accepted"
            record.accepted_at = now
            record.relay_deadline_at = now + timedelta(seconds=settings.relay_timeout_seconds)
            record.message = message
            record.error_code = None
            record.updated_at = now
            record.add_history(status="accepted", recorded_at=now, message=message, duration_ms=duration_ms)
            return

        if status == "relay_activated":
            record.status = "relay_activated"
            record.relay_activated_at = now
            record.message = message
            record.error_code = None
            record.updated_at = now
            record.add_history(status="relay_activated", recorded_at=now, message=message, duration_ms=duration_ms)
            return

        if status == "door_opened":
            record.status = "door_opened"
            record.door_opened_at = now
            record.door_state = "open"
            record.door_close_deadline_at = now + timedelta(seconds=settings.door_open_timeout_seconds)
            record.message = message
            record.error_code = None
            record.updated_at = now
            record.add_history(status="door_opened", recorded_at=now, message=message, door_state="open", duration_ms=duration_ms)
            return

        if status == "door_closed":
            record.status = "door_closed"
            record.door_closed_at = now
            record.door_state = "closed"
            record.door_close_deadline_at = None
            record.message = message
            record.error_code = None
            record.updated_at = now
            record.add_history(status="door_closed", recorded_at=now, message=message, door_state="closed", duration_ms=duration_ms)
            return

        if status == "error":
            self._mark_error(record, now, error_code=error_code or "unknown_error", message=message)

    def _mark_error(self, record: CommandRecord, now: datetime, *, error_code: str, message: Optional[str]) -> None:
        record.status = "error"
        record.error_code = error_code
        record.message = message
        record.updated_at = now
        record.add_history(status="error", recorded_at=now, message=message, error_code=error_code, door_state=record.door_state)


command_store = CommandStore()
