//
//  Downloader_iOS.hpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 27/07/12.
//

#ifndef G3MiOSSDK_Downloader_iOS
#define G3MiOSSDK_Downloader_iOS

#import "Downloader_iOS_WorkerThread.h"
#import "Downloader_iOS_Handler.h"

#include "IDownloader.hpp"


class Downloader_iOS : public IDownloader {
private:
  NSMutableArray*      _workers;

  NSRecursiveLock*     _lock;                // synchronization helper
  NSMutableDictionary* _downloadingHandlers; // downloads current in progress
  NSMutableDictionary* _queuedHandlers;      // queued downloads

  long long _requestIDCounter;

  long long _requestsCounter;
  long long _cancelsCounter;

  bool _started;

  long long request(const URL &url,
                    long long priority,
                    Downloader_iOS_Listener* iosListener);


public:
  
  void removeDownloadingHandlerForNSURL(const NSURL* url);
  
  Downloader_iOS(int maxConcurrentOperationCount);
  
  long long requestBuffer(const URL& url,
                          long long priority,
                          const TimeInterval& timeToCache,
                          bool readExpired,
                          IBufferDownloadListener* listener,
                          bool deleteListener,
                          const std::string& tag);

  long long requestImage(const URL& url,
                         long long priority,
                         const TimeInterval& timeToCache,
                         bool readExpired,
                         IImageDownloadListener* listener,
                         bool deleteListener,
                         const std::string& tag);

  bool cancelRequest(long long requestID);

  void cancelRequestsTagged(const std::string& tag);

  Downloader_iOS_Handler* getHandlerToRun();
  
  virtual ~Downloader_iOS();
  
  void start();
  
  void stop();
  
  const std::string statistics();
  
  void onResume(const G3MContext* context) {
    
  }
  
  void onPause(const G3MContext* context) {
    
  }
  
  void onDestroy(const G3MContext* context) {

  }

  void initialize(const G3MContext* context,
                  FrameTasksExecutor* frameTasksExecutor) {

  }

};

#endif
