# Этап 3. JSON Commands and Statuses

## Назначение

Данный документ описывает формат JSON-сообщений, используемых для обмена данными между FastAPI и ESP32 через MQTT.

Протокол является прикладным уровнем системы GetBox и определяет:

- какие команды может отправлять сервер;
- какие статусы возвращают ESP32;
- какие события могут возникать;
- как происходит мониторинг устройств;
- каким образом отслеживается состояние системы.

---

# Общие правила

Все сообщения должны быть представлены в формате JSON.

Основные требования:

- UTF-8;
- имена полей в `snake_case`;
- обязательное наличие идентификаторов;
- отсутствие привязки к конкретным GPIO;
- расширяемость без нарушения обратной совместимости.

---

# Обязательные идентификаторы

| Поле | Описание |
|---|---|
| `request_id` | Уникальный идентификатор команды |
| `box_id` | Логический идентификатор Box |
| `controller_id` | Идентификатор ESP32 |
| `cell_id` | Номер ячейки внутри Box |

---

# Команда открытия ячейки

## Topic

```text
getbox/{box_id}/cell/{controller_id}/cmd
```

## JSON

```json
{
    "request_id": "req_12345",
    "action": "open_cell",
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "cell_id": 6,
    "duration_ms": 1000
}
```

## Описание

| Поле | Тип | Обязательное | Описание |
|---|---|---|---|
| request_id | string | Да | Идентификатор команды |
| action | string | Да | Тип действия |
| box_id | string | Да | Идентификатор Box |
| controller_id | string | Да | ESP32-получатель |
| cell_id | integer | Да | Номер ячейки |
| duration_ms | integer | Да | Время активации реле |

---

# Статус: команда принята

## Topic

```text
getbox/{box_id}/cell/{controller_id}/status
```

## JSON

```json
{
    "request_id": "req_12345",
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "cell_id": 6,
    "status": "accepted"
}
```

---

# Статус: реле активировано

## JSON

```json
{
    "request_id": "req_12345",
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "cell_id": 6,
    "status": "relay_activated",
    "duration_ms": 1000
}
```

---

# Статус: дверь открыта

## JSON

```json
{
    "request_id": "req_12345",
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "cell_id": 6,
    "status": "door_opened"
}
```

---

# Статус: дверь закрыта

## JSON

```json
{
    "request_id": "req_12345",
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "cell_id": 6,
    "status": "door_closed"
}
```

---

# Ошибка: дверь не открылась

## Topic

```text
getbox/{box_id}/device/{controller_id}/error
```

## JSON

```json
{
    "request_id": "req_12345",
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "cell_id": 6,
    "status": "error",
    "error_code": "door_not_opened_timeout"
}
```

---

# Ошибка: неизвестная ячейка

## JSON

```json
{
    "request_id": "req_12345",
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "status": "error",
    "error_code": "unknown_cell"
}
```

---

# Ошибка: неизвестная команда

## JSON

```json
{
    "request_id": "req_12345",
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "status": "error",
    "error_code": "unsupported_action"
}
```

---

# Событие: возможное вскрытие

Геркон обнаружил открытие двери без предварительной команды.

## Topic

```text
getbox/{box_id}/cell/{controller_id}/event
```

## JSON

```json
{
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "cell_id": 6,
    "event": "forced_open",
    "door_state": "open"
}
```

---

# Событие: дверь открыта штатно

## JSON

```json
{
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "cell_id": 6,
    "event": "normal_open"
}
```

---

# Событие: дверь закрыта

## JSON

```json
{
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "cell_id": 6,
    "event": "door_closed"
}
```

---

# Статус датчиков DHT11

## Topic

```text
getbox/{box_id}/sensors/{controller_id}/status
```

## JSON

```json
{
    "box_id": "box_003",
    "controller_id": "sens_01",
    "type": "sensors",
    "dht": [
        {
            "sensor_id": "dht_01",
            "temperature": 24.5,
            "humidity": 41
        },
        {
            "sensor_id": "dht_02",
            "temperature": 25.1,
            "humidity": 44
        },
        {
            "sensor_id": "dht_03",
            "temperature": 24.8,
            "humidity": 42
        }
    ]
}
```

---

# Heartbeat устройства

Используется для контроля доступности ESP32.

## Topic

```text
getbox/{box_id}/device/{controller_id}/heartbeat
```

## JSON

```json
{
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "device_type": "cell_controller",
    "status": "online",
    "ip": "192.168.1.34",
    "uptime_sec": 5820,
    "free_heap": 178432,
    "firmware_version": "1.0.0"
}
```

---

# Ошибки устройства

## Topic

```text
getbox/{box_id}/device/{controller_id}/error
```

## JSON

```json
{
    "box_id": "box_003",
    "controller_id": "ctrl_02",
    "error_code": "relay_failure",
    "message": "Relay activation failed"
}
```

---

# Рекомендуемые коды ошибок

| Код | Описание |
|---|---|
| unknown_cell | Неизвестная ячейка |
| unsupported_action | Неизвестная команда |
| door_not_opened_timeout | Дверь не открылась после команды |
| relay_failure | Ошибка активации реле |
| reed_switch_failure | Неисправность геркона |
| sensor_read_failed | Ошибка чтения DHT11 |
| mqtt_connection_lost | Потеря MQTT соединения |
| wifi_connection_lost | Потеря Wi-Fi |
| invalid_payload | Некорректный JSON |

---

# Жизненный цикл открытия ячейки

```text
1. FastAPI получает HTTP-запрос.

2. FastAPI определяет:
   Box → Controller → Cell.

3. FastAPI публикует MQTT-команду open_cell.

4. ESP32 возвращает:
   status = accepted.

5. ESP32 активирует реле.

6. ESP32 возвращает:
   status = relay_activated.

7. Геркон фиксирует открытие.

8. ESP32 публикует:
   event = normal_open.

9. Геркон фиксирует закрытие.

10. ESP32 публикует:
    event = door_closed.
```

---

# Сценарий несанкционированного открытия

```text
1. Команда открытия отсутствовала.

2. Геркон обнаруживает изменение состояния.

3. ESP32 определяет событие.

4. Публикуется:

   event = forced_open.
```

---

# Итог

Протокол JSON GetBox обеспечивает:

- единый формат обмена данными;
- отслеживание выполнения команд;
- контроль состояния ячеек;
- обнаружение вскрытия;
- мониторинг температуры и влажности;
- диагностику устройств;
- возможность расширения без изменения существующих клиентов.