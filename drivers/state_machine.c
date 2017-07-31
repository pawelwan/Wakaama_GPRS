#include "state_machine.h"
#include "queue.h"

#define TMP_SIZE 10
#define QUEUE_SIZE 200

typedef enum {
    normal_state,
    plus_state,
    M_state,
    I_state,
    P_state,
    R_state,
    udp_state,
    tcp_state
} state_t;

static void default_func(char, char, state_t);
static void R_func(char);
static void protocol_func(char, xQueueHandle);
static void transfer(xQueueHandle);

static state_t curr_state;

static xQueueHandle tmp_queue;

static xQueueHandle rx_queue;
static xQueueHandle udp_queue;
static xQueueHandle tcp_queue;

uint8_t queues_init() {
    curr_state = normal_state;

    tmp_queue = xQueueCreate(TMP_SIZE, sizeof(char));

    rx_queue = xQueueCreate(QUEUE_SIZE, sizeof(char));
    udp_queue = xQueueCreate(QUEUE_SIZE, sizeof(char));
    tcp_queue = xQueueCreate(QUEUE_SIZE, sizeof(char));

    if (tmp_queue == NULL || rx_queue == NULL ||
        udp_queue == NULL || tcp_queue == NULL) {

        return 0;
    }

    return 1;
}

BaseType_t receive_from_rx(char *key, TickType_t timeout) {
    return xQueueReceive(rx_queue, key, timeout);
}

BaseType_t receive_from_udp(char *key, TickType_t timeout) {
    return xQueueReceive(udp_queue, key, timeout);
}

BaseType_t receive_from_tcp(char *key, TickType_t timeout) {
    return xQueueReceive(tcp_queue, key, timeout);
}

void send_to_queues(char chr) {
    static char expected_chrs[] = {'+', 'M', 'I', 'P', 'R'};
    static state_t next_states[] = {plus_state, M_state, I_state, P_state, R_state};

    if (curr_state == normal_state && chr != '+') {
        xQueueSendToBackFromISR(rx_queue, &chr, NULL);
        return;
    }

    if (curr_state != tcp_state && curr_state != udp_state) {
        xQueueSendToBackFromISR(tmp_queue, &chr, NULL);
    }

    switch (curr_state) {
        case R_state: {
            R_func(chr);
            break;
        }
        case udp_state: {
            protocol_func(chr, udp_queue);
            break;
        }
        case tcp_state: {
            protocol_func(chr, tcp_queue);
            break;
        }
        default: {
            default_func(chr, expected_chrs[curr_state], next_states[curr_state]);
            break;
        }
    }
}

static void default_func(char chr, char expected_chr, state_t next_state) {
    if (chr == expected_chr) {
        curr_state = next_state;
    } else {
        transfer(rx_queue);
        curr_state = normal_state;
    }
}

static void R_func(char chr) {
    if (chr == 'U') {
        transfer(udp_queue);
        curr_state = udp_state;
    } else if (chr == 'T') {
        transfer(tcp_queue);
        curr_state = tcp_state;
    } else {
        transfer(rx_queue);
        curr_state = normal_state;
    }
}

static void protocol_func(char chr, xQueueHandle to_queue) {
    xQueueSendToBackFromISR(to_queue, &chr, NULL);

    if (chr == '\r') {
        curr_state = normal_state;
    }
}

static void transfer(xQueueHandle to_queue) {
    char chr;
    while (xQueueReceiveFromISR(tmp_queue, &chr, NULL)) {
        xQueueSendToBackFromISR(to_queue, &chr, NULL);
    }
}
