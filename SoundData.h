#pragma once

#include <irrklang/irrKlang.h>

class SoundData : public irrklang::ISoundMixedOutputReceiver {

public:
	SoundData() : mAudioData(nullptr), mDataCount(0) {};
	
	virtual void OnAudioDataReady(const void* data, int byteCount, int playbackrate) override {
		mAudioData = static_cast<const unsigned short*>(data);
		mDataCount = byteCount / 2;
	}

	short GetData() {
		return mAudioData[0];
	}
	
private:
	//const char* mAudioData;
	const unsigned short* mAudioData;
	int mDataCount;
};
