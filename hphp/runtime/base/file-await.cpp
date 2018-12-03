#include "hphp/runtime/base/file-await.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"

#include "hphp/util/compatibility.h"

#include <fcntl.h>

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

void FileTimeoutHandler::timeoutExpired() noexcept {
  if (m_fileAwait) {
    m_fileAwait->setFinished(FileAwait::TIMEOUT);
  }
}

void FileEventHandler::handlerReady(uint16_t events) noexcept {
  if (m_fileAwait) {
    m_fileAwait->setFinished(events ? FileAwait::READY : FileAwait::CLOSED);
  }
}

/////////////////////////////////////////////////////////////////////////////

FileAwait::FileAwait(int fd, uint16_t events, double timeout) {
  assertx(fd >= 0);
  assertx(events & FileEventHandler::READ_WRITE);

  auto asio_event_base = getSingleton<AsioEventBase>();
  m_file = std::make_shared<FileEventHandler>(asio_event_base.get(), fd, this);
  m_file->registerHandler(events);

  int64_t timeout_ms = timeout * 1000.0;
  if (timeout_ms > 0) {
    m_timeout = std::make_unique<FileTimeoutHandler>(asio_event_base.get(),
                                                     this);
    asio_event_base->runInEventBaseThreadAndWait([this,timeout_ms] {
      if (m_timeout) {
        m_timeout->scheduleTimeout(timeout_ms);
      }
    });
  }
}

FileAwait::~FileAwait() {
  if (m_file) {
    // Avoid possible race condition
    m_file->m_fileAwait = nullptr;

    m_file->unregisterHandler();
    m_file.reset();
  }
  if (m_timeout) {
    // Avoid race condition, we may (likely) finish destructing
    // before the timeout cancels
    m_timeout->m_fileAwait = nullptr;

    getSingleton<AsioEventBase>()
      ->runInEventBaseThreadAndWait([to{std::move(m_timeout)}] {
        to->cancelTimeout();
      });
  }
}

void FileAwait::unserialize(Cell& c) {
  c.m_type = KindOfInt64;
  c.m_data.num = m_result;
}

void FileAwait::setFinished(int64_t status) {
  if (status > m_result) {
    m_result = status;
  }
  if (!m_finished) {
    markAsFinished();
    m_finished = true;
  }
}

/////////////////////////////////////////////////////////////////////////////

Object File::await(uint16_t events, double timeout) {
  if (isClosed()) {
    Cell closedResult;
    closedResult.m_type = KindOfInt64;
    closedResult.m_data.num = FileAwait::CLOSED;
    return Object{c_StaticWaitHandle::CreateSucceeded(closedResult)};
  }
  if (fd() < 0) {
    SystemLib::throwExceptionObject(
      "Unable to await on stream, invalid file descriptor");
  }
  events = events & FileEventHandler::READ_WRITE;
  if (!events) {
    SystemLib::throwExceptionObject(
      "Must await for reading, writing, or both.");
  }
  const auto originalFlags = ::fcntl(fd(), F_GETFL);
  // This always succeeds...
  ::fcntl(fd(), F_SETFL, originalFlags | O_ASYNC);
  // ... but sometimes doesn't actually do anything
  const bool isAsyncableFd = ::fcntl(fd(), F_GETFL) & O_ASYNC;
  ::fcntl(fd(), F_SETFL, originalFlags);

  if (!isAsyncableFd) {
    SystemLib::throwInvalidOperationExceptionObject(
      folly::sformat(
        "File descriptor {} is not awaitable - real file?",
        fd()
      )
    );
  }

  auto ev = new FileAwait(fd(), events, timeout);
  try {
    return Object{ev->getWaitHandle()};
  } catch (...) {
    assertx(false);
    ev->abandon();
    throw;
  }
}

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
