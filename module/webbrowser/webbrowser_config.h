#ifndef WEBBROWSER_CONFIG_H
#define WEBBROWSER_CONFIG_H

// webrowser_config.h
// 10/10/2011

#include <QtGlobal>

#ifdef WEBBROWSER_LIB
  #define WEBBROWSER_API Q_DECL_EXPORT
#elif defined(WEBBROWSER_STATICLIB)
  #define WEBBROWSER_API
#else
  #define WEBBROWSER_API Q_DECL_IMPORT
#endif // WEBBROWSER_LIB

#define WEBBROWSER_HOMEPAGE     "http://www.google.com"
#define WEBBROWSER_SEARCHENGINE "www.google.com/search?q="

#endif // WEBBROWSER_CONFIG_H