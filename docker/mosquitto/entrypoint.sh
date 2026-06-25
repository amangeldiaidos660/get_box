#!/bin/sh
set -eu

API_PASSWORD_FILE=/run/secrets/mqtt_api_password
DEVICE_PASSWORD_FILE=/run/secrets/mqtt_device_password

PASSWORD_FILE=/mosquitto/data/passwords
ACL_FILE=/mosquitto/data/acl

for variable_name in \
    MQTT_API_USERNAME \
    MQTT_DEVICE_USERNAME \
    MQTT_BOX_ID \
    MQTT_CONTROLLER_ID
do
    eval "variable_value=\${$variable_name:-}"

    if [ -z "$variable_value" ]; then
        echo "Required environment variable is empty: $variable_name" >&2
        exit 1
    fi
done

for required_file in \
    "$API_PASSWORD_FILE" \
    "$DEVICE_PASSWORD_FILE" \
    /mosquitto/certs/fullchain.pem \
    /mosquitto/certs/privkey.pem
do
    if [ ! -s "$required_file" ]; then
        echo "Required file is missing or empty: $required_file" >&2
        exit 1
    fi
done

API_PASSWORD=$(cat "$API_PASSWORD_FILE")
DEVICE_PASSWORD=$(cat "$DEVICE_PASSWORD_FILE")

mosquitto_passwd \
    -b \
    -c \
    "$PASSWORD_FILE" \
    "$MQTT_API_USERNAME" \
    "$API_PASSWORD"

mosquitto_passwd \
    -b \
    "$PASSWORD_FILE" \
    "$MQTT_DEVICE_USERNAME" \
    "$DEVICE_PASSWORD"

cat > "$ACL_FILE" <<EOF
user $MQTT_API_USERNAME
topic readwrite getbox/#

user $MQTT_DEVICE_USERNAME
topic read getbox/$MQTT_BOX_ID/cell/$MQTT_CONTROLLER_ID/cmd
topic write getbox/$MQTT_BOX_ID/cell/$MQTT_CONTROLLER_ID/status
topic write getbox/$MQTT_BOX_ID/cell/$MQTT_CONTROLLER_ID/event
topic write getbox/$MQTT_BOX_ID/device/$MQTT_CONTROLLER_ID/heartbeat
topic write getbox/$MQTT_BOX_ID/device/$MQTT_CONTROLLER_ID/error
EOF

chown mosquitto:mosquitto "$PASSWORD_FILE" "$ACL_FILE"
chmod 600 "$PASSWORD_FILE"
chmod 640 "$ACL_FILE"

unset API_PASSWORD
unset DEVICE_PASSWORD

exec /docker-entrypoint.sh \
    mosquitto \
    -c /mosquitto/config/mosquitto.conf