/**
 * app_main.h
 *
 * Entry point for all applications.
 * @see FISHY_APPLICATION
 */
#ifndef FISHY_APP_MAIN_H
#define FISHY_APP_MAIN_H

// #include <WRAPPERS/NET/http_server.h>

/**
 * System defined exit codes.
 * User defined exit codes should be positive.
 */
struct eExitCode {
  enum type {
    OK = 0,
    EXCEPTION = -1,
    INVALID_FLAGS = -2,
    BAD_FILE = -3,
    BAD_DATA = -4,
  };
};

/**
 * Skeleton application interface
 */
class iFishyApplication {
  public:
  virtual ~iFishyApplication() {}

  /**
   * Called if flag parsing fails to output info about how to
   * run this application.
   */
  virtual void printHelp() = 0;

  /**
   * Called after flag parsing to init the application.
   */
  virtual void init(const char *arg0) = 0;

  /**
   * Called after init and remaining setup.
   * Return code is used as application return code.
   */
  virtual int main() = 0;

  /**
   * Initalized before main() and after init()
   */
  // wrappers::net::http::HttpServer &getStatusServer() { return m_statusServer;
  // }

  private:
  // wrappers::net::http::HttpServer m_statusServer;
};

/**
 * Application pointer, set manually or call
 *   FISHY_APPLICATION( name )
 * macro from your application's main cpp file.
 */
extern iFishyApplication *G_pApplication;

  /**
   * Sets up the global application pointer with
   * a skeleton instance of iFishyApplication named 'name'
   * it is up to the application to fill out the appropriate
   * functions.
   */
#  define FISHY_APPLICATION(name)           \
    class name : public iFishyApplication { \
      public:                               \
      void init(const char *arg0);          \
      int main();                           \
      void printHelp();                     \
    };                                      \
    name G_##name;                          \
    iFishyApplication *G_pApplication = &G_##name;

#endif
