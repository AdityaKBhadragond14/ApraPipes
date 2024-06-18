### 2. Relay sample:
This sample demonstrate the use of relay command in the ApraPipes which can be used to swith the stream from live to recorder and vice versa.
#### - Modules Used:
- RtspClientSrc: To read the live stream from RTSP camera.
- Mp4Reader: To Read recorded stream from the Mp4Video.
- H264Decoder: To decode the h264 frames from RTSP and MP4 sources.
- colorConversion: To transform YUV420 frame to RAW RGB format.
- ImageViewer: To render RGB frames on the screen.
- arguments: 
    - arg1: RTSP camera URL
    - arg2: MP4 video file path
- The Unit test "relay_sample_test.cpp" is located inside the aprapipessampleut executable. Make sure to replace the `rtspUrl` and `mp4VideoPath` with suitable paths.