#ifndef _LOG_HEADER
#define _LOG_HEADER


#ifdef __cplusplus
extern "C" {
#endif

typedef enum II_LogPriority {
  II_LOG_UNKNOWN = 0,
  II_LOG_VERBOSE,
  II_LOG_DEBUG,
  II_LOG_INFO,
  II_LOG_WARN,
  II_LOG_ERROR,
  II_LOG_FATAL,
} II_LogPriority;

#define LOG_PRI(priority, tag, ...) printLog(priority, tag, __VA_ARGS__)

#define LOG(priority, tag, ...) LOG_PRI(II_##priority, tag, __VA_ARGS__)

#if LOG_NDEBUG
#define LOGV(...) 
#else
#define LOGV(...) ((void)LOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#endif

#define LOGD(...) ((void)LOG(LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOGI(...) ((void)LOG(LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)LOG(LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)LOG(LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define LOGF(...) ((void)LOG(LOG_FATAL, LOG_TAG, __VA_ARGS__))



int __log_print(int prio, const char *tag,  const char *fmt, ...);

enum {
  TYPE_NULL,
  TYPE_FILE,
  TYPE_CONSOLE,
  TYPE_DEBUGCONSOLE
};

//void log_init(int type, const char* path);
void log_init(bool bconsole, int console_level, const char* console_path, bool bfile, int file_level, const char* 
file_dirctory);


#define printLog(prio, tag, ...) \
    __log_print(prio, tag, __VA_ARGS__)


#ifdef __cplusplus
}
#endif

#endif // _LOG_HEADER

