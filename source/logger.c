#include "logger.h"


int write_log(struct logger *logger);


struct logger *init_logger() {
    struct logger *logger = malloc(sizeof(struct logger));
    logger->file_descriptor = open(events_log, O_CREAT | O_RDWR | O_TRUNC, 0777);;
    return logger;
}

int do_log_started_fmt(struct logger *logger, timestamp_t timestamp, local_id id, balance_t balance) {
    logger->log_length = sprintf(logger->log_buffer, log_started_fmt,
                                 timestamp,
                                 id,
                                 getpid(), getppid(),
                                 balance);

    return write_log(logger);
}

int do_log_received_all_started_fmt(struct logger *logger, timestamp_t timestamp, local_id id) {
    logger->log_length = sprintf(logger->log_buffer, log_received_all_started_fmt,
                                 timestamp,
                                 id);

    return write_log(logger);
}

int do_log_done_fmt(struct logger *logger, timestamp_t timestamp, local_id id, balance_t balance) {
    logger->log_length = sprintf(logger->log_buffer, log_done_fmt,
                                 timestamp,
                                 id,
                                 balance);

    return write_log(logger);
}

int do_log_transfer_out_fmt(struct logger *logger, timestamp_t timestamp, local_id id, local_id target_id,
                            balance_t balance) {
    logger->log_length = sprintf(logger->log_buffer, log_transfer_out_fmt,
                                 timestamp,
                                 id,
                                 balance,
                                 target_id);

    return write_log(logger);
}

int do_log_transfer_in_fmt(struct logger *logger, timestamp_t timestamp, local_id id, local_id target_id,
                           balance_t balance) {
    logger->log_length = sprintf(logger->log_buffer, log_transfer_in_fmt,
                                 timestamp,
                                 id,
                                 balance,
                                 target_id);

    return write_log(logger);
}

int do_log_received_all_done_fmt(struct logger *logger, timestamp_t timestamp, local_id id) {
    logger->log_length = sprintf(logger->log_buffer, log_received_all_done_fmt,
                                 timestamp,
                                 id);

    return write_log(logger);
}

void close_logger(struct logger *logger) {
    if (NULL == logger)
        return;

    close(logger->file_descriptor);
    free(logger);
}

int write_log(struct logger *logger) {
    return write(logger->file_descriptor, logger->log_buffer, logger->log_length);
}


char *get_msg_type_name(struct logger *logger, MessageType type) {
    char *buffer = logger->log_buffer;

    switch (type) {
        case STARTED:
            sprintf(buffer, "STARTED");
            break;
        case DONE:
            sprintf(buffer, "DONE");
            break;
        case ACK:
            sprintf(buffer, "ACK");
            break;
        case STOP:
            sprintf(buffer, "STOP");
            break;
        case TRANSFER:
            sprintf(buffer, "TRANSFER");
            break;
        case BALANCE_HISTORY:
            sprintf(buffer, "BALANCE_HISTORY");
            break;
        case CS_REQUEST:
            sprintf(buffer, "CS_REQUEST");
            break;
        case CS_REPLY:
            sprintf(buffer, "CS_REPLY");
            break;
        case CS_RELEASE:
            sprintf(buffer, "CS_RELEASE");
            break;
    }
    return buffer;
}
