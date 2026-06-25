import json
from typing import Any

import paho.mqtt.client as mqtt

from app.core.config import settings
from app.services.command_store import (
    CommandRecord,
    command_store,
)
from app.services.event_store import event_store


class MqttService:
    def __init__(self) -> None:
        self._client = mqtt.Client(
            callback_api_version=(
                mqtt.CallbackAPIVersion.VERSION2
            ),
            client_id=settings.mqtt_client_id,
            protocol=mqtt.MQTTv311,
        )

        if settings.mqtt_username:
            self._client.username_pw_set(
                settings.mqtt_username,
                settings.mqtt_password,
            )

        if settings.mqtt_tls_enabled:
            self._client.tls_set()

        self._client.reconnect_delay_set(
            min_delay=1,
            max_delay=30,
        )

        self._client.on_connect = self._on_connect
        self._client.on_disconnect = self._on_disconnect
        self._client.on_message = self._on_message

    def start(self) -> None:
        self._client.connect_async(
            settings.mqtt_host,
            settings.mqtt_port,
            keepalive=30,
        )
        self._client.loop_start()

    def stop(self) -> None:
        self._client.disconnect()
        self._client.loop_stop()
        event_store.set_mqtt_connected(False)

    def is_connected(self) -> bool:
        return self._client.is_connected()

    def publish_open_cell(
        self,
        record: CommandRecord,
    ) -> bool:
        if not self._client.is_connected():
            return False

        topic = (
            f"getbox/{record.box_id}/cell/"
            f"{record.controller_id}/cmd"
        )

        payload = {
            "request_id": record.request_id,
            "action": "open_cell",
            "box_id": record.box_id,
            "controller_id": record.controller_id,
            "cell_id": record.cell_id,
            "duration_ms": record.duration_ms,
        }

        payload_json = json.dumps(
            payload,
            ensure_ascii=False,
            separators=(",", ":"),
        )

        result = self._client.publish(
            topic,
            payload_json,
            qos=1,
            retain=False,
        )

        event_store.add_message(
            direction="out",
            topic=topic,
            payload=payload,
        )

        return result.rc == mqtt.MQTT_ERR_SUCCESS

    def _on_connect(
        self,
        client: mqtt.Client,
        userdata: Any,
        flags: mqtt.ConnectFlags,
        reason_code: mqtt.ReasonCode,
        properties: mqtt.Properties | None,
    ) -> None:
        if reason_code.is_failure:
            event_store.set_mqtt_connected(False)
            return

        event_store.set_mqtt_connected(True)

        client.subscribe(
            [
                ("getbox/+/cell/+/status", 1),
                ("getbox/+/cell/+/event", 1),
                ("getbox/+/device/+/heartbeat", 1),
                ("getbox/+/device/+/error", 1),
            ]
        )

    def _on_disconnect(
        self,
        client: mqtt.Client,
        userdata: Any,
        disconnect_flags: mqtt.DisconnectFlags,
        reason_code: mqtt.ReasonCode,
        properties: mqtt.Properties | None,
    ) -> None:
        event_store.set_mqtt_connected(False)

    def _on_message(
        self,
        client: mqtt.Client,
        userdata: Any,
        message: mqtt.MQTTMessage,
    ) -> None:
        raw_payload = message.payload.decode(
            "utf-8",
            errors="replace",
        )

        try:
            payload: Any = json.loads(raw_payload)
        except json.JSONDecodeError:
            payload = {"raw": raw_payload}

        event_store.add_message(
            direction="in",
            topic=message.topic,
            payload=payload,
        )

        if not isinstance(payload, dict):
            return

        if message.topic.endswith("/heartbeat"):
            event_store.update_controller(payload)
            return

        request_id = payload.get("request_id")

        if message.topic.endswith("/status"):
            if not request_id:
                return

            status = payload.get("status")
            if not status:
                return

            command_store.update_status(
                request_id,
                status,
                message=payload.get("message"),
                error_code=payload.get("error_code"),
                door_state=payload.get("door_state"),
                duration_ms=payload.get("duration_ms"),
            )
            return

        if message.topic.endswith("/error"):
            if request_id:
                command_store.update_status(
                    request_id,
                    "error",
                    message=payload.get("message"),
                    error_code=payload.get(
                        "error_code",
                        "device_error",
                    ),
                )


mqtt_service = MqttService()