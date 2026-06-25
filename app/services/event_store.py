from collections import deque
from copy import deepcopy
from datetime import datetime, timezone
from threading import Lock
from typing import Any


class EventStore:
    def __init__(self) -> None:
        self._lock = Lock()
        self._events: deque[dict[str, Any]] = deque(
            maxlen=200
        )
        self._mqtt_connected = False
        self._controller: dict[str, Any] = {}
        self._controller_received_at: datetime | None = None

    def set_mqtt_connected(self, connected: bool) -> None:
        with self._lock:
            self._mqtt_connected = connected

    def add_message(
        self,
        *,
        direction: str,
        topic: str,
        payload: Any,
    ) -> None:
        event = {
            "recorded_at": datetime.now(
                timezone.utc
            ).isoformat(),
            "direction": direction,
            "topic": topic,
            "payload": deepcopy(payload),
        }

        with self._lock:
            self._events.appendleft(event)

    def update_controller(
        self,
        payload: dict[str, Any],
    ) -> None:
        with self._lock:
            self._controller = deepcopy(payload)
            self._controller_received_at = datetime.now(
                timezone.utc
            )

    def snapshot(self) -> dict[str, Any]:
        now = datetime.now(timezone.utc)

        with self._lock:
            controller = deepcopy(self._controller)
            received_at = self._controller_received_at

            last_seen_seconds = None
            online = False

            if received_at is not None:
                last_seen_seconds = int(
                    (now - received_at).total_seconds()
                )
                online = (
                    controller.get("status") == "online"
                    and last_seen_seconds <= 75
                )

            return {
                "mqtt_connected": self._mqtt_connected,
                "controller": {
                    **controller,
                    "online": online,
                    "last_seen_at": (
                        received_at.isoformat()
                        if received_at
                        else None
                    ),
                    "last_seen_seconds": last_seen_seconds,
                },
                "events": list(self._events),
            }


event_store = EventStore()