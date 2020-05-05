#include "YaoPlayer.h"
#include "YaoAV/YaoAV.h"

YaoPlayerReaderThread::YaoPlayerReaderThread(std::string _path)
{
	path = _path;
}
YaoPlayerReaderThread::~YaoPlayerReaderThread()
{

}
void YaoPlayerReaderThread::run()
{
	YaoAVReader reader;
	int ret = reader.Open(path.c_str());
	if (ret) {
		return;
	}

	int videoStreamIndex = reader.getVideoStreamIndex();
	int audioStreamIndex = reader.getAudioStreamIndex();

	//TODO初始化解码器
	YaoDecodeThread* videoDecodeThread = new  YaoDecodeThread();
	YaoDecodeThread* audioDecodeThread = new  YaoDecodeThread();

	YaoAVStream videoStream;
	reader.getStream(&videoStream, videoStreamIndex);
	videoDecodeThread->init(&videoStream);

	YaoAVStream audioStream;
	reader.getStream(&audioStream, audioStreamIndex);
	audioDecodeThread->init(&audioStream);

	videoDecodeThread->start();
	audioDecodeThread->start();

	while (!stopFlag) {
		if (videoDecodeThread->packetQueueSize() > 5 && audioDecodeThread->packetQueueSize() > 5) {
			continue;
		}

		YaoAVPacket * pkt = new YaoAVPacket();
		int ret = reader.Read(pkt);
		if (ret) {
			delete pkt;
			pkt = nullptr;
			break;
		}

		if (pkt->getIndex() == videoStreamIndex) {
			videoDecodeThread->pushPacket(pkt);
		}

		if (pkt->getIndex() == audioStreamIndex) {
			audioDecodeThread->pushPacket(pkt);
		}
		//将packet放入缓存
		printf("get packet\n");

	}

	videoDecodeThread->stop();
	audioDecodeThread->stop();

	reader.Close();
}