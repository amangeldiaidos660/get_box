FROM eclipse-mosquitto:2

COPY mosquitto.conf /mosquitto/config/mosquitto.conf
COPY entrypoint.sh /usr/local/bin/getbox-mosquitto-entrypoint.sh

RUN sed -i 's/\r$//' /usr/local/bin/getbox-mosquitto-entrypoint.sh \
    && chmod +x /usr/local/bin/getbox-mosquitto-entrypoint.sh

ENTRYPOINT ["/usr/local/bin/getbox-mosquitto-entrypoint.sh"]