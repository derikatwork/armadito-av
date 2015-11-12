#include <glib.h>
#include <syslog.h>
#include <stdio.h>

static const char *level_str(GLogLevelFlags log_level)
{
  switch (log_level & G_LOG_LEVEL_MASK) {
  case G_LOG_LEVEL_ERROR:
    return "ERROR";
  case G_LOG_LEVEL_CRITICAL:
    return "CRITICAL";
  case G_LOG_LEVEL_WARNING:
    return "WARNING";
  case G_LOG_LEVEL_MESSAGE:
    return "MESSAGE";
  case G_LOG_LEVEL_INFO:
    return "INFO";
  case G_LOG_LEVEL_DEBUG:
    return "DEBUG";
  default:
    return "LOG";
  }

  return "???";
}

static int priority_from_level(GLogLevelFlags log_level)
{
  switch (log_level & G_LOG_LEVEL_MASK) {
  case G_LOG_LEVEL_ERROR:
    return LOG_ERR;
  case G_LOG_LEVEL_CRITICAL:
    return LOG_CRIT;
  case G_LOG_LEVEL_WARNING:
    return LOG_WARNING;
  case G_LOG_LEVEL_MESSAGE:
    return LOG_NOTICE;
  case G_LOG_LEVEL_INFO:
    return LOG_INFO;
  case G_LOG_LEVEL_DEBUG:
    return LOG_DEBUG;
  }
}

static void syslog_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
  syslog(priority_from_level(log_level), "%s: %s\n", level_str(log_level), message);
}

static FILE *get_stream(GLogLevelFlags log_level)
{
  gboolean to_stderr = TRUE;

  switch (log_level & G_LOG_LEVEL_MASK) {
    case G_LOG_LEVEL_INFO:
    case G_LOG_LEVEL_DEBUG:
      to_stderr = FALSE;
  }

  return to_stderr ? stderr : stdout;
}

static void stderrout_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
  FILE *stream = get_stream(log_level);
  GString *gstring = g_string_new(NULL);
  gchar *string;

  if (log_domain) {
    g_string_append(gstring, log_domain);
    g_string_append_c(gstring, '-');
  }

  g_string_append(gstring, level_str(log_level));
  g_string_append(gstring, ": ");
  g_string_append(gstring, message);
  g_string_append (gstring, "\n");

  string = g_string_free(gstring, FALSE);

  fputs(string, stream);

  g_free (string);
}

static GLogLevelFlags get_log_level_from_str(const char *s_log_level)
{
  if (!strcmp(s_log_level,"error"))
    return G_LOG_LEVEL_ERROR;
  if (!strcmp(s_log_level,"critical"))
    return G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL;
  if (!strcmp(s_log_level,"warning"))
    return G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING;
  if (!strcmp(s_log_level,"message"))
    return G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE;
  if (!strcmp(s_log_level,"info"))
    return G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO;
  if (!strcmp(s_log_level,"debug"))
    return G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG;

  return -1;
}

void log_init(const char *s_log_level, int use_syslog)
{
  GLogLevelFlags flags, fatal_flags;
  
  flags = get_log_level_from_str(s_log_level) | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION;

  if (use_syslog) {
    openlog("uhuru", LOG_CONS, LOG_USER);
    g_log_set_handler(G_LOG_DOMAIN, flags, syslog_handler, NULL);
  } else
    g_log_set_handler(G_LOG_DOMAIN, flags, stderrout_handler, NULL);

  fatal_flags = g_log_set_fatal_mask(G_LOG_DOMAIN, G_LOG_LEVEL_ERROR);
}
