#pragma once

#include <Arduino.h>
#include <cstring>

#include "firmware_variables.h"

struct RequestCacheItem {
    bool used;
    char requestId[REQUEST_ID_MAX_LENGTH + 1];
    uint16_t cellId;
    char status[32];
    char errorCode[48];
};

inline RequestCacheItem requestCache[
    RECENT_REQUEST_COUNT
] = {};

inline size_t nextRequestCacheIndex = 0;

inline void copyRequestText(
    char* destination,
    size_t destinationSize,
    const char* source
) {
    if (destinationSize == 0) {
        return;
    }

    if (source == nullptr) {
        destination[0] = '\0';
        return;
    }

    strncpy(
        destination,
        source,
        destinationSize - 1
    );

    destination[destinationSize - 1] = '\0';
}

inline RequestCacheItem* findCachedRequest(
    const char* requestId
) {
    if (requestId == nullptr) {
        return nullptr;
    }

    for (
        size_t index = 0;
        index < RECENT_REQUEST_COUNT;
        index++
    ) {
        RequestCacheItem& item = requestCache[index];

        if (
            item.used
            && strcmp(item.requestId, requestId) == 0
        ) {
            return &item;
        }
    }

    return nullptr;
}

inline void saveRequestStatus(
    const char* requestId,
    uint16_t cellId,
    const char* status,
    const char* errorCode = nullptr
) {
    RequestCacheItem* item =
        findCachedRequest(requestId);

    if (item == nullptr) {
        item = &requestCache[nextRequestCacheIndex];

        nextRequestCacheIndex =
            (nextRequestCacheIndex + 1)
            % RECENT_REQUEST_COUNT;

        item->used = true;

        copyRequestText(
            item->requestId,
            sizeof(item->requestId),
            requestId
        );

        item->cellId = cellId;
    }

    copyRequestText(
        item->status,
        sizeof(item->status),
        status
    );

    copyRequestText(
        item->errorCode,
        sizeof(item->errorCode),
        errorCode
    );
}