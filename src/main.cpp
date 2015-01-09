#include "dialog.h"

#include "baromesh/iocore.hpp"
#include "baromesh/daemon.hpp"
#include "baromesh/tcpclient.hpp"

#include <boost/asio/use_future.hpp>

#include <boost/scope_exit.hpp>

#include <QApplication>
#include <QDebug>

#include <atomic>
#include <chrono>
#include <future>
#include <thread>
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

using boost::asio::use_future;

int main(int argc, char *argv[])
{
    std::atomic<bool> distractTheDaemon = { true };
    auto daemonDistracted = std::async(std::launch::async, [&] {
        auto ioCore = baromesh::IoCore::get(false);
        baromesh::TcpClient daemon { ioCore->ios(), boost::log::sources::logger() };
        boost::asio::ip::tcp::resolver resolver { ioCore->ios() };
        auto daemonQuery = decltype(resolver)::query {
            baromesh::daemonHostName(), baromesh::daemonServiceName()
        };
        auto daemonIter = resolver.resolve(daemonQuery);
        baromesh::asyncInitTcpClient(daemon, daemonIter, use_future).get();
        rpc::asio::asyncConnect<barobo::Daemon>(daemon, std::chrono::seconds(1), use_future).get();
        // probably don't need to worry about broadcasts
        //daemonRunDone = rpc::asio::asyncRunClient<barobo::Daemon>(daemon, *this, use_future);
        while (distractTheDaemon) {
            // The idea here is to tell the daemon every m seconds to
            // relinquish the dongle and reacquire it after n seconds. As long
            // as m < n, the daemon will keep its hands off the serial port
            // until distractTheDaemon becomes false.
            asyncFire(daemon, rpc::MethodIn<barobo::Daemon>::cycleDongle{2},
                std::chrono::seconds(1), use_future).get();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    BOOST_SCOPE_EXIT(&distractTheDaemon, &daemonDistracted) {
        distractTheDaemon = false;
        try {
            daemonDistracted.get();
        }
        catch (std::exception& e) {
            qDebug() << "Exception while distracting the daemon:" << e.what();
        }
    } BOOST_SCOPE_EXIT_END

#if 0
#ifdef _WIN32
    auto serviceStatusToString = [] (int status) -> const char* {
        switch (status) {
            case SERVICE_RUNNING:
                return "SERVICE_RUNNING";
            case SERVICE_STOPPED:
                return "SERVICE_STOPPED";
            case SERVICE_STOP_PENDING:
                return "SERVICE_STOP_PENDING";
            default:
                return "(unknown service status)";
        }
    };
    SC_HANDLE scm = 0;
    SC_HANDLE baromeshd = 0;

    scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (scm) {
        baromeshd = OpenService(scm, "baromeshd", SERVICE_START | SERVICE_STOP);
        if (baromeshd) {
            SERVICE_STATUS status;
            auto rc = ControlService(baromeshd, SERVICE_CONTROL_STOP, &status);
            if (!rc) {
                qDebug() << "ControlService failed:" << GetLastError();
            }
            else {
                qDebug() << "ControlService reports:"
                         << serviceStatusToString(status.dwCurrentState);
            }
        }
        else {
            qDebug() << "OpenService failed:" << GetLastError();
        }
    }
    else {
        qDebug() << "OpenSCManager failed:" << GetLastError();
    }

    BOOST_SCOPE_EXIT(scm, baromeshd) {
        if (scm && baromeshd) {
            StartService(baromeshd, 0, nullptr);
        }

        if (scm) {
            auto rc = CloseServiceHandle(scm);
            if (!rc) {
                qDebug() << "CloseServiceHandle(scm) failed:" << GetLastError();
            }
            scm = 0;
        }
        if (baromeshd) {
            auto rc = CloseServiceHandle(baromeshd);
            if (!rc) {
                qDebug() << "CloseServiceHandle(scm) failed:" << GetLastError();
            }
            baromeshd = 0;
        }
    } BOOST_SCOPE_EXIT_END
#endif
#endif

    QApplication a(argc, argv);
    Dialog w { nullptr, argc > 1 ? argv[1] : defaultHexFileName() };
    w.show();

    return a.exec();
}
