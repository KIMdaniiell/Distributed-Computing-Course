#include "logger.h"


int write_log(struct logger *logger);


struct logger *init_logger() {
    struct logger *logger = malloc(sizeof(struct logger));
    logger->file_descriptor = open(events_log, O_CREAT | O_RDWR | O_TRUNC, 0777);;
    return logger;
}

int do_log_started_fmt(struct logger *logger, int local_id) {
    logger->log_length = sprintf(
            logger->log_buffer,
            log_started_fmt,
            local_id,
            getpid(),
            getppid());

    return write_log(logger);
}

int do_log_received_all_started_fmt(struct logger *logger, int local_id) {
    logger->log_length = sprintf(
            logger->log_buffer,
            log_received_all_started_fmt,
            local_id);

    return write_log(logger);
}

int do_log_done_fmt(struct logger *logger, int local_id) {
    logger->log_length = sprintf(
            logger->log_buffer,
            log_done_fmt,
            local_id);

    return write_log(logger);
}

int do_log_received_all_done_fmt(struct logger *logger, int local_id) {
    logger->log_length = sprintf(
            logger->log_buffer,
            log_received_all_done_fmt,
            local_id);

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



