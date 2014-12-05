#include "dialog.h"

#include <boost/scope_exit.hpp>

#include <QApplication>
#include <cassert>

static QString defaultHexFileName () {
#ifndef _WIN32
  return "hexfiles/linkbot_latest.hex";
#else
  /* Get the install path of BaroboLink from the registry */
  DWORD size;
  HKEY key;
  int rc;

  rc = RegOpenKeyExA(
      HKEY_LOCAL_MACHINE,
      "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\BaroboLink.exe",
      0,
      KEY_QUERY_VALUE,
      &key);

  if (ERROR_SUCCESS != rc) {
    // FIXME?
    return "hexfiles\\linkbot_latest.hex";
  }

  /* Find out how much memory to allocate. */
  rc = RegQueryValueExA(
      key,
      "PATH",
      NULL,
      NULL,
      NULL,
      &size);
  assert(ERROR_SUCCESS == rc);

  /* hlh: FIXME this should probably be TCHAR instead, and we should support
   * unicode or whatever */
  char* path = new char [size + 1];

  rc = RegQueryValueExA(
      key,
      "PATH",
      NULL,
      NULL,
      (LPBYTE)path,
      &size);
  assert(ERROR_SUCCESS == rc);

  path[size] = '\0';

  auto ret = QString(path) + "\\hexfiles\\linkbot_latest.hex";
  delete [] path;
  path = NULL;

  return ret;
#endif
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SC_HANDLE scm = 0;
    SC_HANDLE baromeshd = 0;

    scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (scm) {
        baromeshd = OpenService(scm, "baromeshd", SERVICE_START | SERVICE_STOP);
        if (baromeshd) {
          SERVICE_STATUS status;
          ControlService(baromeshd, SERVICE_CONTROL_STOP, &status);
        }
    }

    BOOST_SCOPE_EXIT(scm, baromeshd) {
        if (scm && baromeshd) {
            StartService(baromeshd, 0, nullptr);
        }
    } BOOST_SCOPE_EXIT_END
#endif

    QApplication a(argc, argv);
    Dialog w { nullptr, argc > 1 ? argv[1] : defaultHexFileName() };
    w.show();

    return a.exec();
}
