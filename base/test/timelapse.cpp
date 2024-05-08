#include <boost/test/unit_test.hpp>

#include "Mp4ReaderSource.h"
#include "MotionVectorExtractor.h"
#include "Mp4WriterSink.h"
#include "H264Metadata.h"
#include "PipeLine.h"
#include "OverlayModule.h"
#include "ImageViewerModule.h"
#include "Mp4VideoMetadata.h"
#include "H264EncoderNVCodec.h"
#include "CudaMemCopy.h"
#include "CudaStreamSynchronize.h"
#include "ColorConversionXForm.h"
#include "FileWriterModule.h"

BOOST_AUTO_TEST_SUITE(timeplase_using_motion_detection)

BOOST_AUTO_TEST_CASE(generateTimeLapseUsingMotionDetection)
{
	std::string outFolderPath = "./data/timeplase_videos/";
	auto cuContext = apracucontext_sp(new ApraCUcontext());
	uint32_t gopLength = 25;
	uint32_t bitRateKbps = 1000;
	uint32_t frameRate = 30;
	H264EncoderNVCodecProps::H264CodecProfile profile = H264EncoderNVCodecProps::MAIN;
	bool enableBFrames = false;
    bool sendDecodedFrames = true;
	bool sendOverlayFrames = false;
    std::string videoPath = "./data/Mp4_videos/h264_video_metadata/20230514/0011/video1.mp4";
    auto h264ImageMetadata = framemetadata_sp(new H264Metadata(0, 0));
    auto mp4ReaderProps = Mp4ReaderSourceProps(videoPath, false, 0, true, false, false);
	mp4ReaderProps.fps = 24;
	auto mp4Reader = boost::shared_ptr<Mp4ReaderSource>(new Mp4ReaderSource(mp4ReaderProps));
    mp4Reader -> addOutPutPin(h264ImageMetadata);
	auto mp4Metadata = framemetadata_sp(new Mp4VideoMetadata("v_1"));
	mp4Reader->addOutPutPin(mp4Metadata);
	std::vector<std::string> mImagePin;
	mImagePin = mp4Reader->getAllOutputPinsByType(FrameMetadata::H264_DATA);
    auto motionExtractor = boost::shared_ptr<MotionVectorExtractor>(new MotionVectorExtractor(MotionVectorExtractorProps(MotionVectorExtractorProps::MVExtractMethod::OPENH264, sendDecodedFrames, 50, sendOverlayFrames)));
	auto m2 = boost::shared_ptr<FileWriterModule>(new FileWriterModule(FileWriterModuleProps("./data/testOutput/timelapseframe_????.raw")));
    mp4Reader -> setNext(motionExtractor, mImagePin);
	// motionExtractor -> setNext(m2);

	auto colorchange1 = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::BGR_TO_RGB)));
	motionExtractor -> setNext(colorchange1);

	auto colorchange2 = boost::shared_ptr<ColorConversion>(new ColorConversion(ColorConversionProps(ColorConversionProps::RGB_TO_YUV420PLANAR)));
	colorchange1->setNext(colorchange2);

	cudastream_sp cudaStream_ = boost::shared_ptr<ApraCudaStream>(new ApraCudaStream());
	auto copy = boost::shared_ptr<Module>(new CudaMemCopy(CudaMemCopyProps(cudaMemcpyHostToDevice, cudaStream_)));
	colorchange2->setNext(copy);

	auto sync = boost::shared_ptr<Module>(new CudaStreamSynchronize(CudaStreamSynchronizeProps(cudaStream_)));
	copy->setNext(sync);
	auto encoder = boost::shared_ptr<Module>(new H264EncoderNVCodec(H264EncoderNVCodecProps(bitRateKbps, cuContext, gopLength, frameRate, profile, enableBFrames)));
	sync->setNext(encoder);
	auto mp4WriterSinkProps = Mp4WriterSinkProps(1, 10, 24, outFolderPath, false);
	auto mp4WriterSink = boost::shared_ptr<Module>(new Mp4WriterSink(mp4WriterSinkProps));
	encoder -> setNext(mp4WriterSink);

	PipeLine p("test");
	p.appendModule(mp4Reader);
	p.init();

	p.run_all_threaded();
	boost::this_thread::sleep_for(boost::chrono::seconds(20));

	LOG_INFO << "profiling done - stopping the pipeline";
	p.stop();
	p.term();
	p.wait_for_all();
}

BOOST_AUTO_TEST_SUITE_END()