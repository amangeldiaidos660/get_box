# Основной принцип работы MQTT

Mosquitto MQTT Broker не содержит бизнес-логики.

Он:

- не ищет устройства;
- не определяет принадлежность ячеек;
- не знает структуру Box;
- не принимает решений.

Mosquitto выполняет только две функции:

- принимает сообщения (Publish);
- доставляет сообщения подписчикам (Subscribe).

Главное правило:

```text
Кто подписан на Topic,
тот и получает сообщение.
```

## Правило корреляции команд

- FastAPI создает `request_id`.
- ESP32 возвращает тот же `request_id` во всех сообщениях по одной команде.
- FastAPI использует `request_id` для отслеживания жизненного цикла команды.
- Все сообщения, относящиеся к одной команде, публикуются в пределах одного `cell`-контроллера и одного `box_id`.

---

# Общая структура MQTT Topics

Формат:

```text
getbox/{box_id}/{controller_type}/{controller_id}/{channel}
```

где:

| Параметр | Описание |
|---|---|
| `box_id` | Идентификатор Box |
| `controller_type` | Тип контроллера |
| `controller_id` | Уникальный идентификатор ESP32 |
| `channel` | Назначение канала |

---

# Типы контроллеров

| Controller Type | Назначение |
|---|---|
| `cell` | Контроллер ячеек |
| `sensors` | Контроллер датчиков |
| `device` | Общие системные сообщения устройства |

---

# Каналы (Channels)

| Channel | Назначение |
|---|---|
| `cmd` | Команды устройству |
| `status` | Текущее состояние |
| `event` | События |
| `heartbeat` | Проверка доступности |
| `error` | Ошибки устройства |

## Семантика каналов для GetBox

- `cmd` используется FastAPI для отправки управляющей команды.
- `status` используется ESP32 для ответа на команду и сообщения о переходах состояния.
- `event` используется ESP32 для событий двери, включая штатное открытие и возможное вскрытие.
- `error` используется ESP32 для ошибок устройства и ошибок сценария.
- `heartbeat` используется ESP32 для периодического сигнала доступности.

---

# Примеры MQTT Topics

## Команда контроллеру ячеек

```text
getbox/box_003/cell/ctrl_02/cmd
```

---

## Статус контроллера ячеек

```text
getbox/box_003/cell/ctrl_02/status
```

---

## События контроллера ячеек

```text
getbox/box_003/cell/ctrl_02/event
```

---

## Статус датчиков

```text
getbox/box_003/sensors/sens_01/status
```

---

## Heartbeat устройства

```text
getbox/box_003/device/ctrl_02/heartbeat
```

---

## Ошибки устройства

```text
getbox/box_003/device/ctrl_02/error
```

---

# Таблица MQTT Topics

| Topic | Кто публикует | Кто подписывается | Назначение |
|---|---|---|---|
| `getbox/{box_id}/cell/{controller_id}/cmd` | FastAPI | ESP32 Cell Controller | Команды управления ячейками |
| `getbox/{box_id}/cell/{controller_id}/status` | ESP32 Cell Controller | FastAPI | Ответы по командам и переходы состояний по `request_id` |
| `getbox/{box_id}/cell/{controller_id}/event` | ESP32 Cell Controller | FastAPI | События открытия, закрытия и возможного вскрытия |
| `getbox/{box_id}/sensors/{controller_id}/status` | ESP32 Sensors Controller | FastAPI | Данные DHT11 |
| `getbox/{box_id}/device/{controller_id}/heartbeat` | ESP32 | FastAPI | Информация о доступности устройства |
| `getbox/{box_id}/device/{controller_id}/error` | ESP32 | FastAPI | Ошибки работы устройства и сценария |

## Обязательные поля командного контура

Для всех сообщений, связанных с выполнением команды открытия ячейки, обязательны:

- `request_id`
- `box_id`
- `controller_id`
- `cell_id`

Для `cmd` дополнительно требуется:

- `action`
- `duration_ms`

Для `status` дополнительно требуется:

- `status`
- `message` или `error_code`, если это ошибка

Для `event` дополнительно требуется:

- `event`
- `door_state`, если событие связано с положением двери

---

# Подписки ESP32

Каждая ESP32 подписывается только на собственные команды.

Например:

ESP32:

```text
Controller ID: ctrl_02
Box ID: box_003
```

Подписка:

```text
getbox/box_003/cell/ctrl_02/cmd
```

Таким образом устройство получает только те команды, которые относятся непосредственно к нему.

ESP32 Cell Controller также публикует в:

```text
getbox/box_003/cell/ctrl_02/status
getbox/box_003/cell/ctrl_02/event
getbox/box_003/device/ctrl_02/heartbeat
getbox/box_003/device/ctrl_02/error
```

---

# Подписки FastAPI

FastAPI получает информацию обо всех устройствах системы.

Подписки:

```text
getbox/+/+/+/status
getbox/+/+/+/event
getbox/+/device/+/heartbeat
getbox/+/device/+/error
```

FastAPI использует `request_id` для связывания входящих `status`, `event` и `error` сообщений с конкретной командой.

---

# Использование MQTT Wildcards

## Single-level wildcard

Символ:

```text
+
```

означает один произвольный уровень.

Пример:

```text
getbox/+/cell/+/status
```

Будут получены сообщения:

```text
getbox/box_001/cell/ctrl_01/status
getbox/box_002/cell/ctrl_05/status
getbox/box_003/cell/ctrl_02/status
```

---

## Multi-level wildcard

Символ:

```text
#
```

означает все последующие уровни.

Пример:

```text
getbox/#
```

Подписка получит абсолютно все сообщения системы GetBox.

Использование рекомендуется только для отладки.

---

# Маршрутизация команд

Пример запроса клиента:

```text
Открыть ячейку №6 в Box_003
```

FastAPI выполняет следующие действия:

```text
1. Получает HTTP-запрос.

2. Определяет:
   Box = box_003

3. Находит:
   Cell 6 → Controller ctrl_02

4. Формирует MQTT Topic:

   getbox/box_003/cell/ctrl_02/cmd

5. Публикует команду.

6. ESP32 ctrl_02 получает сообщение.

7. Выполняет действие.

8. Отправляет статус обратно с тем же `request_id`.

9. Переходит к следующему статусу или ошибке в рамках того же `request_id`.
```

---

# !? Что НЕ делает Mosquitto

Mosquitto НЕ:

- хранит карту Box;
- знает расположение ячеек;
- определяет нужную ESP32;
- выполняет бизнес-логику;
- принимает решения.

Все перечисленные функции относятся к FastAPI.

---

