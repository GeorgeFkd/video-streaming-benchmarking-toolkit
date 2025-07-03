/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** main.cpp
**
** test V4L2 capture device
**
** -------------------------------------------------------------------------*/
#include <chrono>
#include <cstdint>
#include <errno.h>
#include <fstream>
#include <memory>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <array>
#include "V4l2Capture.h"
#include "logger.h"
#include <boost/math/statistics/univariate_statistics.hpp>
int stop = 0;

/* ---------------------------------------------------------------------------
**  SIGINT handler
** -------------------------------------------------------------------------*/
void sighandler(int) {
  printf("SIGINT\n");
  stop = 1;
}


std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
  return result;
   
}
/* ---------------------------------------------------------------------------
**  main
** -------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
  int verbose = 0;
  const char *in_devname = "/dev/video0";
  V4l2IoType ioTypeIn = IOTYPE_MMAP;
  int format = 0;
  int width = 0;
  int height = 0;
  int fps = 0;
  int framecount = 0;

  int c = 0;
  while ((c = getopt(argc, argv,
                     "x:hv:"
                     "G:f:r")) != -1) {
    switch (c) {
    case 'v':
      verbose = 1;
      if (optarg && *optarg == 'v')
        verbose++;
      break;
    case 'r':
      ioTypeIn = IOTYPE_READWRITE;
      break;
    case 'G':
      sscanf(optarg, "%dx%dx%d", &width, &height, &fps);
      break;
    case 'f':
      format = V4l2Device::fourcc(optarg);
      break;
    case 'x':
      sscanf(optarg, "%d", &framecount);
      break;
    case 'h': {
      std::cout
          << argv[0]
          << " [-v[v]] [-G <width>x<height>x<fps>] [-f format] [device] [-r]"
          << std::endl;
      std::cout << "\t -G <width>x<height>x<fps> : set capture resolution"
                << std::endl;
      std::cout << "\t -v            : verbose " << std::endl;
      std::cout << "\t -vv           : very verbose " << std::endl;
      std::cout << "\t -r            : V4L2 capture using read interface "
                   "(default use memory mapped buffers)"
                << std::endl;
      std::cout << "\t -x <count>    : read <count> frames and save them in "
                   "current dir."
                << std::endl;
      std::cout << "\t device        : V4L2 capture device (default "
                << in_devname << ")" << std::endl;
      exit(0);
    }
    }
  }
  if (optind < argc) {
    in_devname = argv[optind];
    optind++;
  }

  // initialize log4cpp
  initLogger(verbose);

  using Clock = std::chrono::high_resolution_clock;
  std::chrono::time_point<Clock> start, end;
  V4L2DeviceParameters param(in_devname, format, width, height, fps, ioTypeIn);

  start = Clock::now();
  V4l2Capture *videoCapture = V4l2Capture::create(param);
  end = Clock::now();
  auto startupTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count();

  std::cout << "Camera opened in: " << startupTime << "ms.\n";
  std::vector<long> delays;
  if (framecount != 0) {
    delays.reserve(framecount);
  }
  if (videoCapture == NULL) {
    LOG(WARN) << "Cannot reading from V4L2 capture interface for device:"
              << in_devname;
  } else {
    timeval tv;

    LOG(NOTICE) << "Start reading from " << in_devname;
    signal(SIGINT, sighandler);

    while (!stop) {
      static int buffers_read = 0;
      start = Clock::now();
      tv.tv_sec = 1;
      tv.tv_usec = 0;
      int ret = videoCapture->isReadable(&tv);
      if (ret == 1) {
        char buffer[videoCapture->getBufferSize()];
        int rsize = videoCapture->read(buffer, sizeof(buffer));
        end = Clock::now();
        buffers_read++;
        auto open_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count();
        LOG(NOTICE) << "Buffer: " << buffers_read << "delay: " << open_time
                    << " ms\n";
        delays.push_back(open_time);
        if (rsize == -1) {
          LOG(NOTICE) << "stop " << strerror(errno);
          stop = 1;
        } else {
          if (buffers_read > framecount) {
            stop = 1;
          }
        }
      } else if (ret == -1) {
        LOG(NOTICE) << "stop " << strerror(errno);
        stop = 1;
      }
    }

    delete videoCapture;
  }

  std::cout << "Writing data to file: " << "\n";
  std::ofstream file;
  file.open("camera-delays.txt",std::ios::out);
  std::string cameraInfo = exec("v4l2-ctl --info | grep 'Card type' | tr -d ' ' | awk -F ':' '{print $2}'");
  file << cameraInfo;
  for (long delay: delays) {
      file << delay << "\n";      
  }
  
  double mu = boost::math::statistics::mean(delays);
  double variance = boost::math::statistics::variance(delays);
  double stdev = std::sqrt(variance * ((double) delays.size()/(delays.size()-1)));
  file << "Average Delay:" << mu << "ms\n";
  file << "Standard Deviation: " << stdev << "ms\n";
  
  return 0;
}






