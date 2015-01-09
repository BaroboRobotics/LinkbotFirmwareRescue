#include "listener.h"
#include "baromesh/dongledevicepath.hpp"

Listener::Listener(QObject *parent) : QObject(parent)
{
  timer_ = new QTimer(this);
  QObject::connect(timer_, SIGNAL(timeout()), 
      this, SLOT(doWork()));
}

void Listener::startWork()
{
  timer_->start();
}

void Listener::doWork()
{
  static int lastStatus = 0;
  int rc = 0;
  std::string donglepath;
  try {
      donglepath = baromesh::dongleDevicePath();
  } catch (...) {
      rc = -1;
  }
  if(
      (0 == rc) &&
      (0 != lastStatus)
    ) 
  {
    emit dongleDetected(QString::fromStdString(donglepath));
    timer_->stop();
  }
  lastStatus = rc;
}
