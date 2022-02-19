#ifndef LOGGER_H
#define LOGGER_H

#ifdef LOG_DEBUG
#define log_debug kprintf
#else
#define log_debug
#endif

#ifdef LOG_INFO
#define log_info kprintf
#else
#define log_info
#endif

#ifdef LOG_WARN
#define log_warn kprintf
#else
#define log_warn
#endif

#ifdef LOG_ERROR
#define log_error kprintf
#else
#define log_error
#endif

#endif