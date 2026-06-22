# Этап 2. MQTT Protocol и Topics

## Назначение

Данный документ описывает правила взаимодействия компонентов системы GetBox через MQTT.

MQTT используется исключительно как транспортный уровень обмена сообщениями между серверной частью и устройствами ESP32.

---

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

---

# Распределение ответственности

```text
FastAPI
↓
знает структуру системы

Mosquitto
↓
доставляет сообщения

ESP32
↓
выполняют команды
```

---

# Общая структура MQTT Topics

Формат:

```text
getbox/{box_id}/{controller_type}/{controller_id}/{channel}
```

где:

| Параметр | Описание |
|---|---|
| `box_id` | Логический идентификатор Box |
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
| `getbox/{box_id}/cell/{controller_id}/status` | ESP32 Cell Controller | FastAPI | Ответы по командам и текущее состояние |
| `getbox/{box_id}/cell/{controller_id}/event` | ESP32 Cell Controller | FastAPI | События открытия, закрытия и взлома |
| `getbox/{box_id}/sensors/{controller_id}/status` | ESP32 Sensors Controller | FastAPI | Данные DHT11 |
| `getbox/{box_id}/device/{controller_id}/heartbeat` | ESP32 | FastAPI | Информация о доступности устройства |
| `getbox/{box_id}/device/{controller_id}/error` | ESP32 | FastAPI | Ошибки работы устройства |

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

8. Отправляет статус обратно.
```

---

# Что НЕ делает Mosquitto

Mosquitto НЕ:

- хранит карту Box;
- знает расположение ячеек;
- определяет нужную ESP32;
- выполняет бизнес-логику;
- принимает решения.

Все перечисленные функции относятся к FastAPI.

---

# Итог

MQTT в архитектуре GetBox используется как высокоскоростной транспортный слой обмена сообщениями.

При этом:

- FastAPI отвечает за принятие решений;
- Mosquitto отвечает за доставку сообщений;
- ESP32 отвечают за выполнение команд;
- Box остается логической сущностью, объединяющей группу ESP32.