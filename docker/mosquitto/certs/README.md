# Mosquitto TLS certificates

Перед запуском здесь должны находиться:

- `fullchain.pem` — сертификат MQTT-сервера;
- `privkey.pem` — закрытый ключ MQTT-сервера.

Домен не обязателен. Сертификат может быть выпущен для публичного IP-адреса сервера.

IP должен присутствовать в Subject Alternative Name:

```text
IP:123.123.123.123
```

ESP32 должна хранить корневой CA-сертификат, которым подписан сертификат MQTT-сервера.

Закрытый ключ и сертификаты нельзя отправлять в Git.


## Current server certificate

Текущее тестовое окружение использует публичный IP без домена:

```text
Server IP: 91.243.71.86
MQTT TLS port: 8883
Certificate issuer: GetBox Private CA
Certificate SAN: IP:91.243.71.86
```

Сертификат был создан 25 июня 2026 года и действует до 27 сентября 2028 года.

## Certificate locations on server

Исходные PKI-файлы:

```text
/root/getbox-pki/ca.key
/root/getbox-pki/ca.crt
/root/getbox-pki/server.key
/root/getbox-pki/server.crt
/root/getbox-pki/server.csr
/root/getbox-pki/server.cnf
```

Файлы, подключаемые к Mosquitto:

```text
/opt/getbox/docker/mosquitto/certs/ca.crt
/opt/getbox/docker/mosquitto/certs/fullchain.pem
/opt/getbox/docker/mosquitto/certs/privkey.pem
```

Назначение файлов:

- `ca.crt` — публичный сертификат GetBox CA, позже добавляется в прошивку ESP32;
- `fullchain.pem` — сертификат MQTT-сервера вместе с сертификатом CA;
- `privkey.pem` — закрытый ключ MQTT-сервера;
- `/root/getbox-pki/ca.key` — закрытый ключ центра сертификации, который должен оставаться только на сервере.

## Certificate verification

Посмотреть данные сертификата:

```bash
openssl x509 \
  -in /root/getbox-pki/server.crt \
  -noout \
  -subject \
  -issuer \
  -dates \
  -ext subjectAltName
```

Проверить публичный MQTT TLS:

```bash
openssl s_client \
  -connect 91.243.71.86:8883 \
  -CAfile /root/getbox-pki/ca.crt \
  -verify_ip 91.243.71.86 \
  </dev/null
```

Успешный результат:

```text
Verify return code: 0 (ok)
```

## Important security rules

Никогда не отправлять в Git:

```text
ca.key
server.key
privkey.pem
mqtt_api_password.txt
mqtt_device_password.txt
```

В прошивку ESP32 добавляется только публичный файл:

```text
ca.crt
```

Закрытые ключи `ca.key`, `server.key` и `privkey.pem` в прошивку не добавляются.